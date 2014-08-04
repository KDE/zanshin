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

#include "akonadi/akonadiprojectqueries.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;

Q_DECLARE_METATYPE(MockItemFetchJob*)
Q_DECLARE_METATYPE(MockCollectionFetchJob*)

class AkonadiProjectQueriesTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldLookInAllReportedForAllProjects()
    {
        // GIVEN

        // Two top level collections
        Akonadi::Collection col1(42);
        col1.setParentCollection(Akonadi::Collection::root());
        Akonadi::Collection col2(43);
        col2.setParentCollection(Akonadi::Collection::root());
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2);

        // One project in the first collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col1);
        Domain::Project::Ptr project1(new Domain::Project);
        MockItemFetchJob *itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << item1);

        // Two projects in the second collection
        Akonadi::Item item2(43);
        item2.setParentCollection(col2);
        Domain::Project::Ptr project2(new Domain::Project);
        Akonadi::Item item3(44);
        item3.setParentCollection(col2);
        Domain::Project::Ptr project3(new Domain::Project);
        MockItemFetchJob *itemFetchJob2 = new MockItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << item2 << item3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col1)
                                                           .thenReturn(itemFetchJob1);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col2)
                                                           .thenReturn(itemFetchJob2);

        // Serializer mock returning the projects from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item1).thenReturn(project1);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item2).thenReturn(project2);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item3).thenReturn(project3);

        // Serializer mock returning if the item has a relatedItem
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item1).thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item3).thenReturn(QString());

        // WHEN
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   new MockMonitor(this)));
        Domain::QueryResult<Domain::Project::Ptr>::Ptr result = queries->findAll();

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Tasks)
                                                                         .exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col1).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item3).exactly(1));

        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0), project1);
        QCOMPARE(result->data().at(1), project2);
        QCOMPARE(result->data().at(2), project3);
    }

    void shouldIgnoreItemsWhichAreNotProjects()
    {
        // GIVEN

        // Two top level collections
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // Two projects in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        Domain::Project::Ptr project1(new Domain::Project);
        // One of them is not a project
        Akonadi::Item item2(43);
        item2.setParentCollection(col);
        Domain::Project::Ptr project2;
        MockItemFetchJob *itemFetchJob = new MockItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item1 << item2);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock returning the projects from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item1).thenReturn(project1);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item2).thenReturn(project2);

        // Serializer mock returning if the item has a relatedItem
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item1).thenReturn(QString());

        // WHEN
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   new MockMonitor(this)));
        Domain::QueryResult<Domain::Project::Ptr>::Ptr result = queries->findAll();

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Tasks)
                                                                         .exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item2).exactly(1));

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0), project1);
    }

    void shouldReactToItemAddsForProjectsOnly()
    {
        // GIVEN

        // Empty collection fetch
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks)
                                                                 .thenReturn(collectionFetchJob);

        // Serializer mock returning the projects from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   monitor));
        Domain::QueryResult<Domain::Project::Ptr>::Ptr result = queries->findAll();
        QTest::qWait(150);
        QVERIFY(result->data().isEmpty());

        // WHEN
        Akonadi::Item item1(42);
        Domain::Project::Ptr project1(new Domain::Project);
        Akonadi::Item item2(43);
        Domain::Project::Ptr project2;
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item1).thenReturn(project1);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item2).thenReturn(project2);
        monitor->addItem(item1);
        monitor->addItem(item2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Tasks)
                                                                         .exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item2).exactly(1));

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first(), project1);
    }

    void shouldReactToItemRemovesForAllProjects()
    {
        // GIVEN

        // One top level collections
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // Three projects in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        Domain::Project::Ptr project1(new Domain::Project);
        Akonadi::Item item2(43);
        item2.setParentCollection(col);
        Domain::Project::Ptr project2(new Domain::Project);
        Akonadi::Item item3(44);
        item3.setParentCollection(col);
        Domain::Project::Ptr project3(new Domain::Project);
        MockItemFetchJob *itemFetchJob = new MockItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item1 << item2 << item3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock returning the projects from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item1).thenReturn(project1);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item2).thenReturn(project2);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item3).thenReturn(project3);
        serializerMock(&Akonadi::SerializerInterface::represents).when(project1, item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::represents).when(project2, item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::represents).when(project3, item2).thenReturn(false);

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   monitor));
        Domain::QueryResult<Domain::Project::Ptr>::Ptr result = queries->findAll();
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 3);

        // WHEN
        monitor->removeItem(item2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Tasks)
                                                                         .exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item3).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), project1);
        QCOMPARE(result->data().at(1), project3);
    }

    void shouldReactToItemChangesForAllProjects()
    {
        // GIVEN

        // One top level collections
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // Three project in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        Domain::Project::Ptr project1(new Domain::Project);
        Akonadi::Item item2(43);
        item2.setParentCollection(col);
        Domain::Project::Ptr project2(new Domain::Project);
        Akonadi::Item item3(44);
        item3.setParentCollection(col);
        Domain::Project::Ptr project3(new Domain::Project);
        MockItemFetchJob *itemFetchJob = new MockItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item1 << item2 << item3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock returning the projects from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item1).thenReturn(project1);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item2).thenReturn(project2);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item3).thenReturn(project3);
        serializerMock(&Akonadi::SerializerInterface::updateProjectFromItem).when(project2, item2).thenReturn();
        serializerMock(&Akonadi::SerializerInterface::represents).when(project1, item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::represents).when(project2, item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::represents).when(project3, item2).thenReturn(false);

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   monitor));
        Domain::QueryResult<Domain::Project::Ptr>::Ptr result = queries->findAll();
        // Even though the pointer didn't change it's convenient to user if we call
        // the replace handlers
        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::Project::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 3);

        // WHEN
        monitor->changeItem(item2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Tasks)
                                                                         .exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item3).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::updateProjectFromItem).when(project2, item2).exactly(1));

        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0), project1);
        QCOMPARE(result->data().at(1), project2);
        QCOMPARE(result->data().at(2), project3);
        QVERIFY(replaceHandlerCalled);
    }
};

QTEST_MAIN(AkonadiProjectQueriesTest)

#include "akonadiprojectqueriestest.moc"
