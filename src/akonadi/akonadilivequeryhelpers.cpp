/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "akonadilivequeryhelpers.h"

#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"

#include "utils/jobhandler.h"

using namespace Akonadi;

LiveQueryHelpers::LiveQueryHelpers(const SerializerInterface::Ptr &serializer,
                                   const StorageInterface::Ptr &storage)
    : m_serializer(serializer),
      m_storage(storage)
{
}

LiveQueryHelpers::CollectionFetchFunction LiveQueryHelpers::fetchAllCollections(QObject *parent) const
{
    auto storage = m_storage;
    return [storage, parent] (const Domain::LiveQueryInput<Collection>::AddFunction &add) {
        auto job = storage->fetchCollections(Collection::root(), StorageInterface::Recursive, parent);
        Utils::JobHandler::install(job->kjob(), [job, add] {
            if (job->kjob()->error())
                return;

            foreach (const auto &collection, job->collections())
                add(collection);
        });
    };
}

LiveQueryHelpers::CollectionFetchFunction LiveQueryHelpers::fetchCollections(const Collection &root, QObject *parent) const
{
    auto storage = m_storage;
    return [storage, root, parent] (const Domain::LiveQueryInput<Collection>::AddFunction &add) {
        auto job = storage->fetchCollections(root, StorageInterface::Recursive, parent);
        Utils::JobHandler::install(job->kjob(), [root, job, add] {
            if (job->kjob()->error())
                return;

            auto directChildren = QHash<Collection::Id, Collection>();
            foreach (const auto &collection, job->collections()) {
                auto directChild = collection;
                while (directChild.parentCollection() != root)
                    directChild = directChild.parentCollection();
                if (!directChildren.contains(directChild.id()))
                    directChildren[directChild.id()] = directChild;
            }

            foreach (const auto &directChild, directChildren)
                add(directChild);
        });
    };
}

LiveQueryHelpers::ItemFetchFunction LiveQueryHelpers::fetchItems(QObject *parent) const
{
    auto serializer = m_serializer;
    auto storage = m_storage;
    return [serializer, storage, parent] (const Domain::LiveQueryInput<Item>::AddFunction &add) {
        auto job = storage->fetchCollections(Akonadi::Collection::root(),
                                             StorageInterface::Recursive,
                                             parent);
        Utils::JobHandler::install(job->kjob(), [serializer, storage, job, add, parent] {
            if (job->kjob()->error() != KJob::NoError)
                return;

            foreach (const auto &collection, job->collections()) {
                if (!serializer->isSelectedCollection(collection))
                    continue;

                auto job = storage->fetchItems(collection, parent);
                Utils::JobHandler::install(job->kjob(), [job, add] {
                    if (job->kjob()->error() != KJob::NoError)
                        return;

                    foreach (const auto &item, job->items())
                        add(item);
                });
            }
        });
    };
}

LiveQueryHelpers::ItemFetchFunction LiveQueryHelpers::fetchItems(const Collection &collection, QObject *parent) const
{
    auto storage = m_storage;
    return [storage, collection, parent] (const Domain::LiveQueryInput<Item>::AddFunction &add) {
        auto job = storage->fetchItems(collection, parent);
        Utils::JobHandler::install(job->kjob(), [job, add] {
            if (job->kjob()->error() != KJob::NoError)
                return;

            foreach (const auto &item, job->items())
                add(item);
        });
    };
}

LiveQueryHelpers::ItemFetchFunction LiveQueryHelpers::fetchItemsForContext(const Domain::Context::Ptr &context, QObject *parent) const
{
    auto fetchFunction = fetchItems(parent);
    auto serializer = m_serializer;

    return [context, fetchFunction, serializer] (const Domain::LiveQueryInput<Item>::AddFunction &add) {
        auto filterAdd = [context, add, serializer] (const Item &item) {
            if (serializer->isContextChild(context, item))
                add(item);
        };
        fetchFunction(filterAdd);
    };
}

LiveQueryHelpers::ItemFetchFunction LiveQueryHelpers::fetchTaskAndAncestors(Domain::Task::Ptr task, QObject *parent) const
{
    Akonadi::Item childItem = m_serializer->createItemFromTask(task);
    Q_ASSERT(childItem.parentCollection().isValid()); // do I really need a fetchItem first, like fetchSiblings does?
    // Note: if the task moves to another collection, this live query will then be invalid...

    const Akonadi::Item::Id childId = childItem.id();
    auto storage = m_storage;
    auto serializer = m_serializer;
    return [storage, serializer, childItem, childId, parent] (const Domain::LiveQueryInput<Item>::AddFunction &add) {
        auto job = storage->fetchItems(childItem.parentCollection(), parent);
        Utils::JobHandler::install(job->kjob(), [job, add, serializer, childId] {
            if (job->kjob()->error() != KJob::NoError)
                return;

            const auto items = job->items();
            // The item itself is part of the result, we need that in findProject, to react on changes of the item itself
            // To return a correct child item in case it got updated, we can't use childItem, we need to find it in the list.
            const auto myself = std::find_if(items.cbegin(), items.cend(),
                                             [childId] (const Akonadi::Item &item) {
                                                 return childId == item.id();
                                             });
            if (myself == items.cend()) {
                qWarning() << "Did not find item in the listing for its parent collection. Item ID:" << childId;
                return;
            }
            add(*myself);
            auto parentUid = serializer->relatedUidFromItem(*myself);
            while (!parentUid.isEmpty()) {
                const auto parent = std::find_if(items.cbegin(), items.cend(),
                                                 [serializer, parentUid] (const Akonadi::Item &item) {
                                                     return serializer->itemUid(item) == parentUid;
                                                 });
                if (parent == items.cend()) {
                    break;
                }
                add(*parent);
                parentUid = serializer->relatedUidFromItem(*parent);
            }
        });
    };
}

LiveQueryHelpers::CollectionFetchFunction LiveQueryHelpers::fetchItemCollection(const Item& item, QObject *parent) const
{
    auto storage = m_storage;
    return [storage, item, parent] (const Domain::LiveQueryInput<Collection>::AddFunction &add) {
        auto job = storage->fetchCollections(item.parentCollection(), StorageInterface::Base, parent);
        Utils::JobHandler::install(job->kjob(), [storage, job, add] {
            if (job->kjob()->error() != KJob::NoError)
                return;
            auto collection = job->collections().at(0);
            add(collection);
        });
    };
}


LiveQueryHelpers::ItemFetchFunction LiveQueryHelpers::fetchSiblings(const Item &item, QObject *parent) const
{
    auto storage = m_storage;
    return [storage, item, parent] (const Domain::LiveQueryInput<Item>::AddFunction &add) {
        auto job = storage->fetchItem(item, parent);
        Utils::JobHandler::install(job->kjob(), [storage, job, add, parent] {
            if (job->kjob()->error() != KJob::NoError)
                return;

            Q_ASSERT(job->items().size() == 1);
            auto item = job->items().at(0);
            Q_ASSERT(item.parentCollection().isValid());
            auto job = storage->fetchItems(item.parentCollection(), parent);
            Utils::JobHandler::install(job->kjob(), [job, add] {
                if (job->kjob()->error() != KJob::NoError)
                    return;

                foreach (const auto &item, job->items())
                    add(item);
            });
        });
    };
}
