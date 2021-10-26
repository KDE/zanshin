/*
 * SPDX-FileCopyrightText: 2014 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef ZANSHIN021MIGRATOR_H
#define ZANSHIN021MIGRATOR_H

#include <AkonadiCore/Akonadi/Item>
#include <akonadi/akonadistorage.h>

namespace Akonadi {
    class TransactionSequence;
}

class SeenItem
{
public:
    explicit SeenItem(const Akonadi::Item &theItem)
        : m_item(theItem), m_dirty(false)
    {
    }
    // invalid item, for QHash::value
    SeenItem()
        : m_item(Akonadi::Item()), m_dirty(false)
    {
    }
    bool isDirty() const { return m_dirty; }
    void setDirty() { m_dirty = true; }

    Akonadi::Item &item() { return m_item; }
    const Akonadi::Item &item() const { return m_item; }

private:
    Akonadi::Item m_item;
    bool m_dirty;
};

class Zanshin021Migrator
{
public:
    Zanshin021Migrator();

    typedef QHash<QString /*uid*/, SeenItem> SeenItemHash;
    SeenItemHash fetchAllItems();

    void migrateProjectComments(Zanshin021Migrator::SeenItemHash& items, Akonadi::TransactionSequence* sequence);

    void migrateProjectWithChildren(Zanshin021Migrator::SeenItemHash& items, Akonadi::TransactionSequence* sequence);

    bool migrateProjects();

    // returns true if item is a "new style" project
    static bool isProject(const Akonadi::Item &item);

private:
    void markAsProject(SeenItem &seenItem, Akonadi::TransactionSequence* sequence);

    Akonadi::Storage m_storage;

};

#endif // ZANSHIN021MIGRATOR_H
