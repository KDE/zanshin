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

#include "akonadi/akonadicache.h"
#include "akonadi/akonadilivequeryhelpers.h"
#include "akonadi/akonadilivequeryintegrator.h"

namespace Akonadi {

class ContextQueries : public Domain::ContextQueries
{
public:
    typedef QSharedPointer<ContextQueries> Ptr;

    typedef Domain::LiveQueryInput<Akonadi::Item> ItemInputQuery;
    typedef Domain::LiveQueryOutput<Domain::Task::Ptr> TaskQueryOutput;
    typedef Domain::QueryResult<Domain::Task::Ptr> TaskResult;
    typedef Domain::QueryResultProvider<Domain::Task::Ptr> TaskProvider;

    typedef Domain::LiveQueryOutput<Domain::Context::Ptr> ContextQueryOutput;
    typedef Domain::QueryResult<Domain::Context::Ptr> ContextResult;
    typedef Domain::QueryResultProvider<Domain::Context::Ptr> ContextProvider;

    ContextQueries(const StorageInterface::Ptr &storage,
                   const SerializerInterface::Ptr &serializer,
                   const MonitorInterface::Ptr &monitor,
                   const Cache::Ptr &cache);


    ContextResult::Ptr findAll() const override;
    TaskResult::Ptr findTopLevelTasks(Domain::Context::Ptr context) const override;

private:
    SerializerInterface::Ptr m_serializer;
    Cache::Ptr m_cache;
    LiveQueryHelpers::Ptr m_helpers;
    LiveQueryIntegrator::Ptr m_integrator;

    mutable ContextQueryOutput::Ptr m_findAll;
    using ContextUid = QString;
    mutable QHash<ContextUid, TaskQueryOutput::Ptr> m_findToplevel;
};

} // akonadi namespace

#endif // AKONADI_CONTEXTQUERIES_H
