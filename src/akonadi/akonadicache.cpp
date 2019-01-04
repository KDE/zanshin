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
      m_tagListPopulated(false)
{
    connect(m_monitor.data(), &MonitorInterface::collectionAdded,
            this, &Cache::onCollectionAdded);
    connect(m_monitor.data(), &MonitorInterface::collectionChanged,
            this, &Cache::onCollectionChanged);
    connect(m_monitor.data(), &MonitorInterface::collectionRemoved,
            this, &Cache::onCollectionRemoved);

    connect(m_monitor.data(), &MonitorInterface::tagAdded,
            this, &Cache::onTagAdded);
    connect(m_monitor.data(), &MonitorInterface::tagChanged,
            this, &Cache::onTagChanged);
    connect(m_monitor.data(), &MonitorInterface::tagRemoved,
            this, &Cache::onTagRemoved);

    connect(m_monitor.data(), &MonitorInterface::itemAdded,
            this, &Cache::onItemAdded);
    connect(m_monitor.data(), &MonitorInterface::itemChanged,
            this, &Cache::onItemChanged);
    connect(m_monitor.data(), &MonitorInterface::itemRemoved,
            this, &Cache::onItemRemoved);
}

bool Cache::isContentTypesPopulated(StorageInterface::FetchContentTypes contentTypes) const
{
    return m_populatedContentTypes.contains(contentTypes);
}

Collection::List Cache::collections(StorageInterface::FetchContentTypes contentTypes) const
{
    using namespace std::placeholders;

    if (contentTypes == StorageInterface::AllContent)
        return m_collections;

    auto res = Collection::List();
    std::copy_if(m_collections.cbegin(), m_collections.cend(),
                 std::back_inserter(res),
                 [this, contentTypes] (const Collection &collection) {
                     return matchCollection(contentTypes, collection);
                 });
    return res;
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

void Cache::setCollections(StorageInterface::FetchContentTypes contentTypes, const Collection::List &collections)
{
    m_populatedContentTypes.insert(contentTypes);
    for (const auto &collection : collections) {
        const auto index = m_collections.indexOf(collection);
        if (index >= 0)
            m_collections[index] = collection;
        else
            m_collections.append(collection);
    }
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

bool Cache::isTagListPopulated() const
{
    return m_tagListPopulated;
}

Tag::List Cache::tags() const
{
    return m_tags;
}

bool Cache::isTagKnown(Tag::Id id) const
{
    return m_tags.contains(Tag(id));
}

Tag Cache::tag(Tag::Id id) const
{
    const auto index = m_tags.indexOf(Tag(id));
    if (index >= 0)
        return m_tags.at(index);
    else
        return Tag();
}

bool Cache::isTagPopulated(Tag::Id id) const
{
    return m_tagItems.contains(id);
}

Item::List Cache::items(const Tag &tag) const
{
    const auto ids = m_tagItems.value(tag.id());
    auto items = Item::List();
    items.reserve(ids.size());
    std::transform(ids.cbegin(), ids.cend(),
                   std::back_inserter(items),
                   [this](const Item::Id &id) { return m_items.value(id); });
    return items;
}

void Cache::setTags(const Tag::List &tags)
{
    m_tags = tags;
    m_tagListPopulated = true;
}

void Cache::populateTag(const Tag &tag, const Item::List &items)
{
    auto &ids = m_tagItems[tag.id()];
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

    const auto types = std::initializer_list<StorageInterface::FetchContentTypes>{
            StorageInterface::AllContent,
            StorageInterface::Tasks,
            StorageInterface::Notes,
            (StorageInterface::Tasks|StorageInterface::Notes)
    };

    for (const auto &type : types) {
        if (isContentTypesPopulated(type) && matchCollection(type, collection)) {
            m_collections << collection;
            return;
        }
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

        for (auto &itemList : m_tagItems)
            itemList.removeAll(itemId);
    }

    m_collectionItems.remove(collection.id());
}

void Cache::onTagAdded(const Tag &tag)
{
    if (!m_tagListPopulated)
        return;
    const auto index = m_tags.indexOf(tag);
    if (index >= 0)
        m_tags[index] = tag;
    else
        m_tags.append(tag);
}

void Cache::onTagChanged(const Tag &tag)
{
    onTagAdded(tag);
}

void Cache::onTagRemoved(const Tag &tag)
{
    m_tags.removeAll(tag);
    m_tagItems.remove(tag.id());
}

void Cache::onItemAdded(const Item &item)
{
    bool needsInsert = false;
    const auto it = m_collectionItems.find(item.parentCollection().id());
    if (it != m_collectionItems.end()) {
        *it << item.id();
        needsInsert = true;
    }
    for (const auto &tag : item.tags()) {
        const auto it = m_tagItems.find(tag.id());
        if (it != m_tagItems.end()) {
            *it << item.id();
            needsInsert = true;
        }
    }
    if (needsInsert)
        m_items.insert(item.id(), item);

}

void Cache::onItemChanged(const Item &item)
{
    const auto oldItem = m_items.take(item.id());
    const auto oldTags = oldItem.tags();
    const auto newTags = item.tags();

    if (oldItem.parentCollection() != item.parentCollection()) {
        auto it = m_collectionItems.find(oldItem.parentCollection().id());
        if (it != m_collectionItems.end())
            it->removeAll(oldItem.id());

        it = m_collectionItems.find(item.parentCollection().id());
        if (it != m_collectionItems.end())
            it->append(item.id());
    }

    for (const auto &oldTag : oldTags) {
        if (!newTags.contains(oldTag) && m_tagItems.contains(oldTag.id())) {
            m_tagItems[oldTag.id()].removeAll(oldTag.id());
        }
    }

    for (const auto &newTag : newTags) {
        if (!oldItem.tags().contains(newTag) && m_tagItems.contains(newTag.id())) {
            m_tagItems[newTag.id()].append(item.id());
        }
    }

    const auto inPopulatedTag = std::any_of(newTags.cbegin(), newTags.cend(),
                                            [this](const Tag &tag) { return m_tagItems.contains(tag.id()); });

    if (inPopulatedTag || m_collectionItems.contains(item.parentCollection().id())) {
        m_items.insert(item.id(), item);
    }
}

void Cache::onItemRemoved(const Item &item)
{
    m_items.remove(item.id());
    for (auto &itemList : m_collectionItems)
        itemList.removeAll(item.id());
    for (auto &itemList : m_tagItems)
        itemList.removeAll(item.id());
}

bool Cache::matchCollection(StorageInterface::FetchContentTypes contentTypes, const Collection &collection) const
{
    return (contentTypes == StorageInterface::AllContent)
        || ((contentTypes & StorageInterface::Tasks) && m_serializer->isTaskCollection(collection));
}
