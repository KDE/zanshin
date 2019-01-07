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


#ifndef DOMAIN_LIVEQUERY_H
#define DOMAIN_LIVEQUERY_H

#include "queryresult.h"

namespace Domain {

template <typename InputType>
class LiveQueryInput
{
public:
    typedef QSharedPointer<LiveQueryInput<InputType>> Ptr;
    typedef QWeakPointer<LiveQueryInput<InputType>> WeakPtr;
    typedef QList<Ptr> List;
    typedef QList<WeakPtr> WeakList;

    typedef std::function<void(const InputType &)> AddFunction;
    typedef std::function<void(const AddFunction &)> FetchFunction;
    typedef std::function<bool(const InputType &)> PredicateFunction;

    virtual ~LiveQueryInput() {}

    virtual void reset() = 0;
    virtual void onAdded(const InputType &input) = 0;
    virtual void onChanged(const InputType &input) = 0;
    virtual void onRemoved(const InputType &input) = 0;
};

template <typename OutputType>
class LiveQueryOutput
{
public:
    typedef QSharedPointer<LiveQueryOutput<OutputType>> Ptr;
    typedef QList<Ptr> List;
    typedef QueryResult<OutputType> Result;

    virtual ~LiveQueryOutput() {}
    virtual typename Result::Ptr result() = 0;
    virtual void reset() = 0;
};

template<typename InputType, typename OutputType>
class LiveQuery : public LiveQueryInput<InputType>, public LiveQueryOutput<OutputType>
{
public:
    typedef QSharedPointer<LiveQuery<InputType, OutputType>> Ptr;
    typedef QList<Ptr> List;

    typedef QueryResultProvider<OutputType> Provider;
    typedef QueryResult<OutputType> Result;

    typedef typename LiveQueryInput<InputType>::AddFunction AddFunction;
    typedef typename LiveQueryInput<InputType>::FetchFunction FetchFunction;
    typedef typename LiveQueryInput<InputType>::PredicateFunction PredicateFunction;

    typedef std::function<OutputType(const InputType &)> ConvertFunction;
    typedef std::function<void(const InputType &, OutputType &)> UpdateFunction;
    typedef std::function<bool(const InputType &, const OutputType &)> RepresentsFunction;

    LiveQuery() = default;
    LiveQuery(const LiveQuery &other) = default;
    LiveQuery &operator=(const LiveQuery &other) = default;

    ~LiveQuery()
    {
        clear();
    }

    typename Result::Ptr result() override
    {
        typename Provider::Ptr provider(m_provider.toStrongRef());

        if (provider)
            return Result::create(provider);

        provider = Provider::Ptr::create();
        m_provider = provider.toWeakRef();

        doFetch();

        return Result::create(provider);
    }

    void setFetchFunction(const FetchFunction &fetch)
    {
        m_fetch = fetch;
    }

    void setPredicateFunction(const PredicateFunction &predicate)
    {
        m_predicate = predicate;
    }

    void setConvertFunction(const ConvertFunction &convert)
    {
        m_convert = convert;
    }

    void setUpdateFunction(const UpdateFunction &update)
    {
        m_update = update;
    }

    void setDebugName(const QByteArray &name)
    {
        m_debugName = name;
    }

    void setRepresentsFunction(const RepresentsFunction &represents)
    {
        m_represents = represents;
    }

    void reset() override
    {
        clear();
        doFetch();
    }

    void onAdded(const InputType &input) override
    {
        typename Provider::Ptr provider(m_provider.toStrongRef());

        if (!provider)
            return;

        if (m_predicate(input))
            addToProvider(provider, input);
    }

    void onChanged(const InputType &input) override
    {
        typename Provider::Ptr provider(m_provider.toStrongRef());

        if (!provider)
            return;

        if (!m_predicate(input)) {
            for (int i = 0; i < provider->data().size(); i++) {
                auto output = provider->data().at(i);
                if (m_represents(input, output)) {
                    provider->removeAt(i);
                    i--;
                }
            }
        } else {
            bool found = false;

            for (int i = 0; i < provider->data().size(); i++) {
                auto output = provider->data().at(i);
                if (m_represents(input, output)) {
                    m_update(input, output);
                    provider->replace(i, output);

                    found = true;
                }
            }

            if (!found)
                addToProvider(provider, input);
        }
    }

    void onRemoved(const InputType &input) override
    {
        typename Provider::Ptr provider(m_provider.toStrongRef());

        if (!provider)
            return;

        for (int i = 0; i < provider->data().size(); i++) {
            auto output = provider->data().at(i);
            if (m_represents(input, output)) {
                provider->removeAt(i);
                i--;
            }
        }
    }

private:
    template<typename T>
    bool isValidOutput(const T &/*output*/)
    {
        return true;
    }

    template<typename T>
    bool isValidOutput(const QSharedPointer<T> &output)
    {
        return !output.isNull();
    }

    template<typename T>
    bool isValidOutput(T *output)
    {
        return output != nullptr;
    }

    void addToProvider(const typename Provider::Ptr &provider, const InputType &input)
    {
        auto output = m_convert(input);
        if (isValidOutput(output))
            provider->append(output);
    }

