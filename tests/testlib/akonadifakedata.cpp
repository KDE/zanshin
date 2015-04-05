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
    m_monitor->addCollection(collection);
}

void AkonadiFakeData::modifyCollection(const Akonadi::Collection &collection)
{
    Q_ASSERT(m_collections.contains(collection.id()));

    const auto oldParentId = findParentId(m_collections[collection.id()]);
    m_collections[collection.id()] = collection;
    const auto parentId = findParentId(collection);

    if (oldParentId != parentId) {
        m_childCollections[oldParentId].removeAll(collection.id());
        m_childCollections[parentId] << collection.id();
    }

    m_monitor->changeCollection(collection);
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
    m_monitor->addItem(item);
}

void AkonadiFakeData::modifyItem(const Akonadi::Item &item)
{
    Q_ASSERT(m_items.contains(item.id()));

    const auto oldParentId = findParentId(m_items[item.id()]);
    m_items[item.id()] = item;
    const auto parentId = findParentId(item);

    if (oldParentId != parentId) {
        m_childItems[oldParentId].removeAll(item.id());
        m_childItems[parentId] << item.id();
        m_monitor->moveItem(item);
    }

    m_monitor->changeItem(item);
}

void AkonadiFakeData::removeItem(const Akonadi::Item &item)
{
    Q_ASSERT(m_items.contains(item.id()));
    const auto parentId = findParentId(m_items[item.id()]);
    const auto i = m_items.take(item.id());
    m_childItems[parentId].removeAll(item.id());
    m_monitor->removeItem(i);
}

Akonadi::MonitorInterface *AkonadiFakeData::createMonitor()
{
    auto monitor = new AkonadiFakeMonitor;
    QObject::connect(m_monitor.data(), SIGNAL(collectionAdded(Akonadi::Collection)),
                     monitor, SLOT(addCollection(Akonadi::Collection)));
    QObject::connect(m_monitor.data(), SIGNAL(collectionChanged(Akonadi::Collection)),
                     monitor, SLOT(changeCollection(Akonadi::Collection)));
    QObject::connect(m_monitor.data(), SIGNAL(collectionRemoved(Akonadi::Collection)),
                     monitor, SLOT(removeCollection(Akonadi::Collection)));
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
