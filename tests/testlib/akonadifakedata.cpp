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

static const char s_contextListProperty[] = "ContextList";
static const char s_appName[] = "Zanshin";

// Should be in the serializer ideally ... but we don't link to that from here anyway.
static QStringList extractContextUids(const Akonadi::Item &taskItem)
{
    if (!taskItem.hasPayload<KCalCore::Todo::Ptr>())
        return {};
    auto todo = taskItem.payload<KCalCore::Todo::Ptr>();
    const QString contexts = todo->customProperty(s_appName, s_contextListProperty);
    return contexts.split(',', QString::SkipEmptyParts);
}

// Duplicated from the serializer
static QString contextUid(const Akonadi::Item &contextItem)
{
    auto contextTodo = contextItem.payload<KCalCore::Todo::Ptr>();
    return contextTodo->uid();
}

// Somewhat duplicated from the serializer
static void removeContextFromTask(const QString &contextUid, Akonadi::Item &item)
{
    auto todo = item.payload<KCalCore::Todo::Ptr>();
    const QString contexts = todo->customProperty(s_appName, s_contextListProperty);
    QStringList contextList = contexts.split(',', QString::SkipEmptyParts);
    contextList.removeAll(contextUid);
    if (contextList.isEmpty())
        todo->removeCustomProperty(s_appName, s_contextListProperty);
    else
        todo->setCustomProperty(s_appName, s_contextListProperty, contextList.join(','));
    item.setPayload<KCalCore::Todo::Ptr>(todo);
    Q_ASSERT(contextList == extractContextUids(item));
}

