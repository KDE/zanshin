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


#include "akonadilivequeryhelpers.h"

#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonaditagfetchjobinterface.h"

#include "utils/jobhandler.h"

using namespace Akonadi;

LiveQueryHelpers::LiveQueryHelpers(const SerializerInterface::Ptr &serializer,
                                   const StorageInterface::Ptr &storage)
    : m_serializer(serializer),
      m_storage(storage)
{
}

LiveQueryHelpers::CollectionFetchFunction LiveQueryHelpers::fetchAllCollections(StorageInterface::FetchContentTypes contentTypes) const
{
    auto storage = m_storage;
    return [storage, contentTypes] (const Domain::LiveQueryInput<Collection>::AddFunction &add) {
        auto job = storage->fetchCollections(Collection::root(), StorageInterface::Recursive, contentTypes);
        Utils::JobHandler::install(job->kjob(), [job, add] {
            if (job->kjob()->error())
                return;

            foreach (const auto &collection, job->collections())
                add(collection);
        });
    };
}

LiveQueryHelpers::CollectionFetchFunction LiveQueryHelpers::fetchCollections(const Collection &root, StorageInterface::FetchContentTypes contentTypes) const
{
    auto storage = m_storage;
    return [storage, contentTypes, root] (const Domain::LiveQueryInput<Collection>::AddFunction &add) {
        auto job = storage->fetchCollections(root, StorageInterface::Recursive, contentTypes);
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

LiveQueryHelpers::ItemFetchFunction LiveQueryHelpers::fetchItems(StorageInterface::FetchContentTypes contentTypes) const
{
    auto serializer = m_serializer;
    auto storage = m_storage;
    return [serializer, storage, contentTypes] (const Domain::LiveQueryInput<Item>::AddFunction &add) {
        auto job = storage->fetchCollections(Akonadi::Collection::root(),
                                             StorageInterface::Recursive,
                                             contentTypes);
        Utils::JobHandler::install(job->kjob(), [serializer, storage, job, add] {
            if (job->kjob()->error() != KJob::NoError)
                return;

            foreach (const auto &collection, job->collections()) {
                if (!serializer->isSelectedCollection(collection))
                    continue;

                auto job = storage->fetchItems(collection);
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

LiveQueryHelpers::ItemFetchFunction LiveQueryHelpers::fetchItems(const Collection &collection) const
{
    auto storage = m_storage;
    return [storage, collection] (const Domain::LiveQueryInput<Item>::AddFunction &add) {
        auto job = storage->fetchItems(collection);
        Utils::JobHandler::install(job->kjob(), [job, add] {
            if (job->kjob()->error() != KJob::NoError)
                return;

            foreach (const auto &item, job->items())
                add(item);
        });
    };
}

LiveQueryHelpers::ItemFetchFunction LiveQueryHelpers::fetchItems(const Tag &tag) const
{
    // TODO: Qt5, use the proper implementation once we got a working akonadi
#if 0
    auto storage = m_storage;
    return [storage, tag] (const Domain::LiveQueryInput<Item>::AddFunction &add) {
        auto job = storage->fetchTagItems(tag);
        Utils::JobHandler::install(job->kjob(), [job, add] {
            if (job->kjob()->error() != KJob::NoError)
                return;

            foreach (const auto &item, job->items())
                add(item);
        });
    };
#else
    auto fetchFunction = fetchItems(StorageInterface::Tasks | StorageInterface::Notes);

    return [tag, fetchFunction] (const Domain::LiveQueryInput<Item>::AddFunction &add) {
        auto filterAdd = [tag, add] (const Item &item) {
            if (item.tags().contains(tag))
                add(item);
        };
        fetchFunction(filterAdd);
    };
#endif
}

LiveQueryHelpers::ItemFetchFunction LiveQueryHelpers::fetchTaskAndAncestors(Domain::Task::Ptr task) const
{
    Akonadi::Item childItem = m_serializer->createItemFromTask(task);
    Q_ASSERT(childItem.parentCollection().isValid()); // do I really need a fetchItem first, like fetchSiblings does?
    // Note: if the task moves to another collection, this live query will then be invalid...

    const Akonadi::Item::Id childId = childItem.id();
    auto storage = m_storage;
    auto serializer = m_serializer;
    return [storage, serializer, childItem, childId] (const Domain::LiveQueryInput<Item>::AddFunction &add) {
        auto job = storage->fetchItems(childItem.parentCollection());
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

LiveQueryHelpers::ItemFetchFunction LiveQueryHelpers::fetchSiblings(const Item &item) const
{
    auto storage = m_storage;
    return [storage, item] (const Domain::LiveQueryInput<Item>::AddFunction &add) {
        auto job = storage->fetchItem(item);
        Utils::JobHandler::install(job->kjob(), [storage, job, add] {
            if (job->kjob()->error() != KJob::NoError)
                return;

            Q_ASSERT(job->items().size() == 1);
            auto item = job->items().at(0);
            Q_ASSERT(item.parentCollection().isValid());
            auto job = storage->fetchItems(item.parentCollection());
            Utils::JobHandler::install(job->kjob(), [job, add] {
                if (job->kjob()->error() != KJob::NoError)
                    return;

                foreach (const auto &item, job->items())
                    add(item);
            });
        });
    };
}

LiveQueryHelpers::TagFetchFunction LiveQueryHelpers::fetchTags() const
{
    auto storage = m_storage;
    return [storage] (const Domain::LiveQueryInput<Tag>::AddFunction &add) {
        auto job = storage->fetchTags();
        Utils::JobHandler::install(job->kjob(), [job, add] {
            foreach (const auto &tag, job->tags())
                add(tag);
        });
    };
}
