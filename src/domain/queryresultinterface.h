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
