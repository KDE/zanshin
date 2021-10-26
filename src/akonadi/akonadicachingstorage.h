/*
 * SPDX-FileCopyrightText: 2015 Mario Bensi <mbensi@ipsquad.net>
   SPDX-FileCopyrightText: 2017 Kevin Ottens <ervin@kde.org>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
   */


#ifndef AKONADI_CACHING_STORAGE_H
#define AKONADI_CACHING_STORAGE_H

#include "akonadistorageinterface.h"
#include "akonadicache.h"

namespace Akonadi {

class CachingStorage : public StorageInterface
{
public:
    explicit CachingStorage(const Cache::Ptr &cache, const StorageInterface::Ptr &storage);
    virtual ~CachingStorage();

    Akonadi::Collection defaultCollection() override;

    KJob *createItem(Item item, Collection collection) override;
    KJob *updateItem(Item item, QObject *parent) override;
    KJob *removeItem(Akonadi::Item item, QObject *parent) override;
    KJob *removeItems(Item::List items, QObject *parent) override;
    KJob *moveItem(Item item, Collection collection, QObject *parent) override;
    KJob *moveItems(Item::List item, Collection collection, QObject *parent = nullptr) override;

    KJob *createCollection(Collection collection, QObject *parent) override;
    KJob *updateCollection(Collection collection, QObject *parent) override;
    KJob *removeCollection(Collection collection, QObject *parent) override;

    KJob *createTransaction(QObject *parent) override;

    CollectionFetchJobInterface *fetchCollections(Akonadi::Collection collection, FetchDepth depth, QObject *parent) override;
    ItemFetchJobInterface *fetchItems(Akonadi::Collection collection, QObject *parent) override;
    ItemFetchJobInterface *fetchItem(Akonadi::Item item, QObject *parent) override;

private:
    Cache::Ptr m_cache;
    StorageInterface::Ptr m_storage;
};

}

#endif // AKONADI_CACHING_STORAGE_H
