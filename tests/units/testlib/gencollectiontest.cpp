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

#include "testlib/gencollection.h"

#include "akonadi/akonadiapplicationselectedattribute.h"
#include <AkonadiCore/EntityDisplayAttribute>

#include <testlib/qtest_zanshin.h>

using namespace Testlib;

class GenCollectionTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldImplicitlyConvertBackToCollection()
    {
        // GIVEN
        auto col = Akonadi::Collection(42);
        auto gen = GenCollection(col);

        // WHEN
        Akonadi::Collection newCol = gen;

        // THEN
        QCOMPARE(newCol, col);
    }

    void shouldAllowToSetId()
    {
        // GIVEN
        Akonadi::Collection col = GenCollection().withId(42);

        // THEN
        QCOMPARE(col.id(), 42LL);
    }

    void shouldAllowToSetParent()
    {
        // GIVEN
        Akonadi::Collection col = GenCollection().withParent(42);

        // THEN
        QCOMPARE(col.parentCollection().id(), 42LL);
    }

    void shouldAllowToSetRootAsParent()
    {
        // GIVEN
        Akonadi::Collection col = GenCollection().withRootAsParent();

        // THEN
        QCOMPARE(col.parentCollection(), Akonadi::Collection::root());
    }

    void shouldAllowToSetName()
    {
        // GIVEN
        Akonadi::Collection col = GenCollection().withName(QStringLiteral("42"));

        // THEN
        QCOMPARE(col.name(), QStringLiteral("42"));
    }

    void shouldAllowToSetIcon()
    {
        // GIVEN
        Akonadi::Collection col = GenCollection().withIcon(QStringLiteral("42"));

        // THEN
        QCOMPARE(col.attribute<Akonadi::EntityDisplayAttribute>()->iconName(), QStringLiteral("42"));
    }

    void shouldAllowToSetReferenced()
    {
        // GIVEN
        Akonadi::Collection col = GenCollection().referenced();

        // THEN
        QCOMPARE(col.referenced(), true);

        // WHEN
        col = GenCollection(col).referenced(false);

        // THEN
        QCOMPARE(col.referenced(), false);
    }

    void shouldAllowToSetEnabled()
    {
        // GIVEN
        Akonadi::Collection col = GenCollection().enabled();

        // THEN
        QCOMPARE(col.enabled(), true);

        // WHEN
        col = GenCollection(col).enabled(false);

        // THEN
        QCOMPARE(col.enabled(), false);
    }

    void shouldAllowToSetSelected()
    {
        // GIVEN
        Akonadi::Collection col = GenCollection().selected();

        // THEN
        QVERIFY(!col.hasAttribute<Akonadi::ApplicationSelectedAttribute>());

        // WHEN
        col = GenCollection(col).selected(false);

        // THEN
        QCOMPARE(col.attribute<Akonadi::ApplicationSelectedAttribute>()->isSelected(), false);
    }

    void shouldAllowToSetTaskContent()
    {
        // GIVEN
        Akonadi::Collection col = GenCollection().withTaskContent();

        // THEN
        QVERIFY(col.contentMimeTypes().contains(QStringLiteral("application/x-vnd.akonadi.calendar.todo")));

        // WHEN
        col = GenCollection(col).withTaskContent(false);

        // THEN
        QVERIFY(!col.contentMimeTypes().contains(QStringLiteral("application/x-vnd.akonadi.calendar.todo")));
    }

    void shouldAllowToSetNoteContent()
    {
        // GIVEN
        Akonadi::Collection col = GenCollection().withNoteContent();

        // THEN
        QVERIFY(col.contentMimeTypes().contains(QStringLiteral("text/x-vnd.akonadi.note")));

        // WHEN
        col = GenCollection(col).withNoteContent(false);

        // THEN
        QVERIFY(!col.contentMimeTypes().contains(QStringLiteral("text/x-vnd.akonadi.note")));
    }

    void shouldAllowToSetNoteAndTaskContent()
    {
        // GIVEN
        Akonadi::Collection col = GenCollection().withTaskContent().withNoteContent();

        // THEN
        QVERIFY(col.contentMimeTypes().contains(QStringLiteral("application/x-vnd.akonadi.calendar.todo")));
        QVERIFY(col.contentMimeTypes().contains(QStringLiteral("text/x-vnd.akonadi.note")));

        // WHEN
        col = GenCollection(col).withTaskContent(false).withNoteContent(false);

        // THEN
        QVERIFY(!col.contentMimeTypes().contains(QStringLiteral("application/x-vnd.akonadi.calendar.todo")));
        QVERIFY(!col.contentMimeTypes().contains(QStringLiteral("text/x-vnd.akonadi.note")));
    }
};

ZANSHIN_TEST_MAIN(GenCollectionTest)

#include "gencollectiontest.moc"
