/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_TASKQUERIES_H
#define AKONADI_TASKQUERIES_H

#include "domain/taskqueries.h"

#include "akonadi/akonadicache.h"
#include "akonadi/akonadilivequeryhelpers.h"
#include "akonadi/akonadilivequeryintegrator.h"

class QTimer;

namespace Akonadi {

class TaskQueries : public QObject, public Domain::TaskQueries
{
    Q_OBJECT
public:
    typedef QSharedPointer<TaskQueries> Ptr;

    typedef Domain::LiveQueryInput<Akonadi::Item> ItemInputQuery;
    typedef Domain::LiveQueryOutput<Domain::Task::Ptr> TaskQueryOutput;
    typedef Domain::QueryResultProvider<Domain::Task::Ptr> TaskProvider;
    typedef Domain::QueryResult<Domain::Task::Ptr> TaskResult;

    typedef Domain::QueryResult<Domain::Context::Ptr> ContextResult;
    typedef Domain::LiveQueryOutput<Domain::Context::Ptr> ContextQueryOutput;

    typedef Domain::QueryResult<Domain::Project::Ptr> ProjectResult;
    typedef Domain::LiveQueryOutput<Domain::Project::Ptr> ProjectQueryOutput;

    typedef Domain::QueryResult<Domain::DataSource::Ptr> DataSourceResult;
    typedef Domain::LiveQueryOutput<Domain::DataSource::Ptr> DataSourceQueryOutput;

    TaskQueries(const StorageInterface::Ptr &storage,
                const SerializerInterface::Ptr &serializer,
                const MonitorInterface::Ptr &monitor,
                const Cache::Ptr &cache);

    int workdayPollInterval() const;
    void setWorkdayPollInterval(int interval);

    TaskResult::Ptr findAll() const override;
    TaskResult::Ptr findChildren(Domain::Task::Ptr task) const override;
    TaskResult::Ptr findTopLevel() const override;
    TaskResult::Ptr findInboxTopLevel() const override;
    TaskResult::Ptr findWorkdayTopLevel() const override;
    ContextResult::Ptr findContexts(Domain::Task::Ptr task) const override;
    ProjectResult::Ptr findProject(Domain::Task::Ptr task) const override;
    DataSourceResult::Ptr findDataSource(Domain::Task::Ptr task) const override;


private Q_SLOTS:
    void onWorkdayPollTimeout();

private:
    SerializerInterface::Ptr m_serializer;
    MonitorInterface::Ptr m_monitor;
    Cache::Ptr m_cache;
    LiveQueryHelpers::Ptr m_helpers;
    LiveQueryIntegrator::Ptr m_integrator;
    QTimer *m_workdayPollTimer;
    mutable QDate m_today;

    mutable TaskQueryOutput::Ptr m_findAll;
    mutable QHash<Akonadi::Item::Id, TaskQueryOutput::Ptr> m_findChildren;
    mutable QHash<Akonadi::Item::Id, ProjectQueryOutput::Ptr> m_findProject;
    mutable QHash<Akonadi::Item::Id, ContextQueryOutput::Ptr> m_findContexts;
    mutable QHash<Akonadi::Item::Id, Akonadi::Item> m_findContextsItem;
    mutable QHash<Akonadi::Item::Id, DataSourceQueryOutput::Ptr> m_findDataSource;
    mutable TaskQueryOutput::Ptr m_findTopLevel;
    mutable TaskQueryOutput::Ptr m_findInboxTopLevel;
    mutable TaskQueryOutput::Ptr m_findWorkdayTopLevel;
};

}

#endif // AKONADI_TASKQUERIES_H
