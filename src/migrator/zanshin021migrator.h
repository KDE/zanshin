/* This file is part of Zanshin

   Copyright 2014 David Faure <faure@kde.org>

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

#include <Akonadi/Item>
#include <akonadi/akonadistorage.h>

namespace Akonadi {
    class TransactionSequence;
}

class SeenItem
{
public:
    SeenItem(const Akonadi::Item &theItem)
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

