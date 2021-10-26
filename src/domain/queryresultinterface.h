/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef DOMAIN_QUERYRESULTINTERFACE_H
#define DOMAIN_QUERYRESULTINTERFACE_H

#include <functional>

#include <QSharedPointer>

namespace Domain {

template<typename OutputType>
class QueryResultInterface
{
public:
    typedef QSharedPointer<QueryResultInterface<OutputType>> Ptr;
    typedef QWeakPointer<QueryResultInterface<OutputType>> WeakPtr;
    typedef std::function<void(OutputType, int)> ChangeHandler;

    virtual ~QueryResultInterface() {}

    virtual QList<OutputType> data() const = 0;

    virtual void addPreInsertHandler(const ChangeHandler &handler) = 0;
    virtual void addPostInsertHandler(const ChangeHandler &handler) = 0;
    virtual void addPreRemoveHandler(const ChangeHandler &handler) = 0;
    virtual void addPostRemoveHandler(const ChangeHandler &handler) = 0;
    virtual void addPreReplaceHandler(const ChangeHandler &handler) = 0;
    virtual void addPostReplaceHandler(const ChangeHandler &handler) = 0;
};

}

#endif // DOMAIN_QUERYRESULTINTERFACE_H
