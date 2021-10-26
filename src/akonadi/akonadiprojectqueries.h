/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_PROJECTQUERIES_H
#define AKONADI_PROJECTQUERIES_H

#include "domain/projectqueries.h"

#include "akonadi/akonadilivequeryhelpers.h"
#include "akonadi/akonadilivequeryintegrator.h"

namespace Akonadi {

class ProjectQueries : public QObject, public Domain::ProjectQueries
{
    Q_OBJECT
public:
    typedef QSharedPointer<ProjectQueries> Ptr;

    typedef Domain::LiveQueryInput<Akonadi::Item> ItemInputQuery;
    typedef Domain::LiveQueryOutput<Domain::Project::Ptr> ProjectQueryOutput;
    typedef Domain::QueryResultProvider<Domain::Project::Ptr> ProjectProvider;
    typedef Domain::QueryResult<Domain::Project::Ptr> ProjectResult;

    typedef Domain::LiveQueryOutput<Domain::Task::Ptr> TaskQueryOutput;
    typedef Domain::QueryResultProvider<Domain::Task::Ptr> TaskProvider;
    typedef Domain::QueryResult<Domain::Task::Ptr> TaskResult;

    ProjectQueries(const StorageInterface::Ptr &storage,
                   const SerializerInterface::Ptr &serializer,
                   const MonitorInterface::Ptr &monitor);

    ProjectResult::Ptr findAll() const override;
    TaskResult::Ptr findTopLevel(Domain::Project::Ptr project) const override;

private:
    SerializerInterface::Ptr m_serializer;
    LiveQueryHelpers::Ptr m_helpers;
    LiveQueryIntegrator::Ptr m_integrator;

    mutable ProjectQueryOutput::Ptr m_findAll;
    mutable QHash<Akonadi::Item::Id, TaskQueryOutput::Ptr> m_findTopLevel;
};

}

#endif // AKONADI_PROJECTQUERIES_H
