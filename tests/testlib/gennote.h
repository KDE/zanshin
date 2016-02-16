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

#ifndef TESTLIB_GENNOTE_H
#define TESTLIB_GENNOTE_H

#include <QObject>

#include <AkonadiCore/Item>

namespace Testlib {

class GenNote
{
public:
    explicit GenNote(const Akonadi::Item &item = Akonadi::Item());

    operator Akonadi::Item();

    GenNote &withId(Akonadi::Item::Id id);
    GenNote &withParent(Akonadi::Collection::Id id);
    GenNote &withTags(const QList<Akonadi::Tag::Id> &ids);
    GenNote &withParentUid(const QString &uid);
    GenNote &withTitle(const QString &title);
    GenNote &withText(const QString &text);

private:
    Akonadi::Item m_item;
};

}

#endif // TESTLIB_GENNOTE_H
