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

#ifndef AKONADI_CACHE_H
#define AKONADI_CACHE_H

#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>
#include <AkonadiCore/Tag>

#include "akonadi/akonadimonitorinterface.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

namespace Akonadi {

class Cache : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<Cache> Ptr;

    explicit Cache(const SerializerInterface::Ptr &serializer,
                   const MonitorInterface::Ptr &monitor,
                   QObject *parent = nullptr);

    bool isCollectionListPopulated() const;
    Akonadi::Collection::List collections() const;
    Akonadi::Collection::List allCollections() const;
    bool isCollectionKnown(Collection::Id id) const;
    Collection collection(Collection::Id id) const;
    bool isCollectionPopulated(Collection::Id id) const;
    Item::List items(const Collection &collection) const;

    void setCollections(const Collection::List &collections);
    void populateCollection(const Collection &collection, const Item::List &items);


    bool isTagListPopulated() const;
    Tag::List tags() const;
    bool isTagKnown(Tag::Id id) const;
    Tag tag(Tag::Id id) const;
    bool isTagPopulated(Tag::Id id) const;
    Item::List items(const Tag &tag) const;

    void setTags(const Tag::List &tags);
    void populateTag(const Tag &tag, const Item::List &items);

    Item item(Item::Id id) const;

private slots:
    void onCollectionAdded(const Collection &collection);
    void onCollectionChanged(const Collection &collection);
    void onCollectionRemoved(const Collection &collection);

    void onTagAdded(const Tag &tag);
    void onTagChanged(const Tag &tag);
    void onTagRemoved(const Tag &tag);

    void onItemAdded(const Item &item);
    void onItemChanged(const Item &item);
    void onItemRemoved(const Item &item);

private:
    SerializerInterface::Ptr m_serializer;
    MonitorInterface::Ptr m_monitor;

    bool m_collectionListPopulated;
    Collection::List m_collections;
    QHash<Collection::Id, QVector<Item::Id>> m_collectionItems;

    bool m_tagListPopulated;
    Tag::List m_tags;
    QHash<Tag::Id, QVector<Item::Id>> m_tagItems;

    QHash<Item::Id, Item> m_items;
};

}

#endif // AKONADI_CACHE_H
