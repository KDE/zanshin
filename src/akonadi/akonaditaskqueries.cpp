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


#include "akonaditaskqueries.h"

#include "akonadicollectionfetchjobinterface.h"
#include "akonadiitemfetchjobinterface.h"
#include "akonadimonitorimpl.h"
#include "akonadiserializer.h"
#include "akonadistorage.h"

#include "utils/jobhandler.h"

using namespace Akonadi;

TaskQueries::TaskQueries(QObject *parent)
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

TaskQueries::TaskQueries(StorageInterface *storage, SerializerInterface *serializer, MonitorInterface *monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_monitor(monitor),
      m_ownInterfaces(false)
{
    connect(monitor, SIGNAL(itemAdded(Akonadi::Item)), this, SLOT(onItemAdded(Akonadi::Item)));
    connect(monitor, SIGNAL(itemRemoved(Akonadi::Item)), this, SLOT(onItemRemoved(Akonadi::Item)));
    connect(monitor, SIGNAL(itemChanged(Akonadi::Item)), this, SLOT(onItemChanged(Akonadi::Item)));
}

TaskQueries::~TaskQueries()
{
    if (m_ownInterfaces) {
        delete m_storage;
        delete m_serializer;
        delete m_monitor;
    }
}

TaskQueries::TaskResult::Ptr TaskQueries::findAll() const
{
    TaskProvider::Ptr provider(m_taskProvider.toStrongRef());

    if (provider)
        return TaskResult::create(provider);

    provider = TaskProvider::Ptr::create();
    m_taskProvider = provider.toWeakRef();

    auto result = TaskResult::create(provider);

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
                    auto task = deserializeTask(item);
                    if (task)
                        provider->append(task);
                }
            });
        }
    });

    return result;
}

TaskQueries::TaskResult::Ptr TaskQueries::findChildren(Domain::Task::Ptr task) const
{
    Akonadi::Item item = m_serializer->createItemFromTask(task);
    TaskProvider::Ptr provider;

    if (m_taskChildProviders.contains(item.id())) {
        provider = m_taskChildProviders.value(item.id()).toStrongRef();
        if (provider)
            return TaskResult::create(provider);
    }

    provider = TaskProvider::Ptr(new TaskProvider);
    m_taskChildProviders[item.id()] = provider;

    auto result = TaskResult::create(provider);

    addItemIdInCache(task, item.id());

    ItemFetchJobInterface *job = m_storage->fetchItem(item);
    Utils::JobHandler::install(job->kjob(), [provider, job, task, this] {
        if (job->kjob()->error() != KJob::NoError)
            return;

        Q_ASSERT(job->items().size() == 1);
        auto item = job->items()[0];
        Q_ASSERT(item.parentCollection().isValid());
        ItemFetchJobInterface *job = m_storage->fetchItems(item.parentCollection());
        Utils::JobHandler::install(job->kjob(), [provider, job, task, this] {
            if (job->kjob()->error() != KJob::NoError)
                return;

            for (auto item : job->items()) {
                if (m_serializer->isTaskChild(task, item)) {
                    auto task = deserializeTask(item);
                    if (task)
                        provider->append(task);
                }
            }
        });
    });

    return result;
}

TaskQueries::TaskResult::Ptr TaskQueries::findTopLevel() const
{
    TaskProvider::Ptr provider(m_topTaskProvider.toStrongRef());

    if (provider)
        return TaskResult::create(provider);

    provider = TaskProvider::Ptr::create();
    m_topTaskProvider = provider.toWeakRef();

    auto result = TaskResult::create(provider);

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
                    if (m_serializer->relatedUidFromItem(item).isEmpty()) {
                        auto task = deserializeTask(item);
                        if (task)
                            provider->append(task);
                    }
                }
            });
        }
    });

    return result;
}

TaskQueries::ContextResult::Ptr TaskQueries::findContexts(Domain::Task::Ptr task) const
{
    qFatal("Not implemented yet");
    Q_UNUSED(task);
    return ContextResult::Ptr();
}

void TaskQueries::onItemAdded(const Item &item)
{
    TaskProvider::Ptr provider(m_taskProvider.toStrongRef());
    auto task = deserializeTask(item);

    if (!task)
        return;

    if (provider) {
        provider->append(task);
    }

    TaskProvider::Ptr topLevelProvider(m_topTaskProvider.toStrongRef());
    if (topLevelProvider) {
        if (m_serializer->relatedUidFromItem(item).isEmpty())
            topLevelProvider->append(task);
    }

    if (m_taskChildProviders.isEmpty())
        return;

    TaskProvider::Ptr childProvider = childProviderFromItem(item);
    if (childProvider) {
        childProvider->append(task);
    }
}

