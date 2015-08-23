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

#include "domain/contextqueries.h"

#include <Akonadi/Item>

#include "akonadi/akonadimonitorinterface.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

#include "domain/livequery.h"

namespace Akonadi {

class ContextQueries : public QObject, public Domain::ContextQueries
{
    Q_OBJECT
public:
    typedef QSharedPointer<ContextQueries> Ptr;

    typedef Domain::LiveQuery<Akonadi::Item, Domain::Task::Ptr> TaskQuery;
    typedef Domain::LiveQueryInput<Akonadi::Item> ItemInputQuery;
    typedef Domain::QueryResult<Domain::Task::Ptr> TaskResult;
    typedef Domain::QueryResultProvider<Domain::Task::Ptr> TaskProvider;

    typedef Domain::LiveQuery<Akonadi::Tag, Domain::Context::Ptr> ContextQuery;
    typedef Domain::LiveQueryInput<Akonadi::Tag> TagInputQuery;
    typedef Domain::QueryResult<Domain::Context::Ptr> ContextResult;
    typedef Domain::QueryResultProvider<Domain::Context::Ptr> ContextProvider;

    ContextQueries(const StorageInterface::Ptr &storage,
                   const SerializerInterface::Ptr &serializer,
                   const MonitorInterface::Ptr &monitor);


    ContextResult::Ptr findAll() const Q_DECL_OVERRIDE;
    TaskResult::Ptr findTopLevelTasks(Domain::Context::Ptr context) const Q_DECL_OVERRIDE;

private slots:
    void onTagAdded(const Akonadi::Tag &tag);
    void onTagRemoved(const Akonadi::Tag &tag);
    void onTagChanged(const Akonadi::Tag &tag);

    void onItemAdded(const Akonadi::Item &item);
    void onItemRemoved(const Akonadi::Item &item);
    void onItemChanged(const Akonadi::Item &item);

private:
    ContextQuery::Ptr createContextQuery();
    TaskQuery::Ptr createTaskQuery();

    StorageInterface::Ptr m_storage;
    SerializerInterface::Ptr m_serializer;
    MonitorInterface::Ptr m_monitor;

    ContextQuery::Ptr m_findAll;
    TagInputQuery::List m_tagInputQueries;

    QHash<Akonadi::Tag::Id, TaskQuery::Ptr> m_findToplevel;
    ItemInputQuery::List m_itemInputQueries;
};

} // akonadi namespace

#endif // AKONADI_CONTEXTQUERIES_H
