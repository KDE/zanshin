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

#ifndef TESTLIB_GENTODO_H
#define TESTLIB_GENTODO_H

#include <QObject>

#include <AkonadiCore/Item>

namespace Testlib {

class GenTodo
{
public:
    explicit GenTodo(const Akonadi::Item &item = Akonadi::Item());

    operator Akonadi::Item();

    GenTodo &withId(Akonadi::Item::Id id);
    GenTodo &withParent(Akonadi::Collection::Id id);
    GenTodo &withTags(const QList<Akonadi::Tag::Id> &ids);
    GenTodo &asProject(bool value = true);
    GenTodo &withUid(const QString &uid);
    GenTodo &withParentUid(const QString &uid);
    GenTodo &withTitle(const QString &title);
    GenTodo &withText(const QString &text);
    GenTodo &done(bool value = true);
    GenTodo &withDoneDate(const QString &date);
    GenTodo &withDoneDate(const QDate &date);
    GenTodo &withStartDate(const QString &date);
    GenTodo &withStartDate(const QDate &date);
    GenTodo &withDueDate(const QString &date);
    GenTodo &withDueDate(const QDate &date);

private:
    Akonadi::Item m_item;
};

}

#endif // TESTLIB_GENTODO_H
