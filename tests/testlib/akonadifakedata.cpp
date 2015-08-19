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

#include "akonadifakedata.h"
#include "akonadifakemonitor.h"
#include "akonadifakestorage.h"

#include <Akonadi/Notes/NoteUtils>
#include <KCalCore/Todo>

#include "akonadi/akonadiapplicationselectedattribute.h"

#include <algorithm>

using namespace Testlib;

template<class Entity>
static Akonadi::Collection::Id findParentId(const Entity &entity)
{
    const auto parent = entity.parentCollection();
    return parent.isValid() ? parent.id()
                            : Akonadi::Collection::root().id();
}

AkonadiFakeData::AkonadiFakeData()
    : m_monitor(new AkonadiFakeMonitor)
{
}

AkonadiFakeData::AkonadiFakeData(const AkonadiFakeData &other)
    : m_collections(other.m_collections),
      m_childCollections(other.m_childCollections),
      m_items(other.m_items),
      m_childItems(other.m_childItems),
      m_monitor(new AkonadiFakeMonitor)
{

}

AkonadiFakeData::~AkonadiFakeData()
{
}

Akonadi::Collection::List AkonadiFakeData::collections() const
{
    return m_collections.values();
}

Akonadi::Collection::List AkonadiFakeData::childCollections(Akonadi::Collection::Id parentId) const
{
    if (!m_childCollections.contains(parentId))
        return {};

    const auto ids = m_childCollections.value(parentId);
    auto result = Akonadi::Collection::List();
    std::transform(std::begin(ids), std::end(ids),
                   std::back_inserter(result),
                   [this] (Akonadi::Collection::Id id) {
                       Q_ASSERT(m_collections.contains(id));
                       return m_collections.value(id);
                   });
    return result;
}

Akonadi::Collection AkonadiFakeData::collection(Akonadi::Collection::Id id) const
{
    if (!m_collections.contains(id))
        return Akonadi::Collection();

    return m_collections.value(id);
}

void AkonadiFakeData::createCollection(const Akonadi::Collection &collection)
{
    Q_ASSERT(!m_collections.contains(collection.id()));
    m_collections[collection.id()] = collection;

    const auto parentId = findParentId(collection);
    m_childCollections[parentId] << collection.id();
    m_monitor->addCollection(reconstructAncestors(collection));
}

void AkonadiFakeData::modifyCollection(const Akonadi::Collection &collection)
{
    Q_ASSERT(m_collections.contains(collection.id()));

    const auto oldParentId = findParentId(m_collections[collection.id()]);
    const auto oldCollection = m_collections.take(collection.id());
    auto newCollection = collection;
    newCollection.setRemoteId(oldCollection.remoteId());
    if (!newCollection.parentCollection().isValid())
        newCollection.setParentCollection(oldCollection.parentCollection());
    if (newCollection.name().isEmpty())
        newCollection.setName(oldCollection.name());
    if (newCollection.contentMimeTypes().isEmpty())
        newCollection.setContentMimeTypes(oldCollection.contentMimeTypes());

    m_collections[newCollection.id()] = newCollection;
    const auto parentId = findParentId(newCollection);

    if (oldParentId != parentId) {
        m_childCollections[oldParentId].removeAll(newCollection.id());
        m_childCollections[parentId] << newCollection.id();
    }

    auto notifiedCollection = reconstructAncestors(newCollection);
    m_monitor->changeCollection(notifiedCollection);

    const auto mimeTypes = collection.contentMimeTypes();
    if (mimeTypes.contains(KCalCore::Todo::todoMimeType())
     || mimeTypes.contains(Akonadi::NoteUtils::noteMimeType())) {
        const auto oldAttribute = oldCollection.attribute<Akonadi::ApplicationSelectedAttribute>();
        const auto oldSelected = oldAttribute ? oldAttribute->isSelected() : true;
        const auto newAttribute = newCollection.attribute<Akonadi::ApplicationSelectedAttribute>();
        const auto newSelected = newAttribute ? newAttribute->isSelected() : true;

        if (oldSelected != newSelected) {
            m_monitor->changeCollectionSelection(notifiedCollection);
        }
    }
}

void AkonadiFakeData::removeCollection(const Akonadi::Collection &collection)
{
    Q_ASSERT(m_collections.contains(collection.id()));

    const auto childCollections = m_childCollections[collection.id()];
    foreach (const auto &childId, childCollections) {
        removeCollection(Akonadi::Collection(childId));
    }
    m_childCollections.remove(collection.id());

    const auto childItems = m_childItems[collection.id()];
    foreach (const auto &childId, childItems) {
        removeItem(Akonadi::Item(childId));
    }
    m_childItems.remove(collection.id());

    const auto parentId = findParentId(m_collections[collection.id()]);
    const auto col = m_collections.take(collection.id());
    m_childCollections[parentId].removeAll(collection.id());

    m_monitor->removeCollection(col);
}

