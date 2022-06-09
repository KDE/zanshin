/*
 * SPDX-FileCopyrightText: 2017 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_CACHE_H
#define AKONADI_CACHE_H

#include <Akonadi/Collection>
#include <Akonadi/Item>

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

    Item item(Item::Id id) const;

private slots:
    void onCollectionAdded(const Collection &collection);
    void onCollectionChanged(const Collection &collection);
    void onCollectionRemoved(const Collection &collection);

    void onItemAdded(const Item &item);
    void onItemChanged(const Item &item);
    void onItemRemoved(const Item &item);

private:
    SerializerInterface::Ptr m_serializer;
    MonitorInterface::Ptr m_monitor;

    bool m_collectionListPopulated;
    Collection::List m_collections;
    QHash<Collection::Id, QVector<Item::Id>> m_collectionItems;

    QHash<Item::Id, Item> m_items;
};

}

#endif // AKONADI_CACHE_H
