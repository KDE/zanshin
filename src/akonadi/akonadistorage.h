/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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

#ifndef AKONADI_STORAGE_H
#define AKONADI_STORAGE_H

#include "akonadistorageinterface.h"

#include <AkonadiCore/CollectionFetchJob>

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
