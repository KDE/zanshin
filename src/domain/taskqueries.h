/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef DOMAIN_TASKQUERIES_H
#define DOMAIN_TASKQUERIES_H

#include "context.h"
#include "queryresult.h"
#include "task.h"
#include "project.h"
#include "datasource.h"

namespace Domain {

class TaskQueries
{
public:
    typedef QSharedPointer<TaskQueries> Ptr;

    TaskQueries();
    virtual ~TaskQueries();

    virtual QueryResult<Task::Ptr>::Ptr findAll() const = 0;

    virtual QueryResult<Task::Ptr>::Ptr findChildren(Task::Ptr task) const = 0;

    virtual QueryResult<Task::Ptr>::Ptr findTopLevel() const = 0;

    virtual QueryResult<Task::Ptr>::Ptr findInboxTopLevel() const = 0;

    virtual QueryResult<Task::Ptr>::Ptr findWorkdayTopLevel() const = 0;

    virtual QueryResult<Context::Ptr>::Ptr findContexts(Task::Ptr task) const = 0;

    virtual QueryResult<Project::Ptr>::Ptr findProject(Task::Ptr task) const = 0;

    virtual QueryResult<DataSource::Ptr>::Ptr findDataSource(Task::Ptr task) const = 0;
};

}

#endif // DOMAIN_TASKQUERIES_H
