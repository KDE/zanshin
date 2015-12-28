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

#ifndef TESTLIB_GENCOLLECTION_H
#define TESTLIB_GENCOLLECTION_H

#include <QObject>

#include <AkonadiCore/Collection>

namespace Testlib {

class GenCollection
{
public:
    explicit GenCollection(const Akonadi::Collection &collection = Akonadi::Collection());

    operator Akonadi::Collection();

    GenCollection &withId(Akonadi::Collection::Id id);
    GenCollection &withParent(Akonadi::Collection::Id id);
    GenCollection &withRootAsParent();
    GenCollection &withName(const QString &name);
    GenCollection &withIcon(const QString &iconName);
    GenCollection &referenced(bool value = true);
    GenCollection &enabled(bool value = true);
    GenCollection &selected(bool value = true);
    GenCollection &withTaskContent(bool value = true);
    GenCollection &withNoteContent(bool value = true);

private:
    Akonadi::Collection m_collection;
};

}

#endif // TESTLIB_GENCOLLECTION_H
