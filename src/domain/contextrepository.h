/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef DOMAIN_CONTEXTREPOSITORY_H
#define DOMAIN_CONTEXTREPOSITORY_H

#include "context.h"
#include "datasource.h"
#include "task.h"

class KJob;

namespace Domain {

class ContextRepository
{
public:
    typedef QSharedPointer<ContextRepository> Ptr;

    ContextRepository();
    virtual ~ContextRepository();

    virtual KJob *create(Context::Ptr context, DataSource::Ptr source) = 0;
    virtual KJob *update(Context::Ptr context) = 0;
    virtual KJob *remove(Context::Ptr context) = 0;

    virtual KJob *associate(Context::Ptr parent, Task::Ptr child) = 0;
    virtual KJob *dissociate(Context::Ptr parent, Task::Ptr child) = 0;
    virtual KJob *dissociateAll(Task::Ptr child) = 0;
};

}

#endif // DOMAIN_CONTEXTREPOSITORY_H
