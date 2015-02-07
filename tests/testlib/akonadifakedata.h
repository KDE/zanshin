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

#ifndef TESTLIB_AKONADIFAKEDATA_H
#define TESTLIB_AKONADIFAKEDATA_H

#include <QObject>

#include <Akonadi/Collection>
#include <Akonadi/Item>

namespace Testlib {

class AkonadiFakeData
{
public:
    Akonadi::Collection::List collections() const;
    Akonadi::Collection::List childCollections(Akonadi::Collection::Id parentId) const;
    Akonadi::Collection collection(Akonadi::Collection::Id id) const;
    void createCollection(const Akonadi::Collection &collection);
    void modifyCollection(const Akonadi::Collection &collection);

    Akonadi::Item::List items() const;
    Akonadi::Item::List childItems(Akonadi::Collection::Id parentId) const;
    Akonadi::Item item(Akonadi::Item::Id id) const;
    void createItem(const Akonadi::Item &item);
    void modifyItem(const Akonadi::Item &item);

private:
    QHash<Akonadi::Collection::Id, Akonadi::Collection> m_collections;
    QHash<Akonadi::Collection::Id, QList<Akonadi::Collection::Id>> m_childCollections;

    QHash<Akonadi::Item::Id, Akonadi::Item> m_items;
    QHash<Akonadi::Collection::Id, QList<Akonadi::Item::Id>> m_childItems;
};

}

#endif // TESTLIB_AKONADIFAKEDATA_H
