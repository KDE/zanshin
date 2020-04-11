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

    KJob *createTransaction() override;

    Akonadi::CollectionFetchJobInterface *fetchCollections(Akonadi::Collection collection, FetchDepth depth) override;
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
