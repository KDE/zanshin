/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/


#include "akonadiprojectqueries.h"

#include "akonadicollectionfetchjobinterface.h"
#include "akonadiitemfetchjobinterface.h"
#include "akonadimonitorimpl.h"
#include "akonadiserializer.h"
#include "akonadistorage.h"

#include "utils/jobhandler.h"

using namespace Akonadi;

ProjectQueries::ProjectQueries(QObject *parent)
    : QObject(parent),
      m_storage(new Storage),
      m_serializer(new Serializer),
      m_monitor(new MonitorImpl),
      m_ownInterfaces(true)
{
    connect(m_monitor, SIGNAL(itemAdded(Akonadi::Item)), this, SLOT(onItemAdded(Akonadi::Item)));
    connect(m_monitor, SIGNAL(itemRemoved(Akonadi::Item)), this, SLOT(onItemRemoved(Akonadi::Item)));
    connect(m_monitor, SIGNAL(itemChanged(Akonadi::Item)), this, SLOT(onItemChanged(Akonadi::Item)));
    connect(m_monitor, SIGNAL(collectionSelectionChanged(Akonadi::Collection)), this, SLOT(onCollectionSelectionChanged()));
}

ProjectQueries::ProjectQueries(StorageInterface *storage, SerializerInterface *serializer, MonitorInterface *monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_monitor(monitor),
      m_ownInterfaces(false)
{
    connect(monitor, SIGNAL(itemAdded(Akonadi::Item)), this, SLOT(onItemAdded(Akonadi::Item)));
    connect(monitor, SIGNAL(itemRemoved(Akonadi::Item)), this, SLOT(onItemRemoved(Akonadi::Item)));
    connect(monitor, SIGNAL(itemChanged(Akonadi::Item)), this, SLOT(onItemChanged(Akonadi::Item)));
    connect(m_monitor, SIGNAL(collectionSelectionChanged(Akonadi::Collection)), this, SLOT(onCollectionSelectionChanged()));
}

ProjectQueries::~ProjectQueries()
{
    if (m_ownInterfaces) {
        delete m_storage;
        delete m_serializer;
        delete m_monitor;
    }
}

ProjectQueries::ProjectResult::Ptr ProjectQueries::findAll() const
{
    if (!m_findAll) {
        {
            ProjectQueries *self = const_cast<ProjectQueries*>(this);
            self->m_findAll = self->createProjectQuery();
        }

        m_findAll->setFetchFunction([this] (const ProjectQuery::AddFunction &add) {
            CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                           StorageInterface::Recursive,
                                                                           StorageInterface::Tasks);
            Utils::JobHandler::install(job->kjob(), [this, job, add] {
                if (job->kjob()->error() != KJob::NoError)
                    return;

                for (auto collection : job->collections()) {
                    if (!m_serializer->isSelectedCollection(collection))
                        continue;

                    ItemFetchJobInterface *job = m_storage->fetchItems(collection);
                    Utils::JobHandler::install(job->kjob(), [this, job, add] {
                        if (job->kjob()->error() != KJob::NoError)
                            return;

                        for (auto item : job->items()) {
                            add(item);
                        }
                    });
                }
            });
        });

        m_findAll->setConvertFunction([this] (const Akonadi::Item &item) {
            return m_serializer->createProjectFromItem(item);
        });
        m_findAll->setUpdateFunction([this] (const Akonadi::Item &item, Domain::Project::Ptr &project) {
            m_serializer->updateProjectFromItem(project, item);
        });
        m_findAll->setPredicateFunction([this] (const Akonadi::Item &item) {
            return m_serializer->isProjectItem(item);
        });
        m_findAll->setRepresentsFunction([this] (const Akonadi::Item &item, const Domain::Project::Ptr &project) {
            return m_serializer->representsItem(project, item);
        });
    }

    return m_findAll->result();
}

