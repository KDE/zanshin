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

#include "utils/datetime.h"

#include <QTimer>

using namespace Akonadi;

TaskQueries::TaskQueries(const StorageInterface::Ptr &storage,
                         const SerializerInterface::Ptr &serializer,
                         const MonitorInterface::Ptr &monitor,
                         const Cache::Ptr &cache)
    : m_serializer(serializer),
      m_monitor(monitor),
      m_cache(cache),
      m_helpers(new LiveQueryHelpers(serializer, storage)),
      m_integrator(new LiveQueryIntegrator(serializer, monitor)),
      m_workdayPollTimer(new QTimer(this))
{
    m_workdayPollTimer->setInterval(30000);
    connect(m_workdayPollTimer, &QTimer::timeout, this, &TaskQueries::onWorkdayPollTimeout);

    m_integrator->addRemoveHandler([this] (const Item &item) {
        m_findChildren.remove(item.id());
        m_findContexts.remove(item.id());
    });

    connect(m_monitor.data(), &MonitorInterface::itemChanged, this, [this] (const Item &item) {
        const auto it = m_findContexts.find(item.id());
        if (it == m_findContexts.end())
            return;

        m_findContextsItem[item.id()] = item;
        (*it)->reset();
    });
}

int TaskQueries::workdayPollInterval() const
{
    return m_workdayPollTimer->interval();
}

void TaskQueries::setWorkdayPollInterval(int interval)
{
    m_workdayPollTimer->setInterval(interval);
}

TaskQueries::TaskResult::Ptr TaskQueries::findAll() const
{
    auto fetch = m_helpers->fetchItems(const_cast<TaskQueries*>(this));
    auto predicate = [this] (const Akonadi::Item &item) {
        return m_serializer->isTaskItem(item);
    };
    m_integrator->bind("TaskQueries::findAll", m_findAll, fetch, predicate);
    return m_findAll->result();
}

TaskQueries::TaskResult::Ptr TaskQueries::findChildren(Domain::Task::Ptr task) const
{
    Akonadi::Item item = m_serializer->createItemFromTask(task);
    auto &query = m_findChildren[item.id()];
    auto fetch = m_helpers->fetchSiblings(item, const_cast<TaskQueries*>(this));
    auto predicate = [this, task] (const Akonadi::Item &childItem) {
        return m_serializer->isTaskChild(task, childItem);
    };
    m_integrator->bind("TaskQueries::findChildren", query, fetch, predicate);
    return query->result();
}

TaskQueries::ProjectResult::Ptr TaskQueries::findProject(Domain::Task::Ptr task) const
{
    Akonadi::Item childItem = m_serializer->createItemFromTask(task);
    auto &query = m_findProject[childItem.id()];
    auto fetch = m_helpers->fetchTaskAndAncestors(task, const_cast<TaskQueries*>(this));
    auto predicate = [this, childItem] (const Akonadi::Item &item) {
        return m_serializer->isProjectItem(item);
    };
    auto compare = [] (const Akonadi::Item &item1, const Akonadi::Item &item2) {
        return item1.id() == item2.id();
    };
    m_integrator->bindRelationship("TaskQueries::findProject", query, fetch, compare, predicate);
    return query->result();
}

TaskQueries::DataSourceResult::Ptr TaskQueries::findDataSource(Domain::Task::Ptr task) const
{
    Akonadi::Item item = m_serializer->createItemFromTask(task);
    auto &query = m_findDataSource[item.id()];
    auto fetch = m_helpers->fetchItemCollection(item, const_cast<TaskQueries*>(this));
    auto predicate = [] (const Akonadi::Collection &) { return true; };

    m_integrator->bind("TaskQueries::findDataSource", query, fetch, predicate);
    return query->result();
}

