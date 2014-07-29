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


#include "akonadistorage.h"

#include <algorithm>

#include <KCalCore/Todo>

#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemMoveJob>
#include <Akonadi/Notes/NoteUtils>
#include <Akonadi/TransactionSequence>
#include <Akonadi/TagFetchJob>
#include <Akonadi/TagFetchScope>
#include <Akonadi/TagAttribute>
#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonaditagfetchjobinterface.h"
#include "akonadi/akonadistoragesettings.h"

using namespace Akonadi;

class CollectionJob : public CollectionFetchJob, public CollectionFetchJobInterface
{
public:
    CollectionJob (const Collection &collection, Type type=FirstLevel, QObject *parent=0)
        : CollectionFetchJob(collection, type, parent),
          m_collection(collection)
    {
    }

    Collection::List collections() const
    {
        auto collections = CollectionFetchJob::collections();

        // Memorize them to reconstruct the ancestor chain later
        QMap<Collection::Id, Collection> collectionsMap;
        collectionsMap[m_collection.id()] = m_collection;
        for (auto collection : collections) {
            collectionsMap[collection.id()] = collection;
        }

        // Why the hell isn't fetchScope() const and returning a reference???
        auto self = const_cast<CollectionJob*>(this);
        const auto allowedMimeTypes = self->fetchScope().contentMimeTypes().toSet();

        collections.erase(std::remove_if(collections.begin(), collections.end(),
                                         [allowedMimeTypes] (const Collection &collection) {
                                            auto mimeTypes = collection.contentMimeTypes().toSet();
                                            return mimeTypes.intersect(allowedMimeTypes).isEmpty();
                                         }),
                          collections.end());

        // Replace the dummy parents in the ancestor chain with proper ones
        // full of juicy data
        std::function<Collection(const Collection&)> reconstructAncestors =
        [collectionsMap, &reconstructAncestors, this] (const Collection &collection) {
            Q_ASSERT(collection.isValid());

            if (collection == m_collection)
                return collection;

            auto parent = collection.parentCollection();
            auto reconstructedParent = reconstructAncestors(collectionsMap[parent.id()]);

            auto result = collection;
            result.setParentCollection(reconstructedParent);
            return result;
        };

        std::transform(collections.begin(), collections.end(),
                       collections.begin(), reconstructAncestors);

        return collections;
    }

private:
    const Collection m_collection;
};

class ItemJob : public ItemFetchJob, public ItemFetchJobInterface
{
public:
    using ItemFetchJob::ItemFetchJob;

    Item::List items() const { return ItemFetchJob::items(); }
};

class TagJob : public TagFetchJob, public TagFetchJobInterface
{
public:
    using TagFetchJob::TagFetchJob;

    Tag::List tags() const { return TagFetchJob::tags(); }
};

Storage::Storage()
{
}

Storage::~Storage()
{
}

Collection Storage::defaultTaskCollection()
{
    return StorageSettings::instance().defaultTaskCollection();
}

Collection Storage::defaultNoteCollection()
{
    return StorageSettings::instance().defaultNoteCollection();
}

KJob *Storage::createItem(Item item, Collection collection)
{
    return new ItemCreateJob(item, collection);
}

KJob *Storage::updateItem(Item item, QObject *parent)
{
    return new ItemModifyJob(item, parent);
}

KJob *Storage::removeItem(Item item)
{
    return new ItemDeleteJob(item);
}

KJob *Storage::moveItem(Item item, Collection collection, QObject *parent)
{
    return new ItemMoveJob(item, collection, parent);
}

KJob *Storage::moveItems(Item::List items, Collection collection, QObject *parent)
{
    return new ItemMoveJob(items, collection, parent);
}

KJob *Storage::createTransaction()
{
    return new TransactionSequence();
}

CollectionFetchJobInterface *Storage::fetchCollections(Collection collection, StorageInterface::FetchDepth depth, FetchContentTypes types)
{
    QStringList contentMimeTypes;
    if (types & Notes)
        contentMimeTypes << NoteUtils::noteMimeType();
    if (types & Tasks)
        contentMimeTypes << KCalCore::Todo::todoMimeType();


    Q_ASSERT(!contentMimeTypes.isEmpty());

    auto job = new CollectionJob(collection, jobTypeFromDepth(depth));

    auto scope = job->fetchScope();
    scope.setContentMimeTypes(contentMimeTypes);
    scope.setIncludeStatistics(true);
    scope.setAncestorRetrieval(CollectionFetchScope::All);
    job->setFetchScope(scope);

    return job;
}

ItemFetchJobInterface *Storage::fetchItems(Collection collection)
{
    auto job = new ItemJob(collection);

    configureItemFetchJob(job);

    return job;
}

ItemFetchJobInterface *Storage::fetchItem(Akonadi::Item item)
{
    auto job = new ItemJob(item);

    configureItemFetchJob(job);

    return job;
}

TagFetchJobInterface *Storage::fetchTags()
{
    return new TagJob;
}

CollectionFetchJob::Type Storage::jobTypeFromDepth(StorageInterface::FetchDepth depth)
{
    auto jobType = CollectionJob::Base;

    switch (depth) {
    case Base:
        jobType = CollectionJob::Base;
        break;
    case FirstLevel:
        jobType = CollectionJob::FirstLevel;
        break;
    case Recursive:
        jobType = CollectionJob::Recursive;
        break;
    default:
        qFatal("Unexpected enum value");
        break;
    }

    return jobType;
}

void Storage::configureItemFetchJob(ItemJob *job)
{
    auto scope = job->fetchScope();
    scope.fetchFullPayload();
    scope.fetchAllAttributes();
    scope.setFetchTags(true);
    scope.tagFetchScope().setFetchIdOnly(false);
    scope.setAncestorRetrieval(ItemFetchScope::All);
    job->setFetchScope(scope);
}
