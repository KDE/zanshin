/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "akonadistorage.h"

#include <algorithm>

#include <KCalCore/Todo>

#include <Akonadi/CollectionCreateJob>
#include <Akonadi/CollectionDeleteJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemMoveJob>
#include <Akonadi/TagFetchScope>
#include <Akonadi/TransactionSequence>
#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonadistoragesettings.h"

using namespace Akonadi;

namespace
{
    template<typename T>
    QSet<T> listToSet(const QList<T> &list)
    {
        return {list.cbegin(), list.cend()};
    }
}

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
        const auto allowedMimeTypes = listToSet(self->fetchScope().contentMimeTypes());

        if (!allowedMimeTypes.isEmpty()) {
            collections.erase(std::remove_if(collections.begin(), collections.end(),
                                             [allowedMimeTypes] (const Collection &collection) {
                                                auto mimeTypes = listToSet(collection.contentMimeTypes());
                                                return !mimeTypes.intersects(allowedMimeTypes);
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

KJob *Storage::removeItem(Item item, QObject *parent)
{
    return new ItemDeleteJob(item, parent);
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

KJob *Storage::createTransaction(QObject *parent)
{
    return new TransactionSequence(parent);
}

CollectionFetchJobInterface *Storage::fetchCollections(Collection collection, StorageInterface::FetchDepth depth, QObject *parent)
{
    auto job = new CollectionJob(collection, jobTypeFromDepth(depth), parent);

    auto scope = job->fetchScope();
    scope.setContentMimeTypes({KCalCore::Todo::todoMimeType()});
    scope.setIncludeStatistics(true);
    scope.setAncestorRetrieval(CollectionFetchScope::All);
    scope.setListFilter(Akonadi::CollectionFetchScope::Display);
    job->setFetchScope(scope);

    return job;
}

ItemFetchJobInterface *Storage::fetchItems(Collection collection, QObject *parent)
{
    auto job = new ItemJob(collection, parent);

    configureItemFetchJob(job);

    return job;
}

ItemFetchJobInterface *Storage::fetchItem(Akonadi::Item item, QObject *parent)
{
    auto job = new ItemJob(item, parent);

    configureItemFetchJob(job);

    return job;
}

ItemFetchJobInterface *Storage::fetchItemsWithTags(Collection collection)
{
    auto job = new ItemJob(collection);
    configureItemFetchJob(job);
    job->fetchScope().setFetchTags(true);
    job->fetchScope().tagFetchScope().setFetchIdOnly(false);
    return job;
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
    scope.setFetchTags(false);
    scope.setAncestorRetrieval(ItemFetchScope::All);
    job->setFetchScope(scope);
}

#include "akonadistorage.moc"
