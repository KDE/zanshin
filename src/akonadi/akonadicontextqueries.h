/* This file is part of Zanshin

   Copyright 2014 Franck Arrecot <franck.arrecot@gmail.com>
   Copyright 2014 Rémi Benoit <r3m1.benoit@gmail.com>

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

#ifndef AKONADI_CONTEXTQUERIES_H
#define AKONADI_CONTEXTQUERIES_H

#include <Akonadi/Item>

#include "domain/contextqueries.h"
#include "domain/livequery.h"

namespace Akonadi {

class MonitorInterface;
class SerializerInterface;
class StorageInterface;

class ContextQueries : public QObject, public Domain::ContextQueries
{
    Q_OBJECT
public:
    typedef Domain::QueryResult<Domain::Task::Ptr> TaskResult;
    typedef Domain::QueryResultProvider<Domain::Task::Ptr> TaskProvider;

    typedef Domain::LiveQuery<Akonadi::Tag, Domain::Context::Ptr> ContextQuery;
    typedef Domain::QueryResult<Domain::Context::Ptr> ContextResult;
    typedef Domain::QueryResultProvider<Domain::Context::Ptr> ContextProvider;

    explicit ContextQueries(QObject *parent = 0);
    ContextQueries(StorageInterface *storage, SerializerInterface *serializer, MonitorInterface *monitor);
    virtual ~ContextQueries();


    ContextResult::Ptr findAll() const;
    TaskResult::Ptr findTopLevelTasks(Domain::Context::Ptr context) const;

private slots:
    void onTagAdded(const Akonadi::Tag &tag);
    void onTagRemoved(const Akonadi::Tag &tag);
    void onTagChanged(const Akonadi::Tag &tag);

private:
    ContextQuery::Ptr createContextQuery();

    StorageInterface *m_storage;
    SerializerInterface *m_serializer;
    MonitorInterface *m_monitor;
    bool m_ownInterfaces;

    ContextQuery::Ptr m_findAll;
    ContextQuery::List m_contextQueries;
};

} // akonadi namespace

#endif // AKONADI_CONTEXTQUERIES_H
