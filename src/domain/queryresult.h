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

#include <algorithm>

#include "queryresultinterface.h"
#include "queryresultprovider.h"

namespace Domain {


template<typename InputType, typename OutputType = InputType>
class QueryResult : public QueryResultInputImpl<InputType>, public QueryResultInterface<OutputType>
{
public:
    typedef QSharedPointer<QueryResult<InputType, OutputType>> Ptr;
    typedef QWeakPointer<QueryResult<InputType, OutputType>> WeakPtr;
    typedef std::function<void(OutputType, int)> ChangeHandler;

    static Ptr create(const typename QueryResultProvider<InputType>::Ptr &provider)
    {
        Ptr result(new QueryResult<InputType, OutputType>(provider));
        QueryResultInputImpl<InputType>::registerResult(provider, result);
        return result;
    }

    static Ptr copy(const typename QueryResultInputImpl<InputType>::Ptr &other)
    {
        auto provider = QueryResultInputImpl<InputType>::retrieveProvider(other);
        return create(provider);
    }

    QList<OutputType> data() const
    {
        return dataImpl<OutputType>();
    }

    void addPreInsertHandler(const ChangeHandler &handler)
    {
        QueryResultInputImpl<InputType>::m_preInsertHandlers << handler;
    }

    void addPostInsertHandler(const ChangeHandler &handler)
    {
        QueryResultInputImpl<InputType>::m_postInsertHandlers << handler;
    }

    void addPreRemoveHandler(const ChangeHandler &handler)
    {
        QueryResultInputImpl<InputType>::m_preRemoveHandlers << handler;
    }

    void addPostRemoveHandler(const ChangeHandler &handler)
    {
        QueryResultInputImpl<InputType>::m_postRemoveHandlers << handler;
    }

    void addPreReplaceHandler(const ChangeHandler &handler)
    {
        QueryResultInputImpl<InputType>::m_preReplaceHandlers << handler;
    }

    void addPostReplaceHandler(const ChangeHandler &handler)
    {
        QueryResultInputImpl<InputType>::m_postReplaceHandlers << handler;
    }

private:
    explicit QueryResult(const typename QueryResultProvider<InputType>::Ptr &provider)
        : QueryResultInputImpl<InputType>(provider)
    {
    }

    template<typename T>
    typename std::enable_if<std::is_same<InputType, T>::value, QList<InputType>>::type
    dataImpl() const
    {
        auto provider = QueryResultInputImpl<InputType>::m_provider;
        return provider->data();
    }

    template<typename T>
    typename std::enable_if<!std::is_same<InputType, T>::value, QList<T>>::type
    dataImpl() const
    {
        auto provider = QueryResultInputImpl<InputType>::m_provider;
        QList<InputType> inputData = provider->data();
        QList<OutputType> outputData;
        std::transform(inputData.constBegin(), inputData.constEnd(),
                       std::back_inserter(outputData),
                       [] (const InputType &input) { return OutputType(input); });
        return outputData;
    }
};

}

#endif // DOMAIN_QUERYRESULT_H