Akonadi::Tag::List AkonadiFakeData::tags() const
{
    return m_tags.values();
}

Akonadi::Tag AkonadiFakeData::tag(Akonadi::Tag::Id id) const
{
    if (!m_tags.contains(id))
        return Akonadi::Tag();

    return m_tags.value(id);
}

void AkonadiFakeData::createTag(const Akonadi::Tag &tag)
{
    Q_ASSERT(!m_tags.contains(tag.id()));
    m_tags[tag.id()] = tag;
    m_monitor->addTag(tag);
}

void AkonadiFakeData::modifyTag(const Akonadi::Tag &tag)
{
    Q_ASSERT(m_tags.contains(tag.id()));
    const auto oldTag = m_tags.take(tag.id());
    auto newTag = tag;
    newTag.setGid(oldTag.gid());
    newTag.setRemoteId(oldTag.remoteId());
    m_tags[tag.id()] = newTag;
    m_monitor->changeTag(newTag);
}

void AkonadiFakeData::removeTag(const Akonadi::Tag &tag)
{
    Q_ASSERT(m_tags.contains(tag.id()));

    const auto ids = m_tagItems[tag.id()];
    foreach (const auto &id, ids) {
        Q_ASSERT(m_items.contains(id));
        auto item = m_items.value(id);
        item.clearTag(tag);
        m_items[id] = item;
        m_monitor->changeItem(item);
    }
    m_tagItems.remove(tag.id());

    m_tags.remove(tag.id());
    m_monitor->removeTag(tag);
}

Akonadi::Item::List AkonadiFakeData::items() const
{
    return m_items.values();
}

Akonadi::Item::List AkonadiFakeData::childItems(Akonadi::Collection::Id parentId) const
{
    if (!m_childItems.contains(parentId))
        return {};

    const auto ids = m_childItems.value(parentId);
    auto result = Akonadi::Item::List();
    std::transform(std::begin(ids), std::end(ids),
                   std::back_inserter(result),
                   [this] (Akonadi::Item::Id id) {
                       Q_ASSERT(m_items.contains(id));
                       return m_items.value(id);
                   });
    return result;
}

Akonadi::Item::List AkonadiFakeData::tagItems(Akonadi::Tag::Id tagId) const
{
    if (!m_tagItems.contains(tagId))
        return {};

    const auto ids = m_tagItems.value(tagId);
    auto result = Akonadi::Item::List();
    std::transform(std::begin(ids), std::end(ids),
                   std::back_inserter(result),
                   [this] (Akonadi::Item::Id id) {
                       Q_ASSERT(m_items.contains(id));
                       return m_items.value(id);
                   });
    return result;
}

Akonadi::Item AkonadiFakeData::item(Akonadi::Item::Id id) const
{
    if (!m_items.contains(id))
        return {};

    return m_items.value(id);
}

void AkonadiFakeData::createItem(const Akonadi::Item &item)
{
    Q_ASSERT(!m_items.contains(item.id()));
    m_items[item.id()] = item;

    const auto parentId = findParentId(item);
    m_childItems[parentId] << item.id();

    foreach (const auto &tag, item.tags()) {
        Q_ASSERT(m_tags.contains(tag.id()));
        m_tagItems[tag.id()] << item.id();
    }

    m_monitor->addItem(reconstructItemDependencies(item));
}

void AkonadiFakeData::modifyItem(const Akonadi::Item &item)
{
    Q_ASSERT(m_items.contains(item.id()));

    const auto oldTags = m_items[item.id()].tags();
    const auto oldParentId = findParentId(m_items[item.id()]);
    const auto oldItem = m_items.take(item.id());
    auto newItem = item;
    newItem.setRemoteId(oldItem.remoteId());
    if (!newItem.parentCollection().isValid())
        newItem.setParentCollection(oldItem.parentCollection());

    m_items[newItem.id()] = newItem;
    const auto parentId = findParentId(newItem);

    if (oldParentId != parentId) {
        m_childItems[oldParentId].removeAll(newItem.id());
        m_childItems[parentId] << newItem.id();
        m_monitor->moveItem(reconstructItemDependencies(newItem));
    }

    foreach (const auto &tag, oldTags) {
        m_tagItems[tag.id()].removeAll(newItem.id());
    }

    foreach (const auto &tag, newItem.tags()) {
        Q_ASSERT(m_tags.contains(tag.id()));
        m_tagItems[tag.id()] << newItem.id();
    }

    m_monitor->changeItem(reconstructItemDependencies(newItem));
}

