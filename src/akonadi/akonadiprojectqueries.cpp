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
    ProjectProvider::Ptr provider(m_projectProvider.toStrongRef());

    if (!provider) {
        provider = ProjectProvider::Ptr(new ProjectProvider);
        m_projectProvider = provider.toWeakRef();
    }

    auto result = ProjectResult::create(provider);

    CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                   StorageInterface::Recursive,
                                                                   StorageInterface::Tasks);
    Utils::JobHandler::install(job->kjob(), [provider, job, this] {
        if (job->kjob()->error() != KJob::NoError)
            return;

        for (auto collection : job->collections()) {
            ItemFetchJobInterface *job = m_storage->fetchItems(collection);
            Utils::JobHandler::install(job->kjob(), [provider, job, this] {
                if (job->kjob()->error() != KJob::NoError)
                    return;

                for (auto item : job->items()) {
                    auto project = m_serializer->createProjectFromItem(item);
                    if (project)
                        provider->append(project);
                }
            });
        }
    });

    return result;
}

ProjectQueries::ArtifactResult::Ptr ProjectQueries::findTopLevelArtifacts(Domain::Project::Ptr project) const
{
    qFatal("Not implemented yet");
    Q_UNUSED(project);
    return ArtifactResult::Ptr();
}

void ProjectQueries::onItemAdded(const Item &item)
{
    ProjectProvider::Ptr provider(m_projectProvider.toStrongRef());
    auto project = m_serializer->createProjectFromItem(item);

    if (!project)
        return;

    if (provider) {
        provider->append(project);
    }
}

void ProjectQueries::onItemRemoved(const Item &item)
{
    ProjectProvider::Ptr provider(m_projectProvider.toStrongRef());

    if (provider) {
        for (int i = 0; i < provider->data().size(); i++) {
            auto project = provider->data().at(i);
            if (m_serializer->represents(project, item)) {
                provider->removeAt(i);
                i--;
            }
        }
    }
}

void ProjectQueries::onItemChanged(const Item &item)
{
    ProjectProvider::Ptr provider(m_projectProvider.toStrongRef());

    if (provider) {
        for (int i = 0; i < provider->data().size(); i++) {
            auto project = provider->data().at(i);
            if (m_serializer->represents(project, item)) {
                m_serializer->updateProjectFromItem(project, item);
                provider->replace(i, project);
            }
        }
    }
}
