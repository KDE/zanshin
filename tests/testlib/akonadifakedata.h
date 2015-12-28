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

#ifndef TESTLIB_AKONADIFAKEDATA_H
#define TESTLIB_AKONADIFAKEDATA_H

#include <QObject>

#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>
#include <AkonadiCore/Tag>

#include "testlib/akonadifakestoragebehavior.h"

namespace Akonadi {
class MonitorInterface;
class StorageInterface;
}

namespace Testlib {

class AkonadiFakeMonitor;

class AkonadiFakeData
{
public:
    AkonadiFakeData();
    AkonadiFakeData(const AkonadiFakeData &other);
    ~AkonadiFakeData();

    Akonadi::Collection::List collections() const;
    Akonadi::Collection::List childCollections(Akonadi::Collection::Id parentId) const;
    Akonadi::Collection collection(Akonadi::Collection::Id id) const;
    void createCollection(const Akonadi::Collection &collection);
    void modifyCollection(const Akonadi::Collection &collection);
    void removeCollection(const Akonadi::Collection &collection);

    Akonadi::Tag::List tags() const;
    Akonadi::Tag tag(Akonadi::Tag::Id id) const;
    void createTag(const Akonadi::Tag &tag);
    void modifyTag(const Akonadi::Tag &tag);
    void removeTag(const Akonadi::Tag &tag);

    Akonadi::Item::List items() const;
    Akonadi::Item::List childItems(Akonadi::Collection::Id parentId) const;
    Akonadi::Item::List tagItems(Akonadi::Tag::Id tagId) const;
    Akonadi::Item item(Akonadi::Item::Id id) const;
    void createItem(const Akonadi::Item &item);
    void modifyItem(const Akonadi::Item &item);
    void removeItem(const Akonadi::Item &item);

    Akonadi::MonitorInterface *createMonitor();
    Akonadi::StorageInterface *createStorage();

    Akonadi::Collection::Id maxCollectionId() const;
    Akonadi::Item::Id maxItemId() const;
    Akonadi::Tag::Id maxTagId() const;

    Akonadi::Collection reconstructAncestors(const Akonadi::Collection &collection,
                                             const Akonadi::Collection &root = Akonadi::Collection::root()) const;
    Akonadi::Item reconstructItemDependencies(const Akonadi::Item &item,
                                              const Akonadi::Collection &root = Akonadi::Collection::root()) const;

    const AkonadiFakeStorageBehavior &storageBehavior() const;
    AkonadiFakeStorageBehavior &storageBehavior();

private:
    QHash<Akonadi::Collection::Id, Akonadi::Collection> m_collections;
    QHash<Akonadi::Collection::Id, QList<Akonadi::Collection::Id>> m_childCollections;

    QHash<Akonadi::Tag::Id, Akonadi::Tag> m_tags;

    QHash<Akonadi::Item::Id, Akonadi::Item> m_items;
    QHash<Akonadi::Collection::Id, QList<Akonadi::Item::Id>> m_childItems;
    QHash<Akonadi::Tag::Id, QList<Akonadi::Item::Id>> m_tagItems;

    QScopedPointer<AkonadiFakeMonitor> m_monitor;

    AkonadiFakeStorageBehavior m_storageBehavior;
};

}

#endif // TESTLIB_AKONADIFAKEDATA_H
