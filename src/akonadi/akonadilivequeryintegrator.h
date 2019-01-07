/* This file is part of Zanshin

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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

#ifndef AKONADI_LIVEQUERYINTEGRATOR_H
#define AKONADI_LIVEQUERYINTEGRATOR_H

#include <QObject>
#include <QSharedPointer>

#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>
#include <AkonadiCore/Tag>

#include <functional>

#include "akonadi/akonadimonitorinterface.h"
#include "akonadi/akonadiserializerinterface.h"

#include "domain/livequery.h"

namespace Akonadi {

class LiveQueryIntegrator : public QObject
{
    Q_OBJECT

    // Helper type trait to extract parameter and return types from
    // a function object

    // Lambda or functors (via method of const method)
    template<typename T>
    struct UnaryFunctionTraits
        : public UnaryFunctionTraits<decltype(&T::operator())> {};

    // Traits definition
    template<typename Return, typename Arg>
    struct UnaryFunctionTraits<Return(Arg)>
    {
        typedef Return ReturnType;
        typedef Arg ArgType;
    };

    // Function pointers
    template<typename Return, typename Arg>
    struct UnaryFunctionTraits<Return(*)(Arg)>
        : public UnaryFunctionTraits<Return(Arg)> {};

    // Method
    template<typename Class, typename Return, typename Arg>
    struct UnaryFunctionTraits<Return(Class::*)(Arg)>
        : public UnaryFunctionTraits<Return(Arg)> {};

    // Const method
    template<typename Class, typename Return, typename Arg>
    struct UnaryFunctionTraits<Return(Class::*)(Arg) const>
        : public UnaryFunctionTraits<Return(Arg)> {};

    // std::function object
    template<typename Function>
    struct UnaryFunctionTraits<std::function<Function>>
        : public UnaryFunctionTraits<Function> {};

    // const reference to std::function object
    template<typename Function>
    struct UnaryFunctionTraits<const std::function<Function> &>
        : public UnaryFunctionTraits<Function> {};

public:
    typedef QSharedPointer<LiveQueryIntegrator> Ptr;
    typedef std::function<void(const Collection &)> CollectionRemoveHandler;
    typedef std::function<void(const Item &)> ItemRemoveHandler;
    typedef std::function<void(const Tag &)> TagRemoveHandler;

    LiveQueryIntegrator(const SerializerInterface::Ptr &serializer,
                        const MonitorInterface::Ptr &monitor,
                        QObject *parent = nullptr);



    template<typename OutputType, typename FetchFunction, typename PredicateFunction, typename... ExtraArgs>
    void bind(const QByteArray &debugName,
              QSharedPointer<Domain::LiveQueryOutput<OutputType>> &output,
              FetchFunction fetch,
              PredicateFunction predicate,
              ExtraArgs... extra)
    {
        typedef UnaryFunctionTraits<FetchFunction> FetchTraits;
        typedef UnaryFunctionTraits<typename FetchTraits::ArgType> AddTraits;
        typedef UnaryFunctionTraits<PredicateFunction> PredicateTraits;

        typedef typename std::decay<typename PredicateTraits::ArgType>::type InputType; // typically Akonadi::Item

        static_assert(std::is_same<typename FetchTraits::ReturnType, void>::value,
                      "Fetch function must return void");
        static_assert(std::is_same<typename AddTraits::ReturnType, void>::value,
                      "Fetch add function must return void");
        static_assert(std::is_same<typename PredicateTraits::ReturnType, bool>::value,
                      "Predicate function must return bool");

        typedef typename std::decay<typename AddTraits::ArgType>::type AddInputType;
        static_assert(std::is_same<AddInputType, InputType>::value,
                      "Fetch add and predicate functions must have the same input type");

        if (output)
            return;

        using namespace std::placeholders;

        auto query = Domain::LiveQuery<InputType, OutputType>::Ptr::create();

        query->setDebugName(debugName);
        query->setFetchFunction(fetch);
        query->setPredicateFunction(predicate);
        query->setConvertFunction(std::bind(&LiveQueryIntegrator::create<InputType, OutputType, ExtraArgs...>, this, _1, extra...));
        query->setUpdateFunction(std::bind(&LiveQueryIntegrator::update<InputType, OutputType, ExtraArgs...>, this, _1, _2, extra...));
        query->setRepresentsFunction(std::bind(&LiveQueryIntegrator::represents<InputType, OutputType>, this, _1, _2));

        inputQueries<InputType>() << query;
        output = query;
    }

    template<typename OutputType, typename FetchFunction, typename CompareFunction, typename PredicateFunction, typename... ExtraArgs>
    void bindRelationship(const QByteArray &debugName,
              QSharedPointer<Domain::LiveQueryOutput<OutputType>> &output,
              FetchFunction fetch,
              CompareFunction compare,
              PredicateFunction predicate,
              ExtraArgs... extra)
    {
        typedef UnaryFunctionTraits<FetchFunction> FetchTraits;
        typedef UnaryFunctionTraits<typename FetchTraits::ArgType> AddTraits;
        typedef UnaryFunctionTraits<PredicateFunction> PredicateTraits;

        typedef typename std::decay<typename PredicateTraits::ArgType>::type InputType; // typically Akonadi::Item

        static_assert(std::is_same<typename FetchTraits::ReturnType, void>::value,
                      "Fetch function must return void");
        static_assert(std::is_same<typename AddTraits::ReturnType, void>::value,
                      "Fetch add function must return void");
        static_assert(std::is_same<typename PredicateTraits::ReturnType, bool>::value,
                      "Predicate function must return bool");

        typedef typename std::decay<typename AddTraits::ArgType>::type AddInputType;
        static_assert(std::is_same<AddInputType, InputType>::value,
                      "Fetch add and predicate functions must have the same input type");

        if (output)
            return;

        using namespace std::placeholders;

        auto query = Domain::LiveRelationshipQuery<InputType, OutputType>::Ptr::create();

        query->setDebugName(debugName);
        query->setFetchFunction(fetch);
        query->setCompareFunction(compare);
        query->setPredicateFunction(predicate);
        query->setConvertFunction(std::bind(&LiveQueryIntegrator::create<InputType, OutputType, ExtraArgs...>, this, _1, extra...));
        query->setRepresentsFunction(std::bind(&LiveQueryIntegrator::represents<InputType, OutputType>, this, _1, _2));

        inputQueries<InputType>() << query;
        output = query;
    }

    void addRemoveHandler(const CollectionRemoveHandler &handler);
    void addRemoveHandler(const ItemRemoveHandler &handler);
    void addRemoveHandler(const TagRemoveHandler &handler);

private slots:
    void onCollectionSelectionChanged();

    void onCollectionAdded(const Akonadi::Collection &collection);
    void onCollectionRemoved(const Akonadi::Collection &collection);
    void onCollectionChanged(const Akonadi::Collection &collection);

    void onItemAdded(const Akonadi::Item &item);
    void onItemRemoved(const Akonadi::Item &item);
    void onItemChanged(const Akonadi::Item &item);

    void onTagAdded(const Akonadi::Tag &tag);
    void onTagRemoved(const Akonadi::Tag &tag);
    void onTagChanged(const Akonadi::Tag &tag);

private:
    void cleanupQueries();

    template<typename InputType, typename OutputType, typename... ExtraArgs>
    OutputType create(const InputType &input, ExtraArgs... extra);
    template<typename InputType, typename OutputType, typename... ExtraArgs>
    void update(const InputType &input, OutputType &output, ExtraArgs... extra);
    template<typename InputType, typename OutputType>
    bool represents(const InputType &input, const OutputType &output);

    template<typename InputType>
    typename Domain::LiveQueryInput<InputType>::WeakList &inputQueries();

    Domain::LiveQueryInput<Collection>::WeakList m_collectionInputQueries;
    Domain::LiveQueryInput<Item>::WeakList m_itemInputQueries;
    Domain::LiveQueryInput<Tag>::WeakList m_tagInputQueries;

    QList<CollectionRemoveHandler> m_collectionRemoveHandlers;
    QList<ItemRemoveHandler> m_itemRemoveHandlers;
    QList<TagRemoveHandler> m_tagRemoveHandlers;

    SerializerInterface::Ptr m_serializer;
    MonitorInterface::Ptr m_monitor;
};

template<>
inline Domain::Context::Ptr LiveQueryIntegrator::create<Tag, Domain::Context::Ptr>(const Tag &input)
{
    return m_serializer->createContextFromTag(input);
}

template<>
inline void LiveQueryIntegrator::update<Tag, Domain::Context::Ptr>(const Tag &input, Domain::Context::Ptr &output)
{
    m_serializer->updateContextFromTag(output, input);
}

template<>
inline bool LiveQueryIntegrator::represents<Tag, Domain::Context::Ptr>(const Tag &input, const Domain::Context::Ptr &output)
{
    return m_serializer->isContextTag(output, input);
}

template<>
inline Domain::DataSource::Ptr LiveQueryIntegrator::create<Collection, Domain::DataSource::Ptr>(const Collection &input)
{
    return m_serializer->createDataSourceFromCollection(input, SerializerInterface::BaseName);
}

template<>
inline void LiveQueryIntegrator::update<Collection, Domain::DataSource::Ptr>(const Collection &input, Domain::DataSource::Ptr &output)
{
    m_serializer->updateDataSourceFromCollection(output, input, SerializerInterface::BaseName);
}

template<>
inline Domain::DataSource::Ptr LiveQueryIntegrator::create<Collection, Domain::DataSource::Ptr, SerializerInterface::DataSourceNameScheme>(const Collection &input,
                                                                                                                                           SerializerInterface::DataSourceNameScheme nameScheme)
{
    return m_serializer->createDataSourceFromCollection(input, nameScheme);
}

template<>
inline void LiveQueryIntegrator::update<Collection, Domain::DataSource::Ptr, SerializerInterface::DataSourceNameScheme>(const Collection &input, Domain::DataSource::Ptr &output,
                                                                                                                        SerializerInterface::DataSourceNameScheme nameScheme)
{
    m_serializer->updateDataSourceFromCollection(output, input, nameScheme);
}

template<>
inline bool LiveQueryIntegrator::represents<Collection, Domain::DataSource::Ptr>(const Collection &input, const Domain::DataSource::Ptr &output)
{
    return m_serializer->representsCollection(output, input);
}

template<>
inline Domain::DataSource::Ptr LiveQueryIntegrator::create<Item, Domain::DataSource::Ptr>(const Item &input)
{
    return m_serializer->createDataSourceFromCollection(input.parentCollection(), SerializerInterface::BaseName);
}

template<>
inline void LiveQueryIntegrator::update<Item, Domain::DataSource::Ptr>(const Item &input, Domain::DataSource::Ptr &output)
{
    m_serializer->updateDataSourceFromCollection(output, input.parentCollection(), SerializerInterface::BaseName);
}

template<>
inline bool LiveQueryIntegrator::represents<Item, Domain::DataSource::Ptr>(const Item &input, const Domain::DataSource::Ptr &output)
{
    return m_serializer->representsCollection(output, input.parentCollection());
}

template<>
inline Domain::Project::Ptr LiveQueryIntegrator::create<Item, Domain::Project::Ptr>(const Item &input)
{
    return m_serializer->createProjectFromItem(input);
}

template<>
inline void LiveQueryIntegrator::update<Item, Domain::Project::Ptr>(const Item &input, Domain::Project::Ptr &output)
{
    m_serializer->updateProjectFromItem(output, input);
}

template<>
inline bool LiveQueryIntegrator::represents<Item, Domain::Project::Ptr>(const Item &input, const Domain::Project::Ptr &output)
{
    return m_serializer->representsItem(output, input);
}

template<>
inline Domain::Task::Ptr LiveQueryIntegrator::create<Item, Domain::Task::Ptr>(const Item &input)
{
    return m_serializer->createTaskFromItem(input);
}

template<>
inline void LiveQueryIntegrator::update<Item, Domain::Task::Ptr>(const Item &input, Domain::Task::Ptr &output)
{
    m_serializer->updateTaskFromItem(output, input);
}

template<>
inline bool LiveQueryIntegrator::represents<Item, Domain::Task::Ptr>(const Item &input, const Domain::Task::Ptr &output)
{
    return m_serializer->representsItem(output, input);
}

template<>
inline typename Domain::LiveQueryInput<Collection>::WeakList &LiveQueryIntegrator::inputQueries<Collection>()
{
    return m_collectionInputQueries;
}
template<>
inline typename Domain::LiveQueryInput<Item>::WeakList &LiveQueryIntegrator::inputQueries<Item>()
{
    return m_itemInputQueries;
}
template<>
inline typename Domain::LiveQueryInput<Tag>::WeakList &LiveQueryIntegrator::inputQueries<Tag>()
{
    return m_tagInputQueries;
}

}

#endif // AKONADI_LIVEQUERYINTEGRATOR_H
