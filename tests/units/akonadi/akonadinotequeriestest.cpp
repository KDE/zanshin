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

#include <mockitopp/mockitopp.hpp>

#include "testlib/akonadimocks.h"

#include "akonadi/akonadinotequeries.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;

class AkonadiNoteQueriesTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldLookInAllReportedForAllNotes()
    {
        // GIVEN

        // one top level collections
        Akonadi::Collection col1(42);
        col1.setParentCollection(Akonadi::Collection::root());
        Akonadi::Collection col2(43);
        col2.setParentCollection(Akonadi::Collection::root());
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2);

        // One note in the first collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col1);
        Domain::Note::Ptr note1(new Domain::Note);
        MockItemFetchJob *itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << item1);

        // Two note in the second collection
        Akonadi::Item item2(43);
        item2.setParentCollection(col2);
        Akonadi::Item item3(44);
        item3.setParentCollection(col2);
        Domain::Note::Ptr note2(new Domain::Note);
        Domain::Note::Ptr note3(new Domain::Note);
        MockItemFetchJob *itemFetchJob2 = new MockItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << item2 << item3);


        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col1)
                                                           .thenReturn(itemFetchJob1);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col2)
                                                           .thenReturn(itemFetchJob2);

        // Serializer mock returning the notes from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item1).thenReturn(note1);
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item2).thenReturn(note2);
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item3).thenReturn(note3);

        // WHEN
        QScopedPointer<Domain::NoteQueries> queries(new Akonadi::NoteQueries(&storageMock.getInstance(),
                                                                             &serializerMock.getInstance(),
                                                                             new MockMonitor(this)));
        Domain::QueryResult<Domain::Note::Ptr>::Ptr result = queries->findAll();
        result->data();
        result = queries->findAll();  // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Notes)
                                                                         .exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item1).exactly(1));

        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item3).exactly(1));


        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0), note1);
        QCOMPARE(result->data().at(1), note2);
        QCOMPARE(result->data().at(2), note3);
    }

    void shouldIgnoreItemsWhichAreNotNotes()
    {
        // GIVEN

        // Two top level collections
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // Two notes in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        Domain::Note::Ptr note1(new Domain::Note);
        // One of them is not a note
        Akonadi::Item item2(43);
        item2.setParentCollection(col);
        Domain::Note::Ptr note2;
        MockItemFetchJob *itemFetchJob = new MockItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item1 << item2);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock returning the notes from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item1).thenReturn(note1);
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item2).thenReturn(note2);

        // WHEN
        QScopedPointer<Domain::NoteQueries> queries(new Akonadi::NoteQueries(&storageMock.getInstance(),
                                                                             &serializerMock.getInstance(),
                                                                             new MockMonitor(this)));
        Domain::QueryResult<Domain::Note::Ptr>::Ptr result = queries->findAll();

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Notes)
                                                                         .exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item2).exactly(1));

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0), note1);
    }

    void shouldReactToItemAddsForNotesOnly()
    {
        // GIVEN

        // Empty collection fetch
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);

        // Serializer mock returning the notes from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::NoteQueries> queries(new Akonadi::NoteQueries(&storageMock.getInstance(),
                                                                             &serializerMock.getInstance(),
                                                                             monitor));
        Domain::QueryResult<Domain::Note::Ptr>::Ptr result = queries->findAll();
        QTest::qWait(150);
        QVERIFY(result->data().isEmpty());

        // WHEN
        Akonadi::Item item1(42);
        Domain::Note::Ptr note1(new Domain::Note);
        Akonadi::Item item2(43);
        Domain::Note::Ptr note2;
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item1).thenReturn(note1);
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item2).thenReturn(note2);
        monitor->addItem(item1);
        monitor->addItem(item2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Notes)
                                                                         .exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item2).exactly(1));

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first(), note1);
    }

    void shouldReactToItemRemovesForAllNotes()
    {
        // GIVEN

        // One top level collections
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // Three note in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        Domain::Note::Ptr note1(new Domain::Note);
        Akonadi::Item item2(43);
        item2.setParentCollection(col);
        Domain::Note::Ptr note2(new Domain::Note);
        Akonadi::Item item3(44);
        item3.setParentCollection(col);
        Domain::Note::Ptr note3(new Domain::Note);
        MockItemFetchJob *itemFetchJob = new MockItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item1 << item2 << item3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock returning the notes from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item1).thenReturn(note1);
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item2).thenReturn(note2);
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item3).thenReturn(note3);

        serializerMock(&Akonadi::SerializerInterface::representsItem).when(note1, item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(note2, item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(note3, item2).thenReturn(false);

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::NoteQueries> queries(new Akonadi::NoteQueries(&storageMock.getInstance(),
                                                                             &serializerMock.getInstance(),
                                                                             monitor));
        Domain::QueryResult<Domain::Note::Ptr>::Ptr result = queries->findAll();
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 3);

        // WHEN
        monitor->removeItem(item2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Notes)
                                                                         .exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item3).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), note1);
        QCOMPARE(result->data().at(1), note3);
    }

    void shouldReactToItemChangesForAllNotes()
    {
        // GIVEN

        // One top level collections
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // Three note in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        Domain::Note::Ptr note1(new Domain::Note);
        Akonadi::Item item2(43);
        item2.setParentCollection(col);
        Domain::Note::Ptr note2(new Domain::Note);
        Akonadi::Item item3(44);
        item3.setParentCollection(col);
        Domain::Note::Ptr note3(new Domain::Note);
        MockItemFetchJob *itemFetchJob = new MockItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item1 << item2 << item3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock returning the notes from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item1).thenReturn(note1);
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item2).thenReturn(note2);
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item3).thenReturn(note3);
        serializerMock(&Akonadi::SerializerInterface::updateNoteFromItem).when(note2, item2).thenReturn();

        serializerMock(&Akonadi::SerializerInterface::representsItem).when(note1, item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(note2, item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(note3, item2).thenReturn(false);

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::NoteQueries> queries(new Akonadi::NoteQueries(&storageMock.getInstance(),
                                                                             &serializerMock.getInstance(),
                                                                             monitor));
        Domain::QueryResult<Domain::Note::Ptr>::Ptr result = queries->findAll();
        // Even though the pointer didn't change it's convenient to user if we call
        // the replace handlers
        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::Note::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 3);

        // WHEN
        monitor->changeItem(item2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Notes)
                                                                         .exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item3).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::updateNoteFromItem).when(note2, item2).exactly(1));

        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0), note1);
        QCOMPARE(result->data().at(1), note2);
        QCOMPARE(result->data().at(2), note3);
        QVERIFY(replaceHandlerCalled);
    }
};

QTEST_MAIN(AkonadiNoteQueriesTest)

#include "akonadinotequeriestest.moc"
