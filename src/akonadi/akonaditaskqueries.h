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

#ifndef AKONADI_TASKQUERIES_H
#define AKONADI_TASKQUERIES_H

#include <functional>

#include <QHash>
#include <Akonadi/Item>

#include "domain/taskqueries.h"

class KJob;

namespace Akonadi {

class Item;
class MonitorInterface;
class SerializerInterface;
class StorageInterface;

class TaskQueries : public QObject, public Domain::TaskQueries
{
    Q_OBJECT
public:
    typedef Domain::QueryResultProvider<Domain::Task::Ptr> TaskProvider;
    typedef Domain::QueryResult<Domain::Task::Ptr> TaskResult;

    typedef Domain::QueryResultProvider<Domain::Context::Ptr> ContextProvider;
    typedef Domain::QueryResult<Domain::Context::Ptr> ContextResult;

    explicit TaskQueries(QObject *parent = 0);
    TaskQueries(StorageInterface *storage, SerializerInterface *serializer, MonitorInterface *monitor);
    virtual ~TaskQueries();

    TaskResult::Ptr findAll() const;
    TaskResult::Ptr findChildren(Domain::Task::Ptr task) const;
    TaskResult::Ptr findTopLevel() const;
    ContextResult::Ptr findContexts(Domain::Task::Ptr task) const;

private slots:
    void onItemAdded(const Akonadi::Item &item);
    void onItemRemoved(const Akonadi::Item &item);
    void onItemChanged(const Akonadi::Item &item);

private:
    bool isTaskItem(const Domain::Task::Ptr &task, const Item &item) const;
    Domain::Task::Ptr deserializeTask(const Item &item) const;
    void addItemIdInCache(const Domain::Task::Ptr &task, Akonadi::Entity::Id id) const;
    TaskProvider::Ptr childProviderFromUid(const QString &uid) const;
    void removeItemFromChildProviders(const Item &item);

    StorageInterface *m_storage;
    SerializerInterface *m_serializer;
    MonitorInterface *m_monitor;
    bool m_ownInterfaces;

    mutable TaskProvider::WeakPtr m_taskProvider;
    mutable TaskProvider::WeakPtr m_topTaskProvider;
    mutable QHash<Akonadi::Entity::Id, TaskProvider::WeakPtr> m_taskChildProviders;
    mutable QHash<QString, Akonadi::Entity::Id> m_uidtoIdCache;
    mutable QHash<Akonadi::Entity::Id, QString> m_idToRelatedUidCache;
};

}

#endif // AKONADI_TASKQUERIES_H
