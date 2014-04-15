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


#ifndef DOMAIN_QUERYRESULTPROVIDER_H
#define DOMAIN_QUERYRESULTPROVIDER_H

#include <algorithm>

#include <QList>
#include <QSharedPointer>

#include "queryresult.h"

namespace Domain {

template<typename ItemType>
class QueryResultProvider
{
public:
    typedef QSharedPointer<QueryResultProvider<ItemType>> Ptr;
    typedef QWeakPointer<QueryResultProvider<ItemType>> WeakPtr;

    QueryResultProvider()
    {
    }

    static typename QueryResult<ItemType>::Ptr createResult(const Ptr &provider)
    {
        typename QueryResult<ItemType>::Ptr result(new QueryResult<ItemType>(provider));
        provider->m_results << result;
        return result;
    }

    QList<ItemType> data() const
    {
        return m_list;
    }

    void append(const ItemType &item)
    {
        cleanupResults();
        callChangeHandlers(item, m_list.size(),
                           std::mem_fn(&QueryResult<ItemType>::preInsertHandlers));
        m_list.append(item);
        callChangeHandlers(item, m_list.size()-1,
                           std::mem_fn(&QueryResult<ItemType>::postInsertHandlers));
    }

    void prepend(const ItemType &item)
    {
        cleanupResults();
        callChangeHandlers(item, 0,
                           std::mem_fn(&QueryResult<ItemType>::preInsertHandlers));
        m_list.prepend(item);
        callChangeHandlers(item, 0,
                           std::mem_fn(&QueryResult<ItemType>::postInsertHandlers));
    }

    void insert(int index, const ItemType &item)
    {
        cleanupResults();
        callChangeHandlers(item, index,
                           std::mem_fn(&QueryResult<ItemType>::preInsertHandlers));
        m_list.insert(index, item);
        callChangeHandlers(item, index,
                           std::mem_fn(&QueryResult<ItemType>::postInsertHandlers));
    }

    ItemType takeFirst()
    {
        cleanupResults();
        const ItemType item = m_list.first();
        callChangeHandlers(item, 0,
                           std::mem_fn(&QueryResult<ItemType>::preRemoveHandlers));
        m_list.removeFirst();
        callChangeHandlers(item, 0,
                           std::mem_fn(&QueryResult<ItemType>::postRemoveHandlers));
        return item;
    }

    void removeFirst()
    {
        takeFirst();
    }

    ItemType takeLast()
    {
        cleanupResults();
        const ItemType item = m_list.last();
        callChangeHandlers(item, m_list.size()-1,
                           std::mem_fn(&QueryResult<ItemType>::preRemoveHandlers));
        m_list.removeLast();
        callChangeHandlers(item, m_list.size(),
                           std::mem_fn(&QueryResult<ItemType>::postRemoveHandlers));
        return item;
    }

    void removeLast()
    {
        takeLast();
    }

    ItemType takeAt(int index)
    {
        cleanupResults();
        const ItemType item = m_list.at(index);
        callChangeHandlers(item, index,
                           std::mem_fn(&QueryResult<ItemType>::preRemoveHandlers));
        m_list.removeAt(index);
        callChangeHandlers(item, index,
                           std::mem_fn(&QueryResult<ItemType>::postRemoveHandlers));
        return item;
    }

    void removeAt(int index)
    {
        takeAt(index);
    }

    void replace(int index, const ItemType &item)
    {
        cleanupResults();
        callChangeHandlers(m_list.at(index), index,
                           std::mem_fn(&QueryResult<ItemType>::preReplaceHandlers));
        m_list.replace(index, item);
        callChangeHandlers(item, index,
                           std::mem_fn(&QueryResult<ItemType>::postReplaceHandlers));
    }

    QueryResultProvider &operator<< (const ItemType &item)
    {
        append(item);
        return *this;
    }

private:
    void cleanupResults()
    {
        m_results.erase(std::remove_if(m_results.begin(),
                                       m_results.end(),
                                       std::mem_fn(&QueryResult<ItemType>::WeakPtr::isNull)),
                        m_results.end());
    }

    typedef std::function<typename QueryResult<ItemType>::ChangeHandlerList(typename QueryResult<ItemType>::Ptr)> ChangeHandlerGetter;

    void callChangeHandlers(const ItemType &item, int index, const ChangeHandlerGetter &handlerGetter)
    {
        for (auto weakResult : m_results)
        {
            auto result = weakResult.toStrongRef();
            if (!result) continue;
            for (auto handler : handlerGetter(result))
            {
                handler(item, index);
            }
        }
    }

    QList<ItemType> m_list;
    QList<typename QueryResult<ItemType>::WeakPtr> m_results;
};

}

#endif // DOMAIN_QUERYRESULTPROVIDER_H
