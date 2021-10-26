/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2014 Rémi Benoit <r3m1.benoit@gmail.com>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
   */


#ifndef DOMAIN_CONTEXTQUERIES_H
#define DOMAIN_CONTEXTQUERIES_H

#include "context.h"
#include "queryresult.h"
#include "task.h"

namespace Domain {

class ContextQueries
{
public:
    typedef QSharedPointer<ContextQueries> Ptr;

    ContextQueries();
    virtual ~ContextQueries();

    virtual QueryResult<Context::Ptr>::Ptr findAll() const = 0;

    virtual QueryResult<Task::Ptr>::Ptr findTopLevelTasks(Context::Ptr context) const = 0;
};

}

#endif // DOMAIN_CONTEXTQUERIES_H
