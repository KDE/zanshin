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

    if (provider)
        return ProjectResult::create(provider);

    provider = ProjectProvider::Ptr::create();
    m_projectProvider = provider.toWeakRef();

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
    Akonadi::Item item = m_serializer->createItemFromProject(project);
    ArtifactProvider::Ptr provider;

    if (m_topLevelProviders.contains(item.id())) {
        provider = m_topLevelProviders.value(item.id()).toStrongRef();
        if (provider)
            return ArtifactResult::create(provider);
    }

    provider = ArtifactProvider::Ptr::create();
    m_topLevelProviders[item.id()] = provider;

    auto result = ArtifactResult::create(provider);

    addItemIdInCache(project, item.id());

    CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                   StorageInterface::Recursive,
                                                                   StorageInterface::Tasks | StorageInterface::Notes);
    Utils::JobHandler::install(job->kjob(), [provider, job, project, this] {
        if (job->kjob()->error() != KJob::NoError)
            return;

        for (auto collection : job->collections()) {
            ItemFetchJobInterface *job = m_storage->fetchItems(collection);
            Utils::JobHandler::install(job->kjob(), [provider, job, project, this] {
                if (job->kjob()->error() != KJob::NoError)
                    return;

                for (auto item : job->items()) {
                    if (m_serializer->isProjectChild(project, item)) {
                        auto artifact = deserializeArtifact(item);
                        if (artifact)
                            provider->append(artifact);
                    }
                }
            });
        }
    });

    return result;
}

void ProjectQueries::onItemAdded(const Item &item)
{
    ProjectProvider::Ptr provider(m_projectProvider.toStrongRef());
    auto project = m_serializer->createProjectFromItem(item);

    if (provider && project) {
        provider->append(project);
        return;
    }

    ArtifactProvider::Ptr topLevelProvider = topLevelProviderForItem(item);
    if (topLevelProvider) {
        Domain::Artifact::Ptr artifact = deserializeArtifact(item);
        topLevelProvider->append(artifact);
    }
}

void ProjectQueries::onItemRemoved(const Item &item)
{
    ProjectProvider::Ptr provider(m_projectProvider.toStrongRef());

    if (provider) {
        for (int i = 0; i < provider->data().size(); i++) {
            auto project = provider->data().at(i);
            if (m_serializer->representsItem(project, item)) {
                provider->removeAt(i);
                i--;
            }
        }
    }

    ArtifactProvider::Ptr topLevelProvider = topLevelProviderForItem(item);
    if (topLevelProvider) {
        for (int i = 0; i < topLevelProvider->data().size(); i++) {
            auto artifact = topLevelProvider->data().at(i);
            if (m_serializer->representsItem(artifact, item)) {
                topLevelProvider->removeAt(i);
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
            if (m_serializer->representsItem(project, item)) {
                m_serializer->updateProjectFromItem(project, item);
                provider->replace(i, project);
            }
        }
    }

    ArtifactProvider::Ptr topLevelProvider = topLevelProviderForItem(item);
    if (topLevelProvider) {
        bool itemUpdated = false;
        for (int i = 0; i < topLevelProvider->data().size(); i++) {
            auto artifact = topLevelProvider->data().at(i);
            if (m_serializer->representsItem(artifact, item)) {
                if (auto task = artifact.dynamicCast<Domain::Task>())
                    m_serializer->updateTaskFromItem(task, item);
                else if (auto note = artifact.dynamicCast<Domain::Note>())
                    m_serializer->updateNoteFromItem(note, item);
                topLevelProvider->replace(i, artifact);
                itemUpdated = true;
            }
        }

        if (!itemUpdated) {
            auto artifact = deserializeArtifact(item);
            if (artifact)
                topLevelProvider->append(artifact);
        }
    } else {
        removeItemFromTopLevelProviders(item);
    }

}

void ProjectQueries::addItemIdInCache(const Domain::Project::Ptr &project, Entity::Id id) const
{
    m_uidtoIdCache[m_serializer->objectUid(project)] = id;
}

Domain::Artifact::Ptr ProjectQueries::deserializeArtifact(const Item &item) const
{
    m_idToRelatedUidCache[item.id()] = m_serializer->relatedUidFromItem(item);

    auto task = m_serializer->createTaskFromItem(item);
    if (task) {
        return task;
    }

    auto note = m_serializer->createNoteFromItem(item);
    if (note) {
        return note;
    }

    return Domain::Artifact::Ptr();
}

ProjectQueries::ArtifactProvider::Ptr ProjectQueries::topLevelProviderForItem(const Item &item) const
{
    ArtifactProvider::Ptr topLevelProvider;

    auto uid = m_serializer->relatedUidFromItem(item);
    if (m_uidtoIdCache.contains(uid)) {
        Akonadi::Entity::Id parentId = m_uidtoIdCache.value(uid);
        if (m_topLevelProviders.contains(parentId))
            topLevelProvider = m_topLevelProviders.value(parentId).toStrongRef();
    }

    return topLevelProvider;
}

void ProjectQueries::removeItemFromTopLevelProviders(const Item &item)
{
    if (m_idToRelatedUidCache.contains(item.id())) {
        auto lastRelatedUid = m_idToRelatedUidCache.value(item.id());
        auto relatedUid = m_serializer->relatedUidFromItem(item);
        if (lastRelatedUid == relatedUid)
            return;

        if (m_uidtoIdCache.contains(lastRelatedUid)) {
            auto parentId = m_uidtoIdCache.value(lastRelatedUid);
            if (m_topLevelProviders.contains(parentId)) {
                ArtifactProvider::Ptr topLevelProvider = m_topLevelProviders.value(parentId).toStrongRef();
                if (topLevelProvider) {
                    for (int i = 0; i < topLevelProvider->data().size(); i++) {
                        auto artifact = topLevelProvider->data().at(i);
                        if (m_serializer->representsItem(artifact, item)) {
                            topLevelProvider->removeAt(i);
                            i--;
                        }
                    }
                }
            }
        }
    }
}
