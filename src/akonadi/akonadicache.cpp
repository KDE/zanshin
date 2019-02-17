/* This file is part of Zanshin

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


#include "akonadicache.h"

using namespace Akonadi;

Cache::Cache(const SerializerInterface::Ptr &serializer, const MonitorInterface::Ptr &monitor, QObject *parent)
    : QObject(parent),
      m_serializer(serializer),
      m_monitor(monitor),
      m_collectionListPopulated(false)
{
    connect(m_monitor.data(), &MonitorInterface::collectionAdded,
            this, &Cache::onCollectionAdded);
    connect(m_monitor.data(), &MonitorInterface::collectionChanged,
            this, &Cache::onCollectionChanged);
    connect(m_monitor.data(), &MonitorInterface::collectionRemoved,
            this, &Cache::onCollectionRemoved);

    connect(m_monitor.data(), &MonitorInterface::itemAdded,
            this, &Cache::onItemAdded);
    connect(m_monitor.data(), &MonitorInterface::itemChanged,
            this, &Cache::onItemChanged);
    connect(m_monitor.data(), &MonitorInterface::itemRemoved,
            this, &Cache::onItemRemoved);
}

bool Cache::isCollectionListPopulated() const
{
    return m_collectionListPopulated;
}

Collection::List Cache::collections() const
{
    using namespace std::placeholders;

    auto res = Collection::List();
    std::copy_if(m_collections.cbegin(), m_collections.cend(),
                 std::back_inserter(res),
                 [this] (const Collection &collection) {
                     return m_serializer->isTaskCollection(collection);
                 });
    return res;
}

Collection::List Cache::allCollections() const
{
    return m_collections;
}

bool Cache::isCollectionKnown(Collection::Id id) const
{
    return m_collections.contains(Collection(id));
}

Collection Cache::collection(Collection::Id id) const
{
    const auto index = m_collections.indexOf(Collection(id));
    if (index >= 0)
        return m_collections.at(index);
    else
        return Collection();
}

bool Cache::isCollectionPopulated(Collection::Id id) const
{
    return m_collectionItems.contains(id);
}

Item::List Cache::items(const Collection &collection) const
{
    const auto ids = m_collectionItems.value(collection.id());
    auto items = Item::List();
    items.reserve(ids.size());
    std::transform(ids.cbegin(), ids.cend(),
                   std::back_inserter(items),
                   [this](const Item::Id &id) { return m_items.value(id); });
    return items;
}

void Cache::setCollections(const Collection::List &collections)
{
    m_collections = collections;
    m_collectionListPopulated = true;
}

void Cache::populateCollection(const Collection &collection, const Item::List &items)
{
    auto &ids = m_collectionItems[collection.id()];
    for (const auto &item : items) {
        m_items.insert(item.id(), item);
        if (!ids.contains(item.id()))
            ids << item.id();
    }
}

Item Cache::item(Item::Id id) const
{
    return m_items.value(id);
}

void Cache::onCollectionAdded(const Collection &collection)
{
    const auto index = m_collections.indexOf(collection);
    if (index >= 0) {
        m_collections[index] = collection;
        return;
    }

    if (m_collectionListPopulated && m_serializer->isTaskCollection(collection)) {
        m_collections << collection;
    }
}

void Cache::onCollectionChanged(const Collection &collection)
{
    const auto index = m_collections.indexOf(collection);
    if (index >= 0)
        m_collections[index] = collection;
}

void Cache::onCollectionRemoved(const Collection &collection)
{
    m_collections.removeAll(collection);

    for (const auto itemId : m_collectionItems.value(collection.id())) {
        m_items.remove(itemId);
    }

    m_collectionItems.remove(collection.id());
}


void Cache::onItemAdded(const Item &item)
{
    bool needsInsert = false;
    const auto it = m_collectionItems.find(item.parentCollection().id());
    if (it != m_collectionItems.end()) {
        *it << item.id();
        needsInsert = true;
    }
    if (needsInsert)
        m_items.insert(item.id(), item);

}

void Cache::onItemChanged(const Item &item)
{
    const auto oldItem = m_items.take(item.id());

    if (oldItem.parentCollection() != item.parentCollection()) {
        auto it = m_collectionItems.find(oldItem.parentCollection().id());
        if (it != m_collectionItems.end())
            it->removeAll(oldItem.id());

        it = m_collectionItems.find(item.parentCollection().id());
        if (it != m_collectionItems.end())
            it->append(item.id());
    }

    if (m_collectionItems.contains(item.parentCollection().id())) {
        m_items.insert(item.id(), item);
    }
}

void Cache::onItemRemoved(const Item &item)
{
    m_items.remove(item.id());
    for (auto &itemList : m_collectionItems)
        itemList.removeAll(item.id());
}
