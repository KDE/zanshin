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
#include <Akonadi/Item>
#include <Akonadi/Tag>

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
    enum FetchDepth {
        Base,
        FirstLevel,
        Recursive
    };

    enum FetchContentType {
        Tasks = 0x1,
        Notes = 0x2
    };
    Q_DECLARE_FLAGS(FetchContentTypes, FetchContentType)

    StorageInterface();
    virtual ~StorageInterface();

    virtual Akonadi::Collection defaultTaskCollection() = 0;
    virtual Akonadi::Collection defaultNoteCollection() = 0;

    virtual KJob *createItem(Akonadi::Item item, Akonadi::Collection collection) = 0;
    virtual KJob *updateItem(Akonadi::Item item, QObject *parent = 0) = 0;
    virtual KJob *removeItem(Akonadi::Item item) = 0;
    virtual KJob *removeItems(Item::List items, QObject *parent = 0) = 0;
    virtual KJob *moveItem(Item item, Collection collection, QObject *parent = 0) = 0;
    virtual KJob *moveItems(Item::List item, Collection collection, QObject *parent = 0) = 0;

    virtual KJob *updateCollection(Collection collection, QObject *parent = 0) = 0;

    virtual KJob *createTransaction() = 0;

    virtual KJob *createTag(Akonadi::Tag tag) = 0;

    virtual CollectionFetchJobInterface *fetchCollections(Akonadi::Collection collection, FetchDepth depth, FetchContentTypes types) = 0;
    virtual ItemFetchJobInterface *fetchItems(Akonadi::Collection collection) = 0;
    virtual ItemFetchJobInterface *fetchItem(Akonadi::Item item) = 0;
    virtual TagFetchJobInterface *fetchTags() = 0;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Akonadi::StorageInterface::FetchContentTypes)

#endif // AKONADI_STORAGEINTERFACE_H
