/*
 * SPDX-FileCopyrightText: 2014 Franck Arrecot <franck.arrecot@gmail.com>
   SPDX-FileCopyrightText: 2014 Rémi Benoit <r3m1.benoit@gmail.com>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
   */


#ifndef AKONADI_CONTEXTQUERIES_H
#define AKONADI_CONTEXTQUERIES_H

#include "domain/contextqueries.h"

#include "akonadi/akonadicache.h"
#include "akonadi/akonadilivequeryhelpers.h"
#include "akonadi/akonadilivequeryintegrator.h"

namespace Akonadi {

class ContextQueries : public QObject, public Domain::ContextQueries
{
    Q_OBJECT
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
