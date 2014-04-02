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

TaskQueries::TaskQueries()
    : m_storage(new Storage),
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

    if (!provider) {
        provider = TaskProvider::Ptr(new TaskProvider);
        m_taskProvider = provider.toWeakRef();
    }

    TaskQueries::TaskResult::Ptr result = TaskProvider::createResult(provider);

    CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(), StorageInterface::Recursive);
    Utils::JobHandler::install(job->kjob(), [provider, job, this] {
        for (auto collection : job->collections()) {
            ItemFetchJobInterface *job = m_storage->fetchItems(collection);
            Utils::JobHandler::install(job->kjob(), [provider, job, this] {
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
    TaskProvider::Ptr provider(new TaskProvider);

    TaskQueries::TaskResult::Ptr result = TaskProvider::createResult(provider);

    Akonadi::Item item(task->property("itemId").value<Akonadi::Entity::Id>());

    ItemFetchJobInterface *job = m_storage->fetchItem(item);
    Utils::JobHandler::install(job->kjob(), [provider, job, task, this] {
        Q_ASSERT(job->items().size() == 1);
        auto item = job->items()[0];
        Q_ASSERT(item.parentCollection().isValid());
        ItemFetchJobInterface *job = m_storage->fetchItems(item.parentCollection());
        Utils::JobHandler::install(job->kjob(), [provider, job, task, this] {
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

TaskQueries::ContextResult::Ptr TaskQueries::findContexts(Domain::Task::Ptr task) const
{
    qFatal("Not implemented yet");
    Q_UNUSED(task);
    return ContextProvider::createResult(ContextProvider::Ptr());
}

void TaskQueries::onItemAdded(const Item &item)
{
    TaskProvider::Ptr provider(m_taskProvider.toStrongRef());

    if (provider) {
        auto task = deserializeTask(item);
        if (task)
            provider->append(task);
    }
}

void TaskQueries::onItemRemoved(const Item &item)
{
    TaskProvider::Ptr provider(m_taskProvider.toStrongRef());

    if (provider) {
        for (int i = 0; i < provider->data().size(); i++) {
            auto task = provider->data().at(i);
            if (isTaskItem(task, item)) {
                provider->removeAt(i);
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
            if (isTaskItem(task, item)) {
                m_serializer->updateTaskFromItem(task, item);
                provider->replace(i, task);
            }
        }
    }
}

bool TaskQueries::isTaskItem(const Domain::Task::Ptr &task, const Item &item) const
{
    return task->property("itemId").toLongLong() == item.id();
}

Domain::Task::Ptr TaskQueries::deserializeTask(const Item &item) const
{
    auto task = m_serializer->createTaskFromItem(item);
    if (task)
        task->setProperty("itemId", item.id());
    return task;
}
