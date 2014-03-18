/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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

#include "akonadi/akonaditaskqueries.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;

class AkonadiTaskQueriesTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldLookInAllReportedForAllTasks()
    {
        // GIVEN

        // Two top level collections
        Akonadi::Collection col1(42);
        col1.setParentCollection(Akonadi::Collection::root());
        Akonadi::Collection col2(43);
        col1.setParentCollection(Akonadi::Collection::root());
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2);

        // One task in the first collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col1);
        Domain::Task::Ptr task1(new Domain::Task);
        MockItemFetchJob *itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << item1);

        // Two tasks in the second collection
        Akonadi::Item item2(43);
        item2.setParentCollection(col2);
        Domain::Task::Ptr task2(new Domain::Task);
        Akonadi::Item item3(44);
        item3.setParentCollection(col2);
        Domain::Task::Ptr task3(new Domain::Task);
        MockItemFetchJob *itemFetchJob2 = new MockItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << item2 << item3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col1)
                                                           .thenReturn(itemFetchJob1);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col2)
                                                           .thenReturn(itemFetchJob2);

        // Serializer mock returning the tasks from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::deserializeTask).when(item1).thenReturn(task1);
        serializerMock(&Akonadi::SerializerInterface::deserializeTask).when(item2).thenReturn(task2);
        serializerMock(&Akonadi::SerializerInterface::deserializeTask).when(item3).thenReturn(task3);

        // WHEN
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(&storageMock.getInstance(),
                                                                             &serializerMock.getInstance(),
                                                                             new MockMonitor(this)));
        Domain::QueryResult<Domain::Task::Ptr>::Ptr result = queries->findAll();

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive)
                                                                         .exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col1).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::deserializeTask).when(item1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::deserializeTask).when(item2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::deserializeTask).when(item3).exactly(1));

        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0), task1);
        QCOMPARE(result->data().at(1), task2);
        QCOMPARE(result->data().at(2), task3);
    }

    void shouldReactToItemAddsForAllTasks()
    {
        // GIVEN

        // Empty collection fetch
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive)
                                                                 .thenReturn(collectionFetchJob);

        // Serializer mock returning the tasks from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(&storageMock.getInstance(),
                                                                             &serializerMock.getInstance(),
                                                                             monitor));
        Domain::QueryResult<Domain::Task::Ptr>::Ptr result = queries->findAll();
        QTest::qWait(150);
        QVERIFY(result->data().isEmpty());

        // WHEN
        Akonadi::Item item(42);
        Domain::Task::Ptr task(new Domain::Task);
        serializerMock(&Akonadi::SerializerInterface::deserializeTask).when(item).thenReturn(task);
        monitor->addItem(item);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive)
                                                                         .exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::deserializeTask).when(item).exactly(1));

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first(), task);
    }

    void shouldReactToItemRemovesForAllTasks()
    {
        // GIVEN

        // One top level collections
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // Three task in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        Domain::Task::Ptr task1(new Domain::Task);
        Akonadi::Item item2(43);
        item2.setParentCollection(col);
        Domain::Task::Ptr task2(new Domain::Task);
        Akonadi::Item item3(44);
        item3.setParentCollection(col);
        Domain::Task::Ptr task3(new Domain::Task);
        MockItemFetchJob *itemFetchJob = new MockItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item1 << item2 << item3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock returning the tasks from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::deserializeTask).when(item1).thenReturn(task1);
        serializerMock(&Akonadi::SerializerInterface::deserializeTask).when(item2).thenReturn(task2);
        serializerMock(&Akonadi::SerializerInterface::deserializeTask).when(item3).thenReturn(task3);

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(&storageMock.getInstance(),
                                                                             &serializerMock.getInstance(),
                                                                             monitor));
        Domain::QueryResult<Domain::Task::Ptr>::Ptr result = queries->findAll();
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 3);

        // WHEN
        monitor->removeItem(item2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive)
                                                                         .exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::deserializeTask).when(item1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::deserializeTask).when(item2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::deserializeTask).when(item3).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), task1);
        QCOMPARE(result->data().at(1), task3);
    }
};

QTEST_MAIN(AkonadiTaskQueriesTest)

#include "akonaditaskqueriestest.moc"
