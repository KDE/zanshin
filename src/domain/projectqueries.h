/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef DOMAIN_PROJECTQUERIES_H
#define DOMAIN_PROJECTQUERIES_H

#include "project.h"
#include "queryresult.h"
#include "task.h"

namespace Domain {

class ProjectQueries
{
public:
    typedef QSharedPointer<ProjectQueries> Ptr;

    ProjectQueries();
    virtual ~ProjectQueries();

    virtual QueryResult<Project::Ptr>::Ptr findAll() const = 0;
    virtual QueryResult<Task::Ptr>::Ptr findTopLevel(Project::Ptr project) const = 0;
};

}

#endif // DOMAIN_PROJECTQUERIES_H