void AkonadiFakeData::removeItem(const Akonadi::Item &item)
{
    Q_ASSERT(m_items.contains(item.id()));
    const auto parentId = findParentId(m_items[item.id()]);
    const auto i = m_items.take(item.id());
    m_childItems[parentId].removeAll(item.id());

    foreach (const Akonadi::Tag &tag, item.tags()) {
        m_tagItems[tag.id()].removeAll(item.id());
    }

    m_monitor->removeItem(reconstructItemDependencies(i));
}

Akonadi::MonitorInterface *AkonadiFakeData::createMonitor()
{
    auto monitor = new AkonadiFakeMonitor;
    QObject::connect(m_monitor.data(), SIGNAL(collectionAdded(Akonadi::Collection)),
                     monitor, SLOT(addCollection(Akonadi::Collection)));
    QObject::connect(m_monitor.data(), SIGNAL(collectionChanged(Akonadi::Collection)),
                     monitor, SLOT(changeCollection(Akonadi::Collection)));
    QObject::connect(m_monitor.data(), SIGNAL(collectionSelectionChanged(Akonadi::Collection)),
                     monitor, SLOT(changeCollectionSelection(Akonadi::Collection)));
    QObject::connect(m_monitor.data(), SIGNAL(collectionRemoved(Akonadi::Collection)),
                     monitor, SLOT(removeCollection(Akonadi::Collection)));
    QObject::connect(m_monitor.data(), SIGNAL(tagAdded(Akonadi::Tag)),
                     monitor, SLOT(addTag(Akonadi::Tag)));
    QObject::connect(m_monitor.data(), SIGNAL(tagChanged(Akonadi::Tag)),
                     monitor, SLOT(changeTag(Akonadi::Tag)));
    QObject::connect(m_monitor.data(), SIGNAL(tagRemoved(Akonadi::Tag)),
                     monitor, SLOT(removeTag(Akonadi::Tag)));
    QObject::connect(m_monitor.data(), SIGNAL(itemAdded(Akonadi::Item)),
                     monitor, SLOT(addItem(Akonadi::Item)));
    QObject::connect(m_monitor.data(), SIGNAL(itemChanged(Akonadi::Item)),
                     monitor, SLOT(changeItem(Akonadi::Item)));
    QObject::connect(m_monitor.data(), SIGNAL(itemRemoved(Akonadi::Item)),
                     monitor, SLOT(removeItem(Akonadi::Item)));
    QObject::connect(m_monitor.data(), SIGNAL(itemMoved(Akonadi::Item)),
                     monitor, SLOT(moveItem(Akonadi::Item)));
    return monitor;
}

Akonadi::StorageInterface *AkonadiFakeData::createStorage()
{
    return new AkonadiFakeStorage(this);
}

template<typename T>
bool idLessThan(const T &left, const T &right)
{
    return left.id() < right.id();
}

Akonadi::Entity::Id AkonadiFakeData::maxCollectionId() const
{
    if (m_collections.isEmpty())
        return 0;

    auto it = std::max_element(m_collections.constBegin(), m_collections.constEnd(),
                               idLessThan<Akonadi::Collection>);
    return it.key();
}

Akonadi::Entity::Id AkonadiFakeData::maxItemId() const
{
    if (m_items.isEmpty())
        return 0;

    auto it = std::max_element(m_items.constBegin(), m_items.constEnd(),
                               idLessThan<Akonadi::Item>);
    return it.key();
}

Akonadi::Tag::Id AkonadiFakeData::maxTagId() const
{
    if (m_tags.isEmpty())
        return 0;

    auto it = std::max_element(m_tags.constBegin(), m_tags.constEnd(),
                               idLessThan<Akonadi::Tag>);
    return it.key();
}

Akonadi::Collection AkonadiFakeData::reconstructAncestors(const Akonadi::Collection &collection,
                                                          const Akonadi::Collection &root) const
{
    if (!collection.isValid())
        return Akonadi::Collection::root();

    if (collection == root)
        return collection;

    auto parent = collection.parentCollection();
    auto reconstructedParent = reconstructAncestors(m_collections.value(parent.id()), root);

    auto result = collection;
    result.setParentCollection(reconstructedParent);
    return result;
}

Akonadi::Item AkonadiFakeData::reconstructItemDependencies(const Akonadi::Item &item, const Akonadi::Collection &root) const
{
    auto result = item;
    result.setParentCollection(reconstructAncestors(item.parentCollection(), root));

    auto tags = item.tags();
    std::transform(tags.constBegin(), tags.constEnd(),
                   tags.begin(),
                   [=] (const Akonadi::Tag &t) {
                       return tag(t.id());
                   });
    result.setTags(tags);

    return result;
}

const AkonadiFakeStorageBehavior &AkonadiFakeData::storageBehavior() const
{
    return m_storageBehavior;
}

AkonadiFakeStorageBehavior &AkonadiFakeData::storageBehavior()
{
    return m_storageBehavior;
}