    void doFetch()
    {
        typename Provider::Ptr provider(m_provider.toStrongRef());

        if (!provider)
            return;

        auto addFunction = [this, provider] (const InputType &input) {
            if (m_predicate(input))
                addToProvider(provider, input);
        };

        m_fetch(addFunction);
    }

    void clear()
    {
        typename Provider::Ptr provider(m_provider.toStrongRef());

        if (!provider)
            return;

        while (!provider->data().isEmpty())
            provider->removeFirst();
    }

    FetchFunction m_fetch;
    PredicateFunction m_predicate;
    ConvertFunction m_convert;
    UpdateFunction m_update;
    RepresentsFunction m_represents;
    QByteArray m_debugName;

    typename Provider::WeakPtr m_provider;
};

// A query that stores an intermediate list of results (from the fetch), to react on changes on any item in that list
// and then filters that list with the predicate for the final result
// When one of the intermediary items changes, a full fetch is done again.
template<typename InputType, typename OutputType>
class LiveRelationshipQuery : public LiveQueryInput<InputType>, public LiveQueryOutput<OutputType>
{
public:
    typedef QSharedPointer<LiveRelationshipQuery<InputType, OutputType>> Ptr;
    typedef QList<Ptr> List;

    typedef QueryResultProvider<OutputType> Provider;
    typedef QueryResult<OutputType> Result;

    typedef typename LiveQueryInput<InputType>::AddFunction AddFunction;
    typedef typename LiveQueryInput<InputType>::FetchFunction FetchFunction;
    typedef typename LiveQueryInput<InputType>::PredicateFunction PredicateFunction;

    typedef std::function<OutputType(const InputType &)> ConvertFunction;
    typedef std::function<bool(const InputType &, const OutputType &)> RepresentsFunction;
    typedef std::function<bool(const InputType &, const InputType &)> CompareFunction;

    LiveRelationshipQuery() = default;
    LiveRelationshipQuery(const LiveRelationshipQuery &other) = default;
    LiveRelationshipQuery &operator=(const LiveRelationshipQuery &other) = default;

    ~LiveRelationshipQuery()
    {
        clear();
    }

    typename Result::Ptr result() override
    {
        typename Provider::Ptr provider(m_provider.toStrongRef());

        if (provider)
            return Result::create(provider);
        provider = Provider::Ptr::create();
        m_provider = provider.toWeakRef();

        doFetch();

        return Result::create(provider);
    }

    void setFetchFunction(const FetchFunction &fetch)
    {
        m_fetch = fetch;
    }

    void setPredicateFunction(const PredicateFunction &predicate)
    {
        m_predicate = predicate;
    }

    void setCompareFunction(const CompareFunction &compare)
    {
        m_compare = compare;
    }

    void setConvertFunction(const ConvertFunction &convert)
    {
        m_convert = convert;
    }

    void setDebugName(const QByteArray &name)
    {
        m_debugName = name;
    }

    void setRepresentsFunction(const RepresentsFunction &represents)
    {
        m_represents = represents;
    }

    void reset() override
    {
        clear();
        doFetch();
    }

    void onAdded(const InputType &input) override
    {
        typename Provider::Ptr provider(m_provider.toStrongRef());

        if (!provider)
            return;

        m_intermediaryResults.append(input);
        if (m_predicate(input))
            addToProvider(provider, input);
    }

    void onChanged(const InputType &input) override
    {
        Q_ASSERT(m_compare);
        const bool found = std::any_of(m_intermediaryResults.constBegin(), m_intermediaryResults.constEnd(),
                                       [&input, this](const InputType &existing) {
                                           return m_compare(input, existing);
                                       });
        if (found)
            reset();
    }

    void onRemoved(const InputType &input) override
    {
        onChanged(input);
    }

private:
    template<typename T>
    bool isValidOutput(const T &/*output*/)
    {
        return true;
    }

    template<typename T>
    bool isValidOutput(const QSharedPointer<T> &output)
    {
        return !output.isNull();
    }

    template<typename T>
    bool isValidOutput(T *output)
    {
        return output != nullptr;
    }

    void addToProvider(const typename Provider::Ptr &provider, const InputType &input)
    {
        auto output = m_convert(input);
        if (isValidOutput(output))
            provider->append(output);
    }

    void doFetch()
    {
        auto addFunction = [this] (const InputType &input) {
            onAdded(input);
        };
        m_fetch(addFunction);
    }

    void clear()
    {
        m_intermediaryResults.clear();

        typename Provider::Ptr provider(m_provider.toStrongRef());

        if (!provider)
            return;

        while (!provider->data().isEmpty())
            provider->removeFirst();
    }

    FetchFunction m_fetch;
    PredicateFunction m_predicate;
    ConvertFunction m_convert;
    CompareFunction m_compare;
    RepresentsFunction m_represents;
    QByteArray m_debugName;

    typename Provider::WeakPtr m_provider;
    QList<InputType> m_intermediaryResults;
};

}

#endif // DOMAIN_LIVEQUERY_H
