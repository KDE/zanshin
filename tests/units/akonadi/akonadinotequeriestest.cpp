/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Remi Benoit <r3m1.benoit@gmail.com>

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

#include <QtTest>

#include "akonadi/akonadinotequeries.h"
#include "akonadi/akonadiserializer.h"

#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/gennote.h"
#include "testlib/testhelpers.h"

using namespace Testlib;

class AkonadiNoteQueriesTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldLookInAllReportedForAllNotes()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withNoteContent());

        // One note in the first collection
        data.createItem(GenNote().withId(42).withParent(42).withTitle("42"));

        // Two notes in the second collection
        data.createItem(GenNote().withId(43).withParent(43).withTitle("43"));
        data.createItem(GenNote().withId(44).withParent(43).withTitle("44"));

        // WHEN
        QScopedPointer<Domain::NoteQueries> queries(new Akonadi::NoteQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        result->data();
        result = queries->findAll(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("43"));
        QCOMPARE(result->data().at(2)->title(), QString("44"));
    }

    void shouldIgnoreItemsWhichAreNotNotes()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withNoteContent());

        // Two notes in the collection
        data.createItem(GenNote().withId(42).withParent(42).withTitle("42"));
        auto item = Akonadi::Item(43);
        item.setPayloadFromData("FooBar");
        item.setParentCollection(Akonadi::Collection(42));
        data.createItem(item);

        // WHEN
        QScopedPointer<Domain::NoteQueries> queries(new Akonadi::NoteQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
    }

    void shouldReactToItemAddsForNotesOnly()
    {
        // GIVEN
        AkonadiFakeData data;

        // One empty collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withNoteContent());

        QScopedPointer<Domain::NoteQueries> queries(new Akonadi::NoteQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createItem(GenNote().withId(42).withParent(42).withTitle("42"));
        auto item = Akonadi::Item(43);
        item.setPayloadFromData("FooBar");
        item.setParentCollection(Akonadi::Collection(42));
        data.createItem(item);

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first()->title(), QString("42"));
    }

    void shouldReactToItemRemovesForAllNotes()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withNoteContent());

        // Three notes in the collection
        data.createItem(GenNote().withId(42).withParent(42).withTitle("42"));
        data.createItem(GenNote().withId(43).withParent(42).withTitle("43"));
        data.createItem(GenNote().withId(44).withParent(42).withTitle("44"));

        QScopedPointer<Domain::NoteQueries> queries(new Akonadi::NoteQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 3);

        // WHEN
        data.removeItem(Akonadi::Item(43));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("44"));
    }

    void shouldReactToItemChangesForAllNotes()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withNoteContent());

        // Three Note in the collection
        data.createItem(GenNote().withId(42).withParent(42).withTitle("42"));
        data.createItem(GenNote().withId(43).withParent(42).withTitle("43"));
        data.createItem(GenNote().withId(44).withParent(42).withTitle("44"));

        QScopedPointer<Domain::NoteQueries> queries(new Akonadi::NoteQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        Domain::QueryResult<Domain::Note::Ptr>::Ptr result = queries->findAll();
        // Even though the pointer didn't change it's convenient to user if we call
        // the replace handlers
        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::Note::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 3);

        // WHEN
        data.modifyItem(GenNote(data.item(43)).withTitle("43bis"));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("43bis"));
        QCOMPARE(result->data().at(2)->title(), QString("44"));
        QVERIFY(replaceHandlerCalled);
    }
};

QTEST_MAIN(AkonadiNoteQueriesTest)

#include "akonadinotequeriestest.moc"
