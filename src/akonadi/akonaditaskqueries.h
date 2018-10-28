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

    typedef Domain::QueryResultProvider<Domain::Context::Ptr> ContextProvider;
    typedef Domain::QueryResult<Domain::Context::Ptr> ContextResult;

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

    TaskResult::Ptr findAll() const Q_DECL_OVERRIDE;
    TaskResult::Ptr findChildren(Domain::Task::Ptr task) const Q_DECL_OVERRIDE;
    TaskResult::Ptr findTopLevel() const Q_DECL_OVERRIDE;
    TaskResult::Ptr findInboxTopLevel() const Q_DECL_OVERRIDE;
    TaskResult::Ptr findWorkdayTopLevel() const Q_DECL_OVERRIDE;
    ContextResult::Ptr findContexts(Domain::Task::Ptr task) const Q_DECL_OVERRIDE;
    ProjectResult::Ptr findProject(Domain::Task::Ptr task) const Q_DECL_OVERRIDE;
    DataSourceResult::Ptr findDataSource(Domain::Task::Ptr task) const Q_DECL_OVERRIDE;


private slots:
    void onWorkdayPollTimeout();

private:
    SerializerInterface::Ptr m_serializer;
    Cache::Ptr m_cache;
    LiveQueryHelpers::Ptr m_helpers;
    LiveQueryIntegrator::Ptr m_integrator;
    QTimer *m_workdayPollTimer;
    mutable QDate m_today;

    mutable TaskQueryOutput::Ptr m_findAll;
    mutable QHash<Akonadi::Item::Id, TaskQueryOutput::Ptr> m_findChildren;
    mutable QHash<Akonadi::Item::Id, ProjectQueryOutput::Ptr> m_findProject;
    mutable QHash<Akonadi::Item::Id, DataSourceQueryOutput::Ptr> m_findDataSource;
    mutable TaskQueryOutput::Ptr m_findTopLevel;
    mutable TaskQueryOutput::Ptr m_findInboxTopLevel;
    mutable TaskQueryOutput::Ptr m_findWorkdayTopLevel;
};

}

#endif // AKONADI_TASKQUERIES_H