TaskQueries::TaskResult::Ptr TaskQueries::findTopLevel() const
{
    Q_ASSERT(m_cache);
    auto fetch = m_helpers->fetchItems(const_cast<TaskQueries*>(this));
    auto predicate = [this] (const Akonadi::Item &item) {
        // Tasks with no parent, or whose parent is a project (not a task)
        if (!m_serializer->isTaskItem(item))
            return false;

        const auto items = m_cache->items(item.parentCollection());
        auto currentItem = item;
        auto parentUid = m_serializer->relatedUidFromItem(currentItem);
        while (!parentUid.isEmpty()) {
            const auto parent = std::find_if(items.cbegin(), items.cend(),
                                             [this, parentUid] (const Akonadi::Item &item) {
                                                 return m_serializer->itemUid(item) == parentUid;
                                             });
            if (parent == items.cend())
                break;

            if (m_serializer->isTaskItem(*parent))
                return false;

            currentItem = *parent;
            parentUid = m_serializer->relatedUidFromItem(currentItem);
        }
        return true;
    };
    m_integrator->bind("TaskQueries::findTopLevel", m_findTopLevel, fetch, predicate);
    return m_findTopLevel->result();
}

TaskQueries::TaskResult::Ptr TaskQueries::findInboxTopLevel() const
{
    auto fetch = m_helpers->fetchItems(const_cast<TaskQueries*>(this));
    auto predicate = [this] (const Akonadi::Item &item) {
        // Tasks without a parent (neither task nor project)
        return m_serializer->isTaskItem(item) && m_serializer->relatedUidFromItem(item).isEmpty();
    };
    m_integrator->bind("TaskQueries::findInboxTopLevel", m_findInboxTopLevel, fetch, predicate);
    return m_findInboxTopLevel->result();
}

TaskQueries::TaskResult::Ptr TaskQueries::findWorkdayTopLevel() const
{
    if (!m_findWorkdayTopLevel) {
        m_workdayPollTimer->start();
        m_today = Utils::DateTime::currentDate();
    }

    auto fetch = m_helpers->fetchItems(const_cast<TaskQueries*>(this));
    auto isWorkdayItem = [this] (const Akonadi::Item &item) {
        if (!m_serializer->isTaskItem(item))
            return false;

        const Domain::Task::Ptr task = m_serializer->createTaskFromItem(item);

        const QDate doneDate = task->doneDate();
        const QDate startDate = task->startDate();
        const QDate dueDate = task->dueDate();
        const QDate today = Utils::DateTime::currentDate();

        const bool pastStartDate = startDate.isValid() && startDate <= today;
        const bool pastDueDate = dueDate.isValid() && dueDate <= today;
        const bool todayDoneDate = doneDate == today;

        if (task->isDone())
            return todayDoneDate;
        else
            return pastStartDate || pastDueDate;
    };
    auto predicate = [this, isWorkdayItem] (const Akonadi::Item &item) {
        if (!isWorkdayItem(item))
            return false;

        const auto items = m_cache->items(item.parentCollection());
        auto currentItem = item;
        auto parentUid = m_serializer->relatedUidFromItem(currentItem);
        while (!parentUid.isEmpty()) {
            const auto parent = std::find_if(items.cbegin(), items.cend(),
                                             [this, parentUid] (const Akonadi::Item &item) {
                                                 return m_serializer->itemUid(item) == parentUid;
                                             });
            if (parent == items.cend())
                break;

            if (isWorkdayItem(*parent))
                return false;

            currentItem = *parent;
            parentUid = m_serializer->relatedUidFromItem(currentItem);
        }

        return true;
    };
    m_integrator->bind("TaskQueries::findWorkdayTopLevel", m_findWorkdayTopLevel, fetch, predicate);
    return m_findWorkdayTopLevel->result();
}

TaskQueries::ContextResult::Ptr TaskQueries::findContexts(Domain::Task::Ptr task) const
{
    Akonadi::Item taskItem = m_serializer->createItemFromTask(task);
    const auto taskItemId = taskItem.id();
    m_findContextsItem[taskItemId] = taskItem;

    auto &query = m_findContexts[taskItemId];
    auto fetch = m_helpers->fetchItems(const_cast<TaskQueries*>(this));
    auto predicate = [this, taskItemId] (const Akonadi::Item &contextItem) {
        auto context = m_serializer->createContextFromItem(contextItem);
        if (!context)
            return false;

        const auto taskItem = m_findContextsItem[taskItemId];
        return m_serializer->isContextChild(context, taskItem);
    };
    m_integrator->bind("TaskQueries::findContexts", query, fetch, predicate);
    return query->result();
}

void TaskQueries::onWorkdayPollTimeout()
{
    auto newDate = Utils::DateTime::currentDate();
    if (m_findWorkdayTopLevel && m_today != newDate) {
        m_today = newDate;
        m_findWorkdayTopLevel->reset();
    }
}
