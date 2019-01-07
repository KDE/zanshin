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

#include <AkonadiCore/CollectionCreateJob>
#include <AkonadiCore/CollectionDeleteJob>
#include <AkonadiCore/CollectionFetchScope>
#include <AkonadiCore/CollectionModifyJob>
#include <AkonadiCore/ItemCreateJob>
#include <AkonadiCore/ItemDeleteJob>
#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>
#include <AkonadiCore/ItemModifyJob>
#include <AkonadiCore/ItemMoveJob>
#include <AkonadiCore/TransactionSequence>
#include <AkonadiCore/TagCreateJob>
#include <AkonadiCore/TagDeleteJob>
#include <AkonadiCore/TagFetchJob>
#include <AkonadiCore/TagFetchScope>
#include <AkonadiCore/TagModifyJob>
#include <AkonadiCore/TagAttribute>
#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonaditagfetchjobinterface.h"
#include "akonadi/akonadistoragesettings.h"

using namespace Akonadi;

class CollectionJob : public CollectionFetchJob, public CollectionFetchJobInterface
{
    Q_OBJECT
public:
    CollectionJob(const Collection &collection, Type type = FirstLevel, QObject *parent = nullptr)
        : CollectionFetchJob(collection, type, parent),
          m_collection(collection),
          m_type(type)
    {
    }

    Collection::List collections() const override
    {
        auto collections = CollectionFetchJob::collections();

        // Memorize them to reconstruct the ancestor chain later
        QMap<Collection::Id, Collection> collectionsMap;
        collectionsMap[m_collection.id()] = m_collection;
        foreach (const auto &collection, collections) {
            collectionsMap[collection.id()] = collection;
        }

        // Why the hell isn't fetchScope() const and returning a reference???
        auto self = const_cast<CollectionJob*>(this);
        const auto allowedMimeTypes = self->fetchScope().contentMimeTypes().toSet();

        if (!allowedMimeTypes.isEmpty()) {
            collections.erase(std::remove_if(collections.begin(), collections.end(),
                                             [allowedMimeTypes] (const Collection &collection) {
                                                auto mimeTypes = collection.contentMimeTypes().toSet();
                                                return mimeTypes.intersect(allowedMimeTypes).isEmpty();
                                             }),
                              collections.end());
        }

        if (m_type != Base) {
            // Replace the dummy parents in the ancestor chain with proper ones
            // full of juicy data
            std::function<Collection(const Collection&)> reconstructAncestors =
            [collectionsMap, &reconstructAncestors, this] (const Collection &collection) -> Collection {
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
        }

        return collections;
    }

    void setResource(const QString &resource) override
    {
        fetchScope().setResource(resource);
    }

private:
    const Collection m_collection;
    const Type m_type;
};

class ItemJob : public ItemFetchJob, public ItemFetchJobInterface
{
    Q_OBJECT
public:
    using ItemFetchJob::ItemFetchJob;

    Item::List items() const override { return ItemFetchJob::items(); }

    void setCollection(const Collection &collection) override
    {
        ItemFetchJob::setCollection(collection);
    }
};

class TagJob : public TagFetchJob, public TagFetchJobInterface
{
    Q_OBJECT
public:
    using TagFetchJob::TagFetchJob;

    Tag::List tags() const override { return TagFetchJob::tags(); }
};

Storage::Storage()
{
}

Storage::~Storage()
{
}

Collection Storage::defaultCollection()
{
    return StorageSettings::instance().defaultCollection();
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

KJob *Storage::removeItems(Item::List items, QObject *parent)
{
    return new ItemDeleteJob(items, parent);
}

KJob *Storage::moveItem(Item item, Collection collection, QObject *parent)
{
    return new ItemMoveJob(item, collection, parent);
}

KJob *Storage::moveItems(Item::List items, Collection collection, QObject *parent)
{
    return new ItemMoveJob(items, collection, parent);
}

KJob *Storage::createCollection(Collection collection, QObject *parent)
{
    return new CollectionCreateJob(collection, parent);
}

KJob *Storage::updateCollection(Collection collection, QObject *parent)
{
    return new CollectionModifyJob(collection, parent);
}

KJob *Storage::removeCollection(Collection collection, QObject *parent)
{
    return new CollectionDeleteJob(collection, parent);
}

KJob *Storage::createTransaction()
{
    return new TransactionSequence();
}

KJob *Storage::createTag(Tag tag)
{
    return new TagCreateJob(tag);
}

KJob *Storage::updateTag(Tag tag)
{
    return new TagModifyJob(tag);
}

KJob *Storage::removeTag(Tag tag)
{
    return new Akonadi::TagDeleteJob(tag);
}

CollectionFetchJobInterface *Storage::fetchCollections(Collection collection, StorageInterface::FetchDepth depth)
{
    auto job = new CollectionJob(collection, jobTypeFromDepth(depth));

    auto scope = job->fetchScope();
    scope.setContentMimeTypes({KCalCore::Todo::todoMimeType()});
    scope.setIncludeStatistics(true);
    scope.setAncestorRetrieval(CollectionFetchScope::All);
    scope.setListFilter(Akonadi::CollectionFetchScope::Display);
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

ItemFetchJobInterface *Storage::fetchTagItems(Tag tag)
{
    auto job = new ItemJob(tag);

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

#include "akonadistorage.moc"
