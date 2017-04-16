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

using namespace Akonadi;

CachingStorage::CachingStorage(const Cache::Ptr &cache, const StorageInterface::Ptr &storage)
    : m_cache(cache),
      m_storage(storage)
{
}

CachingStorage::~CachingStorage()
{
}

Collection CachingStorage::defaultTaskCollection()
{
    return m_storage->defaultTaskCollection();
}

Collection CachingStorage::defaultNoteCollection()
{
    return m_storage->defaultNoteCollection();
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

KJob *CachingStorage::createTag(Tag tag)
{
    return m_storage->createTag(tag);
}

KJob *CachingStorage::updateTag(Tag tag)
{
    return m_storage->updateTag(tag);
}

KJob *CachingStorage::removeTag(Tag tag)
{
    return m_storage->removeTag(tag);
}

CollectionFetchJobInterface *CachingStorage::fetchCollections(Collection collection, StorageInterface::FetchDepth depth, FetchContentTypes types)
{
    return m_storage->fetchCollections(collection, depth, types);
}

ItemFetchJobInterface *CachingStorage::fetchItems(Collection collection)
{
    return m_storage->fetchItems(collection);
}

ItemFetchJobInterface *CachingStorage::fetchItem(Akonadi::Item item)
{
    return m_storage->fetchItem(item);
}

ItemFetchJobInterface *CachingStorage::fetchTagItems(Tag tag)
{
    return m_storage->fetchTagItems(tag);
}

TagFetchJobInterface *CachingStorage::fetchTags()
{
    return m_storage->fetchTags();
}
