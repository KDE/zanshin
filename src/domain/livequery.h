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


template<typename InputType, typename OutputType>
class LiveQuery
{
public:
    typedef QSharedPointer<LiveQuery<InputType, OutputType>> Ptr;
    typedef QList<Ptr> List;

    typedef QueryResultProvider<OutputType> Provider;
    typedef QueryResult<OutputType> Result;

    typedef std::function<void(const InputType &)> AddFunction;

    typedef std::function<void(const AddFunction &)> FetchFunction;
    typedef std::function<bool(const InputType &)> PredicateFunction;
    typedef std::function<OutputType(const InputType &)> ConvertFunction;
    typedef std::function<void(const InputType &, OutputType &)> UpdateFunction;
    typedef std::function<bool(const InputType &, const OutputType &)> RepresentsFunction;

    ~LiveQuery()
    {
        clear();
    }

    typename Result::Ptr result()
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

    void setRepresentsFunction(const RepresentsFunction &represents)
    {
        m_represents = represents;
    }

    void reset()
    {
        clear();
        doFetch();
    }

    void onAdded(const InputType &input)
    {
        typename Provider::Ptr provider(m_provider.toStrongRef());

        if (!provider)
            return;

        if (m_predicate(input))
            addToProvider(provider, input);
    }

    void onChanged(const InputType &input)
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

    void onRemoved(const InputType &input)
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
    bool isValidOutput(const T &/*ouput*/)
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
        return output != Q_NULLPTR;
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

    typename Provider::WeakPtr m_provider;
};


}

#endif // DOMAIN_LIVEQUERY_H
