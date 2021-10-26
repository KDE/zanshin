/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef TESTLIB_AKONADIFAKEDATA_H
#define TESTLIB_AKONADIFAKEDATA_H

#include <QObject>

#include <AkonadiCore/Akonadi/Collection>
#include <AkonadiCore/Akonadi/Item>

#include "testlib/akonadifakestoragebehavior.h"

namespace Akonadi {
class MonitorInterface;
class StorageInterface;
}

namespace Utils {
class DependencyManager;
}

namespace Testlib {

class AkonadiFakeMonitor;

class AkonadiFakeData
{
public:
    AkonadiFakeData();
    AkonadiFakeData(const AkonadiFakeData &other);
    ~AkonadiFakeData();

    AkonadiFakeData &operator=(const AkonadiFakeData &other);

    Akonadi::Collection::List collections() const;
    Akonadi::Collection::List childCollections(Akonadi::Collection::Id parentId) const;
    Akonadi::Collection collection(Akonadi::Collection::Id id) const;
    void createCollection(const Akonadi::Collection &collection);
    void modifyCollection(const Akonadi::Collection &collection);
    void removeCollection(const Akonadi::Collection &collection);

    QStringList contextsUids() const;
    Akonadi::Item::List contexts() const;
    Akonadi::Item contextItem(const QString &uid) const;
    void createContext(const Akonadi::Item &item);
    void modifyContext(const Akonadi::Item &item);
    void removeContext(const Akonadi::Item &item);

    Akonadi::Item::List items() const;
    Akonadi::Item::List childItems(Akonadi::Collection::Id parentId) const;
    Akonadi::Item::List contextItems(const QString& contextUid) const;
    Akonadi::Item item(Akonadi::Item::Id id) const;
    void createItem(const Akonadi::Item &item);
    void modifyItem(const Akonadi::Item &item);
    void removeItem(const Akonadi::Item &item);

    Akonadi::MonitorInterface *createMonitor();
    Akonadi::StorageInterface *createStorage();
    std::unique_ptr<Utils::DependencyManager> createDependencies();

    Akonadi::Collection::Id maxCollectionId() const;
    Akonadi::Item::Id maxItemId() const;

    Akonadi::Collection reconstructAncestors(const Akonadi::Collection &collection,
                                             const Akonadi::Collection &root = Akonadi::Collection::root()) const;
    Akonadi::Item reconstructItemDependencies(const Akonadi::Item &item,
                                              const Akonadi::Collection &root = Akonadi::Collection::root()) const;

    const AkonadiFakeStorageBehavior &storageBehavior() const;
    AkonadiFakeStorageBehavior &storageBehavior();

private:
    QHash<Akonadi::Collection::Id, Akonadi::Collection> m_collections;
    QHash<Akonadi::Collection::Id, QList<Akonadi::Collection::Id>> m_childCollections;

    using ContextUid = QString;
    QHash<ContextUid, Akonadi::Item> m_contexts;

    QHash<Akonadi::Item::Id, Akonadi::Item> m_items;
    QHash<Akonadi::Collection::Id, QList<Akonadi::Item::Id>> m_childItems;
    QHash<ContextUid, QList<Akonadi::Item::Id>> m_contextItems;

    QScopedPointer<AkonadiFakeMonitor> m_monitor;

    AkonadiFakeStorageBehavior m_storageBehavior;
};

}

#endif // TESTLIB_AKONADIFAKEDATA_H
