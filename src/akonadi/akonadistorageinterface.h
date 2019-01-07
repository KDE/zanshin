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

#ifndef AKONADI_STORAGEINTERFACE_H
#define AKONADI_STORAGEINTERFACE_H

#include <QFlags>
#include <AkonadiCore/Item>
#include <AkonadiCore/Tag>

class KJob;
class QObject;
class QByteArray;

namespace Akonadi {

class Collection;
class CollectionFetchJobInterface;
class ItemFetchJobInterface;
class TagFetchJobInterface;


class StorageInterface
{
public:
    typedef QSharedPointer<StorageInterface> Ptr;

    enum FetchDepth {
        Base,
        FirstLevel,
        Recursive
    };

    StorageInterface();
    virtual ~StorageInterface();

    virtual Akonadi::Collection defaultCollection() = 0;

    virtual KJob *createItem(Akonadi::Item item, Akonadi::Collection collection) = 0;
    virtual KJob *updateItem(Akonadi::Item item, QObject *parent = nullptr) = 0;
    virtual KJob *removeItem(Akonadi::Item item) = 0;
    virtual KJob *removeItems(Item::List items, QObject *parent = nullptr) = 0;
    virtual KJob *moveItem(Item item, Collection collection, QObject *parent = nullptr) = 0;
    virtual KJob *moveItems(Item::List item, Collection collection, QObject *parent = nullptr) = 0;

    virtual KJob *createCollection(Collection collection, QObject *parent = nullptr) = 0;
    virtual KJob *updateCollection(Collection collection, QObject *parent = nullptr) = 0;
    virtual KJob *removeCollection(Collection collection, QObject *parent = nullptr) = 0;

    virtual KJob *createTransaction() = 0;

    virtual KJob *createTag(Akonadi::Tag tag) = 0;
    virtual KJob *updateTag(Akonadi::Tag tag) = 0;
    virtual KJob *removeTag(Akonadi::Tag tag) = 0;

    virtual CollectionFetchJobInterface *fetchCollections(Akonadi::Collection collection, FetchDepth depth) = 0;
    virtual ItemFetchJobInterface *fetchItems(Akonadi::Collection collection) = 0;
    virtual ItemFetchJobInterface *fetchItem(Akonadi::Item item) = 0;
    virtual ItemFetchJobInterface *fetchTagItems(Akonadi::Tag tag) = 0;
    virtual TagFetchJobInterface *fetchTags() = 0;
};

}

#endif // AKONADI_STORAGEINTERFACE_H
