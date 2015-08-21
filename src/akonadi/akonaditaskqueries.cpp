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

#include "utils/datetime.h"
#include "utils/jobhandler.h"

#include <functional>

using namespace std::placeholders;

using namespace Akonadi;

TaskQueries::TaskQueries(const StorageInterface::Ptr &storage,
                         const SerializerInterface::Ptr &serializer,
                         const MonitorInterface::Ptr &monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_monitor(monitor)
{
    connect(m_monitor.data(), SIGNAL(itemAdded(Akonadi::Item)), this, SLOT(onItemAdded(Akonadi::Item)));
    connect(m_monitor.data(), SIGNAL(itemRemoved(Akonadi::Item)), this, SLOT(onItemRemoved(Akonadi::Item)));
    connect(m_monitor.data(), SIGNAL(itemChanged(Akonadi::Item)), this, SLOT(onItemChanged(Akonadi::Item)));
}

TaskQueries::TaskResult::Ptr TaskQueries::findAll() const
{
    if (!m_findAll) {
        {
            TaskQueries *self = const_cast<TaskQueries*>(this);
            self->m_findAll = self->createTaskQuery();
        }

        m_findAll->setFetchFunction([this] (const TaskQuery::AddFunction &add) {
            CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                           StorageInterface::Recursive,
                                                                           StorageInterface::Tasks);
            Utils::JobHandler::install(job->kjob(), [this, job, add] {
                if (job->kjob()->error() != KJob::NoError)
                    return;

                for (auto collection : job->collections()) {
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
            return m_serializer->isTaskItem(item);
        });
    }

    return m_findAll->result();
}

TaskQueries::TaskResult::Ptr TaskQueries::findChildren(Domain::Task::Ptr task) const
{
    Akonadi::Item item = m_serializer->createItemFromTask(task);   
    if (!m_findChildren.contains(item.id())) {
        TaskQuery::Ptr query;

        {
            TaskQueries *self = const_cast<TaskQueries*>(this);
            query = self->createTaskQuery();
            self->m_findChildren.insert(item.id(), query);
        }

        query->setFetchFunction([this, item] (const TaskQuery::AddFunction &add) {
            ItemFetchJobInterface *job = m_storage->fetchItem(item);
            Utils::JobHandler::install(job->kjob(), [this, job, add] {
                if (job->kjob()->error() != KJob::NoError)
                    return;

                Q_ASSERT(job->items().size() == 1);
                auto item = job->items()[0];
                Q_ASSERT(item.parentCollection().isValid());
                ItemFetchJobInterface *job = m_storage->fetchItems(item.parentCollection());
                Utils::JobHandler::install(job->kjob(), [this, job, add] {
                    if (job->kjob()->error() != KJob::NoError)
                        return;

                    for (auto item : job->items())
                        add(item);
                });
            });
        });
        query->setPredicateFunction([this, task] (const Akonadi::Item &item) {
            return m_serializer->isTaskChild(task, item);
        });
    }

    return m_findChildren.value(item.id())->result();
}

TaskQueries::TaskResult::Ptr TaskQueries::findTopLevel() const
{
    if (!m_findTopLevel) {
        {
            TaskQueries *self = const_cast<TaskQueries*>(this);
            self->m_findTopLevel = self->createTaskQuery();
        }

        m_findTopLevel->setFetchFunction([this] (const TaskQuery::AddFunction &add) {
            CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                           StorageInterface::Recursive,
                                                                           StorageInterface::Tasks);
            Utils::JobHandler::install(job->kjob(), [this, job, add] {
                if (job->kjob()->error() != KJob::NoError)
                    return;

                for (auto collection : job->collections()) {
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
        m_findTopLevel->setPredicateFunction([this] (const Akonadi::Item &item) {
            return m_serializer->relatedUidFromItem(item).isEmpty() && m_serializer->isTaskItem(item);
        });
    }

    return m_findTopLevel->result();
}

TaskQueries::TaskResult::Ptr TaskQueries::findWorkdayTopLevel() const
{
    if (!m_findWorkdayTopLevel) {
        {
            TaskQueries *self = const_cast<TaskQueries*>(this);
            self->m_findWorkdayTopLevel = self->createTaskQuery();
        }

        m_findWorkdayTopLevel->setFetchFunction([this] (const TaskQuery::AddFunction &add) {
            CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                           StorageInterface::Recursive,
                                                                           StorageInterface::Tasks);
            Utils::JobHandler::install(job->kjob(), [this, job, add] {
                if (job->kjob()->error() != KJob::NoError)
                    return;

                for (auto collection : job->collections()) {
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
        m_findWorkdayTopLevel->setPredicateFunction([this] (const Akonadi::Item &item) {
            if (!m_serializer->isTaskItem(item))
                return false;

            const Domain::Task::Ptr task = m_serializer->createTaskFromItem(item);

            const QDate doneDate = task->doneDate().date();
            const QDate startDate = task->startDate().date();
            const QDate dueDate = task->dueDate().date();
            const QDate today = Utils::DateTime::currentDateTime().date();

            const bool pastStartDate = startDate.isValid() && startDate <= today;
            const bool pastDueDate = dueDate.isValid() && dueDate <= today;
            const bool todayDoneDate = doneDate == today;

            if (task->isDone())
                return todayDoneDate;
            else
                return pastStartDate || pastDueDate;

        });
    }

    return m_findWorkdayTopLevel->result();
}

TaskQueries::ContextResult::Ptr TaskQueries::findContexts(Domain::Task::Ptr task) const
{
    qFatal("Not implemented yet");
    Q_UNUSED(task);
    return ContextResult::Ptr();
}

void TaskQueries::onItemAdded(const Item &item)
{
    foreach (const TaskQuery::Ptr &query, m_taskQueries)
        query->onAdded(item);
}

void TaskQueries::onItemRemoved(const Item &item)
{
    foreach (const TaskQuery::Ptr &query, m_taskQueries)
        query->onRemoved(item);

    if (m_findChildren.contains(item.id())) {
        auto query = m_findChildren.take(item.id());
        m_taskQueries.removeAll(query);
    }
}

void TaskQueries::onItemChanged(const Item &item)
{
    foreach (const TaskQuery::Ptr &query, m_taskQueries)
        query->onChanged(item);
}

TaskQueries::TaskQuery::Ptr TaskQueries::createTaskQuery()
{
    auto query = TaskQueries::TaskQuery::Ptr::create();

    query->setConvertFunction(std::bind(&SerializerInterface::createTaskFromItem, m_serializer, _1));
    query->setUpdateFunction(std::bind(&SerializerInterface::updateTaskFromItem, m_serializer, _2, _1));
    query->setRepresentsFunction(std::bind(&SerializerInterface::representsItem, m_serializer, _2, _1));

    m_taskQueries << query;
    return query;
}
