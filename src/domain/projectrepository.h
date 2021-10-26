/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef DOMAIN_PROJECTREPOSITORY_H
#define DOMAIN_PROJECTREPOSITORY_H

#include "task.h"
#include "datasource.h"
#include "project.h"

class KJob;

namespace Domain {

class ProjectRepository
{
public:
    typedef QSharedPointer<ProjectRepository> Ptr;

    ProjectRepository();
    virtual ~ProjectRepository();

    virtual KJob *create(Project::Ptr project, DataSource::Ptr source) = 0;
    virtual KJob *update(Project::Ptr project) = 0;
    virtual KJob *remove(Project::Ptr project) = 0;

    virtual KJob *associate(Project::Ptr parent, Task::Ptr child) = 0;
    virtual KJob *dissociate(Task::Ptr child) = 0;
};

}

#endif // DOMAIN_PROJECTREPOSITORY_H
