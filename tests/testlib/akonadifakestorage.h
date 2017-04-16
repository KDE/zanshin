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

    Akonadi::Collection defaultTaskCollection() Q_DECL_OVERRIDE;
    Akonadi::Collection defaultNoteCollection() Q_DECL_OVERRIDE;

    KJob *createItem(Akonadi::Item item, Akonadi::Collection collection) Q_DECL_OVERRIDE;
    KJob *updateItem(Akonadi::Item item, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;
    KJob *removeItem(Akonadi::Item item) Q_DECL_OVERRIDE;
    KJob *removeItems(Akonadi::Item::List items, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;
    KJob *moveItem(Akonadi::Item item, Akonadi::Collection collection, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;
    KJob *moveItems(Akonadi::Item::List items, Akonadi::Collection collection, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;

    KJob *createCollection(Akonadi::Collection collection, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;
    KJob *updateCollection(Akonadi::Collection collection, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;
    KJob *removeCollection(Akonadi::Collection collection, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;

    KJob *createTransaction() Q_DECL_OVERRIDE;

    KJob *createTag(Akonadi::Tag tag) Q_DECL_OVERRIDE;
    KJob *updateTag(Akonadi::Tag tag) Q_DECL_OVERRIDE;
    KJob *removeTag(Akonadi::Tag tag) Q_DECL_OVERRIDE;

    Akonadi::CollectionFetchJobInterface *fetchCollections(Akonadi::Collection collection, FetchDepth depth, FetchContentTypes types) Q_DECL_OVERRIDE;
    Akonadi::ItemFetchJobInterface *fetchItems(Akonadi::Collection collection) Q_DECL_OVERRIDE;
    Akonadi::ItemFetchJobInterface *fetchItem(Akonadi::Item item) Q_DECL_OVERRIDE;
    Akonadi::ItemFetchJobInterface *fetchTagItems(Akonadi::Tag tag) Q_DECL_OVERRIDE;
    Akonadi::TagFetchJobInterface *fetchTags() Q_DECL_OVERRIDE;

private:
    Akonadi::Tag::Id findId(const Akonadi::Tag &tag);
    Akonadi::Collection::Id findId(const Akonadi::Collection &collection);
    Akonadi::Item::Id findId(const Akonadi::Item &item);
    Akonadi::Collection::List collectChildren(const Akonadi::Collection &root);

    AkonadiFakeData *m_data;
};

}

#endif // TESTLIB_AKONADIFAKESTORAGE_H
