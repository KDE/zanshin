/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include "testlib/gencollection.h"

#include "akonadi/akonadiapplicationselectedattribute.h"
#include <Akonadi/EntityDisplayAttribute>

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
