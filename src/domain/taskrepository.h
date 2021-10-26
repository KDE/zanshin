/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef DOMAIN_TASKREPOSITORY_H
#define DOMAIN_TASKREPOSITORY_H

#include "context.h"
#include "datasource.h"
#include "project.h"
#include "task.h"

class KJob;

namespace Domain {

class TaskRepository
{
public:
    typedef QSharedPointer<TaskRepository> Ptr;

    TaskRepository();
    virtual ~TaskRepository();

    virtual KJob *create(Task::Ptr task) = 0;
    virtual KJob *createChild(Domain::Task::Ptr task, Domain::Task::Ptr parent) = 0;
    virtual KJob *createInProject(Task::Ptr task, Project::Ptr project) = 0;
    virtual KJob *createInContext(Task::Ptr task, Context::Ptr context) = 0;

    virtual KJob *update(Task::Ptr task) = 0;
    virtual KJob *remove(Task::Ptr task) = 0;

    virtual KJob *promoteToProject(Task::Ptr task) = 0;

    virtual KJob *associate(Task::Ptr parent, Task::Ptr child) = 0;
    virtual KJob *dissociate(Task::Ptr child) = 0;
    virtual KJob *dissociateAll(Task::Ptr child) = 0;
};

}

#endif // DOMAIN_TASKREPOSITORY_H
