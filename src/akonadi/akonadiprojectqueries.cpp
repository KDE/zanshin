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

#include "utils/jobhandler.h"

#include <functional>

using namespace std::placeholders;

using namespace Akonadi;

ProjectQueries::ProjectQueries(const StorageInterface::Ptr &storage, const SerializerInterface::Ptr &serializer, const MonitorInterface::Ptr &monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_monitor(monitor)
{
    connect(m_monitor.data(), SIGNAL(itemAdded(Akonadi::Item)), this, SLOT(onItemAdded(Akonadi::Item)));
    connect(m_monitor.data(), SIGNAL(itemRemoved(Akonadi::Item)), this, SLOT(onItemRemoved(Akonadi::Item)));
    connect(m_monitor.data(), SIGNAL(itemChanged(Akonadi::Item)), this, SLOT(onItemChanged(Akonadi::Item)));
    connect(m_monitor.data(), SIGNAL(collectionSelectionChanged(Akonadi::Collection)), this, SLOT(onCollectionSelectionChanged()));
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
        m_findAll->setPredicateFunction([this] (const Akonadi::Item &item) {
            return m_serializer->isProjectItem(item);
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
        query->setPredicateFunction([this, project] (const Akonadi::Item &item) {
            return m_serializer->isProjectChild(project, item);
        });
    }

    return m_findTopLevel.value(item.id())->result();
}

void ProjectQueries::onItemAdded(const Item &item)
{
    foreach (const auto &query, m_itemInputQueries)
        query->onAdded(item);
}

void ProjectQueries::onItemRemoved(const Item &item)
{
    foreach (const auto &query, m_itemInputQueries)
        query->onRemoved(item);
}

void ProjectQueries::onItemChanged(const Item &item)
{
    foreach (const auto &query, m_itemInputQueries)
        query->onChanged(item);
}

void ProjectQueries::onCollectionSelectionChanged()
{
    foreach (const auto &query, m_itemInputQueries)
        query->reset();
}

ProjectQueries::ProjectQuery::Ptr ProjectQueries::createProjectQuery()
{
    auto query = ProjectQueries::ProjectQuery::Ptr::create();

    query->setConvertFunction(std::bind(&SerializerInterface::createProjectFromItem, m_serializer, _1));
    query->setUpdateFunction(std::bind(&SerializerInterface::updateProjectFromItem, m_serializer, _2, _1));
    query->setRepresentsFunction(std::bind(&SerializerInterface::representsItem, m_serializer, _2, _1));

    m_itemInputQueries << query;
    return query;
}

ProjectQueries::ArtifactQuery::Ptr ProjectQueries::createArtifactQuery()
{
    auto query = ProjectQueries::ArtifactQuery::Ptr::create();

    query->setConvertFunction(std::bind(&SerializerInterface::createArtifactFromItem, m_serializer, _1));
    query->setUpdateFunction(std::bind(&SerializerInterface::updateArtifactFromItem, m_serializer, _2, _1));
    query->setRepresentsFunction(std::bind(&SerializerInterface::representsItem, m_serializer, _2, _1));

    m_itemInputQueries << query;
    return query;
}
