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

    virtual KJob *delegate(Task::Ptr task, Task::Delegate delegate) = 0;
};

}

#endif // DOMAIN_TASKREPOSITORY_H
