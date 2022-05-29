/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef TESTLIB_GENCOLLECTION_H
#define TESTLIB_GENCOLLECTION_H

#include <QObject>

#include <Akonadi/Collection>

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
    GenCollection &selected(bool value = true);
    GenCollection &withTaskContent(bool value = true);
    GenCollection &withNoteContent(bool value = true);

private:
    Akonadi::Collection m_collection;
};

}

#endif // TESTLIB_GENCOLLECTION_H
