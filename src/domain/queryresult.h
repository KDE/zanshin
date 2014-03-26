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


#ifndef DOMAIN_QUERYRESULT_H
#define DOMAIN_QUERYRESULT_H

#include <functional>

#include <QSharedPointer>

namespace Domain {

template<typename ItemType>
class QueryResultProvider;

template<typename ItemType>
class QueryResult
{
public:
    typedef QSharedPointer<QueryResult<ItemType> > Ptr;
    typedef QWeakPointer<QueryResult<ItemType>> WeakPtr;
    typedef std::function<void(ItemType, int)> ChangeHandler;
    typedef QList<ChangeHandler> ChangeHandlerList;

    QList<ItemType> data() const
    {
        return m_provider->data();
    }

    void addPreInsertHandler(const ChangeHandler &handler)
    {
        m_preInsertHandlers << handler;
    }

    void addPostInsertHandler(const ChangeHandler &handler)
    {
        m_postInsertHandlers << handler;
    }

    void addPreRemoveHandler(const ChangeHandler &handler)
    {
        m_preRemoveHandlers << handler;
    }

    void addPostRemoveHandler(const ChangeHandler &handler)
    {
        m_postRemoveHandlers << handler;
    }

    void addPreReplaceHandler(const ChangeHandler &handler)
    {
        m_preReplaceHandlers << handler;
    }

    void addPostReplaceHandler(const ChangeHandler &handler)
    {
        m_postReplaceHandlers << handler;
    }

private:
    QueryResult(const typename QueryResultProvider<ItemType>::Ptr &provider)
        : m_provider(provider)
    {
    }

    // cppcheck can't figure out the friend class
    // cppcheck-suppress unusedPrivateFunction
    ChangeHandlerList preInsertHandlers() const
    {
        return m_preInsertHandlers;
    }

    // cppcheck can't figure out the friend class
    // cppcheck-suppress unusedPrivateFunction
    ChangeHandlerList postInsertHandlers() const
    {
        return m_postInsertHandlers;
    }

    // cppcheck can't figure out the friend class
    // cppcheck-suppress unusedPrivateFunction
    ChangeHandlerList preRemoveHandlers() const
    {
        return m_preRemoveHandlers;
    }

    // cppcheck can't figure out the friend class
    // cppcheck-suppress unusedPrivateFunction
    ChangeHandlerList postRemoveHandlers() const
    {
        return m_postRemoveHandlers;
    }

    // cppcheck can't figure out the friend class
    // cppcheck-suppress unusedPrivateFunction
    ChangeHandlerList preReplaceHandlers() const
    {
        return m_preReplaceHandlers;
    }

    // cppcheck can't figure out the friend class
    // cppcheck-suppress unusedPrivateFunction
    ChangeHandlerList postReplaceHandlers() const
    {
        return m_postReplaceHandlers;
    }

    friend class QueryResultProvider<ItemType>;
    typename QueryResultProvider<ItemType>::Ptr m_provider;
    ChangeHandlerList m_preInsertHandlers;
    ChangeHandlerList m_postInsertHandlers;
    ChangeHandlerList m_preRemoveHandlers;
    ChangeHandlerList m_postRemoveHandlers;
    ChangeHandlerList m_preReplaceHandlers;
    ChangeHandlerList m_postReplaceHandlers;
};

}

#endif // DOMAIN_QUERYRESULT_H
