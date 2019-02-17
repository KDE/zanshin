/* This file is part of Zanshin

   Copyright 2015 Mario Bensi <mbensi@ipsquad.net>
   Copyright 2017 Kevin Ottens <ervin@kde.org>

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


#include "akonadicachingstorage.h"
#include "akonadistorage.h"

#include "akonadicollectionfetchjobinterface.h"
#include "akonadiitemfetchjobinterface.h"
#include "akonaditagfetchjobinterface.h"

#include <QTimer>

using namespace Akonadi;

class CachingCollectionFetchJob : public KCompositeJob, public CollectionFetchJobInterface
{
    Q_OBJECT
public:
    CachingCollectionFetchJob(const StorageInterface::Ptr &storage,
                              const Cache::Ptr &cache,
                              const Collection &collection,
                              StorageInterface::FetchDepth depth,
                              QObject *parent = nullptr)
        : KCompositeJob(parent),
          m_started(false),
          m_storage(storage),
          m_cache(cache),
          m_collection(collection),
          m_depth(depth)
    {
        QTimer::singleShot(0, this, &CachingCollectionFetchJob::start);
    }

    void start() override
    {
        if (m_started)
            return;

        if (m_cache->isCollectionListPopulated()) {
            QTimer::singleShot(0, this, &CachingCollectionFetchJob::retrieveFromCache);
        } else {
            auto job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                   Akonadi::StorageInterface::Recursive);
            job->setResource(m_resource);
            addSubjob(job->kjob());
        }

        m_started = true;
    }


    Collection::List collections() const override
    {
        const auto isInputCollection = [this] (const Collection &collection) {
            return collection.id() == m_collection.id()
                || (!m_collection.remoteId().isEmpty() && collection.remoteId() == m_collection.remoteId());
        };

        if (m_depth == StorageInterface::Base) {
            auto it = std::find_if(m_collections.cbegin(), m_collections.cend(), isInputCollection);
            if (it != m_collections.cend())
                return Collection::List() << *it;
            else
                return Collection::List();
        }

        auto collections = m_collections;
        auto it = collections.begin();

        if (m_depth == StorageInterface::FirstLevel) {
            it = std::remove_if(collections.begin(), collections.end(),
                                [isInputCollection] (const Collection &collection) {
                                    return !isInputCollection(collection.parentCollection());
                                });
        } else {
            it = std::remove_if(collections.begin(), collections.end(),
                                [isInputCollection] (const Collection &collection) {
                                    auto parent = collection.parentCollection();
                                    while (parent.isValid() && !isInputCollection(parent))
                                        parent = parent.parentCollection();
                                    return !isInputCollection(parent);
                                });
        }

        collections.erase(it, collections.end());
        return collections;
    }

    void setResource(const QString &resource) override
    {
        m_resource = resource;
    }

private:
    void slotResult(KJob *kjob) override
    {
        if (kjob->error()) {
            KCompositeJob::slotResult(kjob);
            return;
        }

        auto job = dynamic_cast<CollectionFetchJobInterface*>(kjob);
        Q_ASSERT(job);
        auto cachedCollections = job->collections();
        for (const auto &collection : job->collections()) {
            auto parent = collection.parentCollection();
            while (parent.isValid() && parent != Akonadi::Collection::root()) {
                if (!cachedCollections.contains(parent)) {
                    cachedCollections.append(parent);
                }
                parent = parent.parentCollection();
            }
        }
        m_cache->setCollections(cachedCollections);
        m_collections = job->collections();
        emitResult();
    }

    void retrieveFromCache()
    {
        m_collections = m_cache->collections();
        emitResult();
    }

    bool m_started;
    StorageInterface::Ptr m_storage;
    Cache::Ptr m_cache;
    QString m_resource;
    const Collection m_collection;
    const StorageInterface::FetchDepth m_depth;
    Collection::List m_collections;
};

class CachingCollectionItemsFetchJob : public KCompositeJob, public ItemFetchJobInterface
{
    Q_OBJECT
public:
    CachingCollectionItemsFetchJob(const StorageInterface::Ptr &storage,
                                   const Cache::Ptr &cache,
                                   const Collection &collection,
                                   QObject *parent = nullptr)
        : KCompositeJob(parent),
          m_started(false),
          m_storage(storage),
          m_cache(cache),
          m_collection(collection)
    {
        QTimer::singleShot(0, this, &CachingCollectionItemsFetchJob::start);
    }

    void start() override
    {
        if (m_started)
            return;

        if (m_cache->isCollectionPopulated(m_collection.id())) {
            QTimer::singleShot(0, this, &CachingCollectionItemsFetchJob::retrieveFromCache);
        } else {
            auto job = m_storage->fetchItems(m_collection);
            addSubjob(job->kjob());
        }

        m_started = true;
    }


    Item::List items() const override
    {
        return m_items;
    }

    void setCollection(const Collection &collection) override
    {
        m_collection = collection;
    }

private:
    void slotResult(KJob *kjob) override
    {
        if (kjob->error()) {
            KCompositeJob::slotResult(kjob);
            return;
        }

        auto job = dynamic_cast<ItemFetchJobInterface*>(kjob);
        Q_ASSERT(job);
        m_items = job->items();
        m_cache->populateCollection(m_collection, m_items);
        emitResult();
    }

    void retrieveFromCache()
    {
        m_items = m_cache->items(m_collection);
        emitResult();
    }

    bool m_started;
    StorageInterface::Ptr m_storage;
    Cache::Ptr m_cache;
    Collection m_collection;
    Item::List m_items;
};

class CachingSingleItemFetchJob : public KCompositeJob, public ItemFetchJobInterface
{
    Q_OBJECT
public:
    CachingSingleItemFetchJob(const StorageInterface::Ptr &storage,
                              const Cache::Ptr &cache,
                              const Item &item,
                                   QObject *parent = nullptr)
        : KCompositeJob(parent),
          m_started(false),
          m_storage(storage),
          m_cache(cache),
          m_item(item)
    {
        QTimer::singleShot(0, this, &CachingSingleItemFetchJob::start);
    }

    void start() override
    {
        if (m_started)
            return;

        const auto item = m_cache->item(m_item.id());
        if (item.isValid()) {
            QTimer::singleShot(0, this, [this, item] {
                retrieveFromCache(item);
            });
        } else {
            auto job = m_storage->fetchItem(m_item);
            job->setCollection(m_collection);
            addSubjob(job->kjob());
        }

        m_started = true;
    }


    Item::List items() const override
    {
        return m_items;
    }

    void setCollection(const Collection &collection) override
    {
        m_collection = collection;
    }

private:
    void slotResult(KJob *kjob) override
    {
        if (kjob->error()) {
            KCompositeJob::slotResult(kjob);
            return;
        }

        auto job = dynamic_cast<ItemFetchJobInterface*>(kjob);
        Q_ASSERT(job);
        m_items = job->items();
        emitResult();
    }

    void retrieveFromCache(const Item &item)
    {
        m_items = Item::List() << item;
        emitResult();
    }

    bool m_started;
    StorageInterface::Ptr m_storage;
    Cache::Ptr m_cache;
    Item m_item;
    Collection m_collection;
    Item::List m_items;
};

class CachingTagItemsFetchJob : public KCompositeJob, public ItemFetchJobInterface
{
    Q_OBJECT
public:
    CachingTagItemsFetchJob(const StorageInterface::Ptr &storage,
                            const Cache::Ptr &cache,
                            const Tag &tag,
                            QObject *parent = nullptr)
        : KCompositeJob(parent),
          m_started(false),
          m_storage(storage),
          m_cache(cache),
          m_tag(tag)
    {
        QTimer::singleShot(0, this, &CachingTagItemsFetchJob::start);
    }

    void start() override
    {
        if (m_started)
            return;

        if (m_cache->isTagPopulated(m_tag.id())) {
            QTimer::singleShot(0, this, &CachingTagItemsFetchJob::retrieveFromCache);
        } else {
            auto job = m_storage->fetchTagItems(m_tag);
            job->setCollection(m_collection);
            addSubjob(job->kjob());
        }

        m_started = true;
    }


    Item::List items() const override
    {
        return m_items;
    }

    void setCollection(const Collection &collection) override
    {
        m_collection = collection;
    }

private:
    void slotResult(KJob *kjob) override
    {
        if (kjob->error()) {
            KCompositeJob::slotResult(kjob);
            return;
        }

        auto job = dynamic_cast<ItemFetchJobInterface*>(kjob);
        Q_ASSERT(job);
        m_items = job->items();
        m_cache->populateTag(m_tag, m_items);
        emitResult();
    }

    void retrieveFromCache()
    {
        m_items = m_cache->items(m_tag);
        emitResult();
    }

    bool m_started;
    StorageInterface::Ptr m_storage;
    Cache::Ptr m_cache;
    Tag m_tag;
    Collection m_collection;
    Item::List m_items;
};

class CachingTagFetchJob : public KCompositeJob, public TagFetchJobInterface
{
    Q_OBJECT
public:
    CachingTagFetchJob(const StorageInterface::Ptr &storage,
                       const Cache::Ptr &cache,
                       QObject *parent = nullptr)
        : KCompositeJob(parent),
          m_started(false),
          m_storage(storage),
          m_cache(cache)
    {
        QTimer::singleShot(0, this, &CachingTagFetchJob::start);
    }

    void start() override
    {
        if (m_started)
            return;

        if (m_cache->isTagListPopulated()) {
            QTimer::singleShot(0, this, &CachingTagFetchJob::retrieveFromCache);
        } else {
            auto job = m_storage->fetchTags();
            addSubjob(job->kjob());
        }

        m_started = true;
    }

    Tag::List tags() const override
    {
        return m_tags;
    }

private:
    void slotResult(KJob *kjob) override
    {
        if (kjob->error()) {
            KCompositeJob::slotResult(kjob);
            return;
        }

        auto job = dynamic_cast<TagFetchJobInterface*>(kjob);
        Q_ASSERT(job);
        m_tags = job->tags();
        m_cache->setTags(m_tags);
        emitResult();
    }

    void retrieveFromCache()
    {
        m_tags = m_cache->tags();
        emitResult();
    }

    bool m_started;
    StorageInterface::Ptr m_storage;
    Cache::Ptr m_cache;
    Tag::List m_tags;
};

CachingStorage::CachingStorage(const Cache::Ptr &cache, const StorageInterface::Ptr &storage)
    : m_cache(cache),
      m_storage(storage)
{
}

CachingStorage::~CachingStorage()
{
}

Collection CachingStorage::defaultCollection()
{
    return m_storage->defaultCollection();
}

KJob *CachingStorage::createItem(Item item, Collection collection)
{
    return m_storage->createItem(item, collection);
}

KJob *CachingStorage::updateItem(Item item, QObject *parent)
{
    return m_storage->updateItem(item, parent);
}

KJob *CachingStorage::removeItem(Item item)
{
    return m_storage->removeItem(item);
}

KJob *CachingStorage::removeItems(Item::List items, QObject *parent)
{
    return m_storage->removeItems(items, parent);
}

KJob *CachingStorage::moveItem(Item item, Collection collection, QObject *parent)
{
    return m_storage->moveItem(item, collection, parent);
}

KJob *CachingStorage::moveItems(Item::List items, Collection collection, QObject *parent)
{
    return m_storage->moveItems(items, collection, parent);
}

KJob *CachingStorage::createCollection(Collection collection, QObject *parent)
{
    return m_storage->createCollection(collection, parent);
}

KJob *CachingStorage::updateCollection(Collection collection, QObject *parent)
{
    return m_storage->updateCollection(collection, parent);
}

KJob *CachingStorage::removeCollection(Collection collection, QObject *parent)
{
    return m_storage->removeCollection(collection, parent);
}

KJob *CachingStorage::createTransaction()
{
    return m_storage->createTransaction();
}

CollectionFetchJobInterface *CachingStorage::fetchCollections(Collection collection, StorageInterface::FetchDepth depth)
{
    return new CachingCollectionFetchJob(m_storage, m_cache, collection, depth);
}

ItemFetchJobInterface *CachingStorage::fetchItems(Collection collection)
{
    return new CachingCollectionItemsFetchJob(m_storage, m_cache, collection);
}

ItemFetchJobInterface *CachingStorage::fetchItem(Akonadi::Item item)
{
    return new CachingSingleItemFetchJob(m_storage, m_cache, item);
}

ItemFetchJobInterface *CachingStorage::fetchTagItems(Tag tag)
{
    return new CachingTagItemsFetchJob(m_storage, m_cache, tag);
}

TagFetchJobInterface *CachingStorage::fetchTags()
{
    return new CachingTagFetchJob(m_storage, m_cache);
}

#include "akonadicachingstorage.moc"
