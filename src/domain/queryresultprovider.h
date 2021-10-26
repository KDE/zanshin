/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef DOMAIN_QUERYRESULTPROVIDER_H
#define DOMAIN_QUERYRESULTPROVIDER_H

#include <algorithm>
#include <functional>

#include <QList>
#include <QSharedPointer>

#include "utils/mem_fn.h"

namespace Domain {

template<typename ItemType>
class QueryResultProvider;

template<typename InputType>
class QueryResultInputImpl
{
public:
    typedef typename QueryResultProvider<InputType>::Ptr ProviderPtr;
    typedef QSharedPointer<QueryResultInputImpl<InputType> > Ptr;
    typedef QWeakPointer<QueryResultInputImpl<InputType>> WeakPtr;
    typedef std::function<void(InputType, int)> ChangeHandler;
    typedef QList<ChangeHandler> ChangeHandlerList;

    virtual ~QueryResultInputImpl() {}

protected:
    explicit QueryResultInputImpl(const ProviderPtr &provider)
        : m_provider(provider)
    {
    }

    static void registerResult(const ProviderPtr &provider, const Ptr &result)
    {
        provider->m_results << result;
    }

    static ProviderPtr retrieveProvider(const Ptr &result)
    {
        return result->m_provider;
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

    friend class QueryResultProvider<InputType>;
    ProviderPtr m_provider;
    ChangeHandlerList m_preInsertHandlers;
    ChangeHandlerList m_postInsertHandlers;
    ChangeHandlerList m_preRemoveHandlers;
    ChangeHandlerList m_postRemoveHandlers;
    ChangeHandlerList m_preReplaceHandlers;
    ChangeHandlerList m_postReplaceHandlers;
};

template<typename ItemType>
class QueryResultProvider
{
public:
    typedef QSharedPointer<QueryResultProvider<ItemType>> Ptr;
    typedef QWeakPointer<QueryResultProvider<ItemType>> WeakPtr;

    typedef QSharedPointer<QueryResultInputImpl<ItemType> > ResultPtr;
    typedef QWeakPointer<QueryResultInputImpl<ItemType>> ResultWeakPtr;
    typedef std::function<void(ItemType, int)> ChangeHandler;
    typedef QList<ChangeHandler> ChangeHandlerList;


    QueryResultProvider()
    {
    }

    QList<ItemType> data() const
    {
        return m_list;
    }

    void append(const ItemType &item)
    {
        cleanupResults();
        callChangeHandlers(item, m_list.size(),
                           Utils::mem_fn(&QueryResultInputImpl<ItemType>::preInsertHandlers));
        m_list.append(item);
        callChangeHandlers(item, m_list.size()-1,
                           Utils::mem_fn(&QueryResultInputImpl<ItemType>::postInsertHandlers));
    }

    void prepend(const ItemType &item)
    {
        cleanupResults();
        callChangeHandlers(item, 0,
                           Utils::mem_fn(&QueryResultInputImpl<ItemType>::preInsertHandlers));
        m_list.prepend(item);
        callChangeHandlers(item, 0,
                           Utils::mem_fn(&QueryResultInputImpl<ItemType>::postInsertHandlers));
    }

    void insert(int index, const ItemType &item)
    {
        cleanupResults();
        callChangeHandlers(item, index,
                           Utils::mem_fn(&QueryResultInputImpl<ItemType>::preInsertHandlers));
        m_list.insert(index, item);
        callChangeHandlers(item, index,
                           Utils::mem_fn(&QueryResultInputImpl<ItemType>::postInsertHandlers));
    }

    ItemType takeFirst()
    {
        cleanupResults();
        const ItemType item = m_list.first();
        callChangeHandlers(item, 0,
                           Utils::mem_fn(&QueryResultInputImpl<ItemType>::preRemoveHandlers));
        m_list.removeFirst();
        callChangeHandlers(item, 0,
                           Utils::mem_fn(&QueryResultInputImpl<ItemType>::postRemoveHandlers));
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
                           Utils::mem_fn(&QueryResultInputImpl<ItemType>::preRemoveHandlers));
        m_list.removeLast();
        callChangeHandlers(item, m_list.size(),
                           Utils::mem_fn(&QueryResultInputImpl<ItemType>::postRemoveHandlers));
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
                           Utils::mem_fn(&QueryResultInputImpl<ItemType>::preRemoveHandlers));
        m_list.removeAt(index);
        callChangeHandlers(item, index,
                           Utils::mem_fn(&QueryResultInputImpl<ItemType>::postRemoveHandlers));
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
                           Utils::mem_fn(&QueryResultInputImpl<ItemType>::preReplaceHandlers));
        m_list.replace(index, item);
        callChangeHandlers(item, index,
                           Utils::mem_fn(&QueryResultInputImpl<ItemType>::postReplaceHandlers));
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
                                       Utils::mem_fn(&QueryResultInputImpl<ItemType>::WeakPtr::isNull)),
                        m_results.end());
    }

    typedef std::function<ChangeHandlerList(ResultPtr)> ChangeHandlerGetter;

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

    friend class QueryResultInputImpl<ItemType>;
    QList<ItemType> m_list;
    QList<ResultWeakPtr> m_results;
};

}

#endif // DOMAIN_QUERYRESULTPROVIDER_H
