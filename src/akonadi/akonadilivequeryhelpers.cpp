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
#include "akonadi/akonadicollectionsearchjobinterface.h"
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

LiveQueryHelpers::CollectionFetchFunction LiveQueryHelpers::searchCollections(const Collection &root, const QString *searchTerm,
                                                                              StorageInterface::FetchContentTypes contentTypes) const
{
    auto storage = m_storage;
    return [storage, contentTypes, searchTerm, root] (const Domain::LiveQueryInput<Collection>::AddFunction &add) {
        if (searchTerm->isEmpty())
            return;

        auto job = storage->searchCollections(*searchTerm, contentTypes);
        Utils::JobHandler::install(job->kjob(), [root, job, add] {
            if (job->kjob()->error())
                return;

            auto directChildren = QHash<Collection::Id, Collection>();
            foreach (const auto &collection, job->collections()) {
                auto directChild = collection;
                while (directChild.parentCollection() != root && directChild.parentCollection().isValid())
                    directChild = directChild.parentCollection();
                if (directChild.parentCollection() != root)
                    continue;
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

LiveQueryHelpers::ItemFetchFunction LiveQueryHelpers::fetchSiblings(const Item &item) const
{
    auto storage = m_storage;
    return [storage, item] (const Domain::LiveQueryInput<Item>::AddFunction &add) {
        auto job = storage->fetchItem(item);
        Utils::JobHandler::install(job->kjob(), [storage, job, add] {
            if (job->kjob()->error() != KJob::NoError)
                return;

            Q_ASSERT(job->items().size() == 1);
            auto item = job->items()[0];
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