void TaskQueries::onItemRemoved(const Item &item)
{
    TaskProvider::Ptr provider(m_taskProvider.toStrongRef());

    if (provider) {
        for (int i = 0; i < provider->data().size(); i++) {
            auto task = provider->data().at(i);
            if (m_serializer->representsItem(task, item)) {
                provider->removeAt(i);
                i--;
            }
        }
    }

    TaskProvider::Ptr topLevelProvider(m_topTaskProvider.toStrongRef());
    if (topLevelProvider) {
        if (m_serializer->relatedUidFromItem(item).isEmpty()) {
            for (int i = 0; i < topLevelProvider->data().size(); i++) {
                auto task = topLevelProvider->data().at(i);
                if (m_serializer->representsItem(task, item)) {
                    topLevelProvider->removeAt(i);
                    i--;
                }
            }
        }
    }

    if (m_taskChildProviders.isEmpty())
        return;

    if (m_taskChildProviders.contains(item.id())) {
        TaskProvider::Ptr childProvider = m_taskChildProviders.value(item.id());
        if (childProvider) {
            while (!childProvider->data().isEmpty()) {
                auto task = childProvider->takeFirst();
                if (topLevelProvider)
                    topLevelProvider->append(task);
            }
        }
    }

    TaskProvider::Ptr childProvider = childProviderFromItem(item);
    if (childProvider) {
        for (int i = 0; i < childProvider->data().size(); i++) {
            auto task = childProvider->data().at(i);
            if (m_serializer->representsItem(task, item)) {
                childProvider->removeAt(i);
                i--;
            }
        }
    }
}

void TaskQueries::onItemChanged(const Item &item)
{
    TaskProvider::Ptr provider(m_taskProvider.toStrongRef());

    if (provider) {
        for (int i = 0; i < provider->data().size(); i++) {
            auto task = provider->data().at(i);
            if (m_serializer->representsItem(task, item)) {
                m_serializer->updateTaskFromItem(task, item);
                provider->replace(i, task);
            }
        }
    }

    TaskProvider::Ptr topLevelProvider(m_topTaskProvider.toStrongRef());
    if (topLevelProvider) {
        bool itemFound = false;
        QString uid = m_serializer->relatedUidFromItem(item);
        for (int i = 0; i < topLevelProvider->data().size(); i++) {
            auto task = topLevelProvider->data().at(i);
            if (m_serializer->representsItem(task, item)) {
                itemFound = true;
                if (uid.isEmpty()) {
                    m_serializer->updateTaskFromItem(task, item);
                    topLevelProvider->replace(i, task);
                } else {
                    topLevelProvider->removeAt(i);
                    i--;
                }
            }
        }
        if (!itemFound && uid.isEmpty()) {
            auto task = deserializeTask(item);
            if (task)
                topLevelProvider->append(task);
        }
    }

    if (m_taskChildProviders.isEmpty())
        return;

    TaskProvider::Ptr childProvider = childProviderFromItem(item);
    if (childProvider) {
        bool itemUpdated = false;
        for (int i = 0; i < childProvider->data().size(); i++) {
            auto task = childProvider->data().at(i);
            if (m_serializer->representsItem(task, item)) {
                m_serializer->updateTaskFromItem(task, item);
                childProvider->replace(i, task);
                itemUpdated = true;
            }
        }

        if (!itemUpdated) {
            auto task = deserializeTask(item);
            if (task)
                childProvider->append(task);
        }
    } else {
        removeItemFromChildProviders(item);
    }
}

Domain::Task::Ptr TaskQueries::deserializeTask(const Item &item) const
{
    auto task = m_serializer->createTaskFromItem(item);
    if (task) {
        addItemIdInCache(task, item.id());
        m_idToRelatedUidCache[item.id()] = m_serializer->relatedUidFromItem(item);
    }

    return task;
}

void TaskQueries::addItemIdInCache(const Domain::Task::Ptr &task, Akonadi::Entity::Id id) const
{
    m_uidtoIdCache[m_serializer->objectUid(task)] = id;
}

TaskQueries::TaskProvider::Ptr TaskQueries::childProviderFromItem(const Item &item) const
{
    TaskProvider::Ptr childProvider;

    auto uid = m_serializer->relatedUidFromItem(item);
    if (m_uidtoIdCache.contains(uid)) {
        auto parentId = m_uidtoIdCache.value(uid);
        if (m_taskChildProviders.contains(parentId))
            childProvider = m_taskChildProviders.value(parentId).toStrongRef();
    }
    return childProvider;
}

void TaskQueries::removeItemFromChildProviders(const Item &item)
{
    if (m_idToRelatedUidCache.contains(item.id())) {
        auto lastRelatedUid = m_idToRelatedUidCache.value(item.id());
        auto relatedUid = m_serializer->relatedUidFromItem(item);
        if (lastRelatedUid == relatedUid)
            return;

        if (m_uidtoIdCache.contains(lastRelatedUid)) {
            auto parentId = m_uidtoIdCache.value(lastRelatedUid);
            if (m_taskChildProviders.contains(parentId)) {
                TaskProvider::Ptr childProvider = m_taskChildProviders.value(parentId).toStrongRef();
                if (childProvider) {
                    for (int i = 0; i < childProvider->data().size(); i++) {
                        auto task = childProvider->data().at(i);
                        if (m_serializer->representsItem(task, item)) {
                            childProvider->removeAt(i);
                            i--;
                        }
                    }
                }
            }
        }
    }
}
