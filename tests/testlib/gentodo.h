/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef TESTLIB_GENTODO_H
#define TESTLIB_GENTODO_H

#include <QObject>

#include <AkonadiCore/Akonadi/Item>

namespace Testlib {

class GenTodo
{
public:
    explicit GenTodo(const Akonadi::Item &item = Akonadi::Item());

    operator Akonadi::Item();

    GenTodo &withId(Akonadi::Item::Id id);
    GenTodo &withParent(Akonadi::Collection::Id id);
    GenTodo &withContexts(const QStringList &contextUids);
    GenTodo &asProject(bool value = true);
    GenTodo &asContext(bool value = true);
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