ProjectQueries::ArtifactResult::Ptr ProjectQueries::findTopLevelArtifacts(Domain::Project::Ptr project) const
{
    Akonadi::Item item = m_serializer->createItemFromProject(project);
    if (!m_findTopLevel.contains(item.id())) {
        ArtifactQuery::Ptr query;

        {
            ProjectQueries *self = const_cast<ProjectQueries*>(this);
            query = self->createArtifactQuery();
            self->m_findTopLevel.insert(item.id(), query);
        }

        query->setFetchFunction([this, item] (const ArtifactQuery::AddFunction &add) {
            CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                           StorageInterface::Recursive,
                                                                           StorageInterface::Tasks | StorageInterface::Notes);
            Utils::JobHandler::install(job->kjob(), [this, job, add] {
                if (job->kjob()->error() != KJob::NoError)
                    return;

                for (auto collection : job->collections()) {
                    ItemFetchJobInterface *job = m_storage->fetchItems(collection);
                    Utils::JobHandler::install(job->kjob(), [this, job, add] {
                        if (job->kjob()->error() != KJob::NoError)
                            return;

                        for (auto item : job->items())
                            add(item);
                    });
                }
            });
        });
        query->setConvertFunction([this] (const Akonadi::Item &item) {
            if (m_serializer->isTaskItem(item)) {
                auto task = m_serializer->createTaskFromItem(item);
                return Domain::Artifact::Ptr(task);

            } else if (m_serializer->isNoteItem(item)) {
                auto note = m_serializer->createNoteFromItem(item);
                return Domain::Artifact::Ptr(note);

            } else {
                return Domain::Artifact::Ptr();
            }
        });
        query->setUpdateFunction([this] (const Akonadi::Item &item, Domain::Artifact::Ptr &artifact) {
            if (auto task = artifact.dynamicCast<Domain::Task>()) {
                m_serializer->updateTaskFromItem(task, item);
            } else if (auto note = artifact.dynamicCast<Domain::Note>()) {
                m_serializer->updateNoteFromItem(note, item);
            }
        });
        query->setPredicateFunction([this, project] (const Akonadi::Item &item) {
            return m_serializer->isProjectChild(project, item);
        });
        query->setRepresentsFunction([this] (const Akonadi::Item &item, const Domain::Artifact::Ptr &artifact) {
            return m_serializer->representsItem(artifact, item);
        });
    }

    return m_findTopLevel.value(item.id())->result();
}

void ProjectQueries::onItemAdded(const Item &item)
{
    foreach (const ProjectQuery::Ptr &query, m_projectQueries)
        query->onAdded(item);

    foreach (const ArtifactQuery::Ptr &query, m_artifactQueries)
        query->onAdded(item);
}

void ProjectQueries::onItemRemoved(const Item &item)
{
    foreach (const ProjectQuery::Ptr &query, m_projectQueries)
        query->onRemoved(item);

    foreach (const ArtifactQuery::Ptr &query, m_artifactQueries)
        query->onRemoved(item);
}

void ProjectQueries::onItemChanged(const Item &item)
{
    foreach (const ProjectQuery::Ptr &query, m_projectQueries)
        query->onChanged(item);

    foreach (const ArtifactQuery::Ptr &query, m_artifactQueries)
        query->onChanged(item);
}

void ProjectQueries::onCollectionSelectionChanged()
{
    foreach (const ProjectQuery::Ptr &query, m_projectQueries)
        query->reset();
}

ProjectQueries::ProjectQuery::Ptr ProjectQueries::createProjectQuery()
{
    auto query = ProjectQueries::ProjectQuery::Ptr::create();
    m_projectQueries << query;
    return query;
}

ProjectQueries::ArtifactQuery::Ptr ProjectQueries::createArtifactQuery()
{
    auto query = ProjectQueries::ArtifactQuery::Ptr::create();
    m_artifactQueries << query;
    return query;
}
