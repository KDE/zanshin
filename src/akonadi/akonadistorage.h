/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_STORAGE_H
#define AKONADI_STORAGE_H

#include "akonadistorageinterface.h"

#include <AkonadiCore/Akonadi/CollectionFetchJob>

class ItemJob;
namespace Akonadi {

class Storage : public StorageInterface
{
public:
    Storage();
    virtual ~Storage();

    Akonadi::Collection defaultCollection() override;

    KJob *createItem(Item item, Collection collection) override;
    KJob *updateItem(Item item, QObject *parent) override;
    KJob *removeItem(Akonadi::Item item, QObject *parent) override;
    KJob *removeItems(Item::List items, QObject *parent) override;
    KJob *moveItem(Item item, Collection collection, QObject *parent) override;
    KJob *moveItems(Item::List item, Collection collection, QObject *parent) override;

    KJob *createCollection(Collection collection, QObject *parent) override;
    KJob *updateCollection(Collection collection, QObject *parent) override;
    KJob *removeCollection(Collection collection, QObject *parent) override;

    KJob *createTransaction(QObject *parent) override;

    CollectionFetchJobInterface *fetchCollections(Akonadi::Collection collection, FetchDepth depth, QObject *parent) override;
    ItemFetchJobInterface *fetchItems(Akonadi::Collection collection, QObject *parent) override;
    ItemFetchJobInterface *fetchItem(Akonadi::Item item, QObject *parent) override;

    ItemFetchJobInterface *fetchItemsWithTags(Akonadi::Collection collection);

private:
    CollectionFetchJob::Type jobTypeFromDepth(StorageInterface::FetchDepth depth);
    void configureItemFetchJob(ItemJob *job);
};

}

#endif // AKONADI_STORAGE_H
