/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef TESTLIB_AKONADIFAKESTORAGE_H
#define TESTLIB_AKONADIFAKESTORAGE_H

#include "akonadi/akonadistorageinterface.h"

namespace Testlib {

class AkonadiFakeData;

class AkonadiFakeStorage : public Akonadi::StorageInterface
{
public:
    explicit AkonadiFakeStorage(AkonadiFakeData *data);

    Akonadi::Collection defaultCollection() override;

    KJob *createItem(Akonadi::Item item, Akonadi::Collection collection) override;
    KJob *updateItem(Akonadi::Item item, QObject *parent) override;
    KJob *removeItem(Akonadi::Item item, QObject *parent) override;
    KJob *removeItems(Akonadi::Item::List items, QObject *parent) override;
    KJob *moveItem(Akonadi::Item item, Akonadi::Collection collection, QObject *parent) override;
    KJob *moveItems(Akonadi::Item::List items, Akonadi::Collection collection, QObject *parent) override;

    KJob *createCollection(Akonadi::Collection collection, QObject *parent) override;
    KJob *updateCollection(Akonadi::Collection collection, QObject *parent) override;
    KJob *removeCollection(Akonadi::Collection collection, QObject *parent) override;

    KJob *createTransaction(QObject *parent) override;

    Akonadi::CollectionFetchJobInterface *fetchCollections(Akonadi::Collection collection, FetchDepth depth, QObject *parent) override;
    Akonadi::ItemFetchJobInterface *fetchItems(Akonadi::Collection collection, QObject *parent) override;
    Akonadi::ItemFetchJobInterface *fetchItem(Akonadi::Item item, QObject *parent) override;

private:
    Akonadi::Collection::Id findId(const Akonadi::Collection &collection);
    Akonadi::Item::Id findId(const Akonadi::Item &item);
    Akonadi::Collection::List collectChildren(const Akonadi::Collection &root);

    AkonadiFakeData *m_data;
};

}

#endif // TESTLIB_AKONADIFAKESTORAGE_H
