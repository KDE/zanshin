/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
        if (!other)
            return Ptr();

        auto provider = QueryResultInputImpl<InputType>::retrieveProvider(other);
        return create(provider);
    }

    QList<OutputType> data() const override
    {
        return dataImpl<OutputType>();
    }

    void addPreInsertHandler(const ChangeHandler &handler) override
    {
        QueryResultInputImpl<InputType>::m_preInsertHandlers << handler;
    }

    void addPostInsertHandler(const ChangeHandler &handler) override
    {
        QueryResultInputImpl<InputType>::m_postInsertHandlers << handler;
    }

    void addPreRemoveHandler(const ChangeHandler &handler) override
    {
        QueryResultInputImpl<InputType>::m_preRemoveHandlers << handler;
    }

    void addPostRemoveHandler(const ChangeHandler &handler) override
    {
        QueryResultInputImpl<InputType>::m_postRemoveHandlers << handler;
    }

    void addPreReplaceHandler(const ChangeHandler &handler) override
    {
        QueryResultInputImpl<InputType>::m_preReplaceHandlers << handler;
    }

    void addPostReplaceHandler(const ChangeHandler &handler) override
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