// Duplicate from the serializer
static bool isContext(const Akonadi::Item &item)
{
    if (!item.hasPayload<KCalCore::Todo::Ptr>())
        return false;

    auto todo = item.payload<KCalCore::Todo::Ptr>();
    return !todo->customProperty(s_appName, "Context").isEmpty();
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

AkonadiFakeData &AkonadiFakeData::operator=(const AkonadiFakeData &other)
{
    m_collections = other.m_collections;
    m_childCollections = other.m_childCollections;
    m_items = other.m_items;
    m_childItems = other.m_childItems;
    return *this;
}

Akonadi::Collection::List AkonadiFakeData::collections() const
{
    return m_collections.values().toVector();
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
    if (mimeTypes.contains(KCalCore::Todo::todoMimeType())) {
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

QStringList AkonadiFakeData::contextsUids() const
{
    return m_contexts.keys();
}

Akonadi::Item::List AkonadiFakeData::contexts() const
{
    return m_contexts.values().toVector();
}

Akonadi::Item AkonadiFakeData::contextItem(const QString &uid) const
{
    return m_contexts.value(uid);
}

void AkonadiFakeData::createContext(const Akonadi::Item &contextItem)
{
    const QString uid = contextUid(contextItem);
    Q_ASSERT(!m_contexts.contains(uid));
    m_contexts[uid] = contextItem;
    m_monitor->addItem(contextItem);
}

void AkonadiFakeData::modifyContext(const Akonadi::Item &contextItem)
{
    const QString uid = contextUid(contextItem);
    Q_ASSERT(m_contexts.contains(uid));
    const auto oldContextItem = m_contexts.take(uid);
    auto newItem = contextItem;
    newItem.setGid(oldContextItem.gid());
    newItem.setRemoteId(oldContextItem.remoteId());
    m_contexts[uid] = newItem;
    m_monitor->changeItem(newItem);
}

void AkonadiFakeData::removeContext(const Akonadi::Item &contextItem)
{
    const QString uid = contextUid(contextItem);
    Q_ASSERT(m_contexts.contains(uid));

    const auto ids = m_contextItems[uid];
    foreach (const auto &id, ids) {
        Q_ASSERT(m_items.contains(id));
        auto item = m_items.value(id);
        removeContextFromTask(uid, item);
        m_items[id] = item;
        m_monitor->changeItem(item);
    }
    m_contextItems.remove(uid);

    m_contexts.remove(uid);
    m_monitor->removeItem(Akonadi::Item(contextItem.id())); // Akonadi doesn't emit a full item
}

Akonadi::Item::List AkonadiFakeData::items() const
{
    return m_items.values().toVector();
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

Akonadi::Item::List AkonadiFakeData::contextItems(const QString &contextUid) const
{
    const auto ids = m_contextItems.value(contextUid);
    auto result = Akonadi::Item::List();
    result.reserve(ids.size());
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
    Q_ASSERT(item.isValid());
    Q_ASSERT(!m_items.contains(item.id()));
    m_items[item.id()] = item;

    const auto parentId = findParentId(item);
    m_childItems[parentId] << item.id();

    const QStringList contextUids = extractContextUids(item);
    foreach (const auto &contextUid, contextUids) {
        m_contextItems[contextUid] << item.id();
    }

    if (isContext(item)) {
        createContext(item);
    } else {
        m_monitor->addItem(reconstructItemDependencies(item));
    }
}

void AkonadiFakeData::modifyItem(const Akonadi::Item &item)
{
    Q_ASSERT(m_items.contains(item.id()));
    Q_ASSERT(item.isValid());

    const auto oldContexts = extractContextUids(m_items[item.id()]);
    const auto newContexts = extractContextUids(item);
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

    foreach (const auto &contextUid, oldContexts) {
        m_contextItems[contextUid].removeAll(newItem.id());
    }

    foreach (const auto &contextUid, newContexts) {
        Q_ASSERT(m_contexts.contains(contextUid));
        m_contextItems[contextUid] << newItem.id();
    }

    if (isContext(item)) {
        modifyContext(item);
    } else {
        m_monitor->changeItem(reconstructItemDependencies(newItem));
    }
}

void AkonadiFakeData::removeItem(const Akonadi::Item &item)
{
    Q_ASSERT(item.isValid());
    Q_ASSERT(m_items.contains(item.id()));
    const auto parentId = findParentId(m_items[item.id()]);
    const auto i = m_items.take(item.id());
    m_childItems[parentId].removeAll(item.id());

    foreach (const auto &contextUid, extractContextUids(item)) {
        m_contextItems[contextUid].removeAll(item.id());
    }

    if (isContext(item)) {
        removeContext(item);
    } else {
        // Akonadi doesn't emit the payload, but the ancestors are there
        m_monitor->removeItem(reconstructItemDependencies(Akonadi::Item(i.id())));
    }
}

Akonadi::MonitorInterface *AkonadiFakeData::createMonitor()
{
    auto monitor = new AkonadiFakeMonitor;
    QObject::connect(m_monitor.data(), &AkonadiFakeMonitor::collectionAdded,
                     monitor, &AkonadiFakeMonitor::addCollection);
    QObject::connect(m_monitor.data(), &AkonadiFakeMonitor::collectionChanged,
                     monitor, &AkonadiFakeMonitor::changeCollection);
    QObject::connect(m_monitor.data(), &AkonadiFakeMonitor::collectionSelectionChanged,
                     monitor, &AkonadiFakeMonitor::changeCollectionSelection);
    QObject::connect(m_monitor.data(), &AkonadiFakeMonitor::collectionRemoved,
                     monitor, &AkonadiFakeMonitor::removeCollection);
    QObject::connect(m_monitor.data(), &AkonadiFakeMonitor::itemAdded,
                     monitor, &AkonadiFakeMonitor::addItem);
    QObject::connect(m_monitor.data(), &AkonadiFakeMonitor::itemChanged,
                     monitor, &AkonadiFakeMonitor::changeItem);
    QObject::connect(m_monitor.data(), &AkonadiFakeMonitor::itemRemoved,
                     monitor, &AkonadiFakeMonitor::removeItem);
    QObject::connect(m_monitor.data(), &AkonadiFakeMonitor::itemMoved,
                     monitor, &AkonadiFakeMonitor::moveItem);
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

Akonadi::Collection::Id AkonadiFakeData::maxCollectionId() const
{
    if (m_collections.isEmpty())
        return 0;

    auto it = std::max_element(m_collections.constBegin(), m_collections.constEnd(),
                               idLessThan<Akonadi::Collection>);
    return it.key();
}

Akonadi::Item::Id AkonadiFakeData::maxItemId() const
{
    if (m_items.isEmpty())
        return 0;

    auto it = std::max_element(m_items.constBegin(), m_items.constEnd(),
                               idLessThan<Akonadi::Item>);
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
