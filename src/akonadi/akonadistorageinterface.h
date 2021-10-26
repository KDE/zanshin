/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_STORAGEINTERFACE_H
#define AKONADI_STORAGEINTERFACE_H

#include <QFlags>
#include <AkonadiCore/Akonadi/Item>

class KJob;
class QObject;
class QByteArray;

namespace Akonadi {

class Collection;
class CollectionFetchJobInterface;
class ItemFetchJobInterface;


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
    virtual KJob *updateItem(Akonadi::Item item, QObject *parent) = 0;
    virtual KJob *removeItem(Akonadi::Item item, QObject *parent) = 0;
    virtual KJob *removeItems(Item::List items, QObject *parent) = 0;
    virtual KJob *moveItem(Item item, Collection collection, QObject *parent) = 0;
    virtual KJob *moveItems(Item::List item, Collection collection, QObject *parent) = 0;

    virtual KJob *createCollection(Collection collection, QObject *parent) = 0;
    virtual KJob *updateCollection(Collection collection, QObject *parent) = 0;
    virtual KJob *removeCollection(Collection collection, QObject *parent) = 0;

    virtual KJob *createTransaction(QObject *parent) = 0;

    virtual CollectionFetchJobInterface *fetchCollections(Akonadi::Collection collection, FetchDepth depth, QObject *parent) = 0;
    virtual ItemFetchJobInterface *fetchItems(Akonadi::Collection collection, QObject *parent) = 0;
    virtual ItemFetchJobInterface *fetchItem(Akonadi::Item item, QObject *parent) = 0;
};

}

#endif // AKONADI_STORAGEINTERFACE_H
