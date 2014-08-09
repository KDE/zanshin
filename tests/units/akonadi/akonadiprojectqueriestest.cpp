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
        result->data();
        result = queries->findAll(); // Should not cause any problem or wrong data

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
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).thenReturn(QString());
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
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(project1, item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(project2, item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(project3, item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).thenReturn(QString());

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
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(project1, item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(project2, item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(project3, item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).thenReturn(QString());

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

    void shouldLookInAllCollectionsForProjectTopLevelArtifacts()
    {
        // GIVEN

        // Two top level collections
        Akonadi::Collection col1(42);
        col1.setParentCollection(Akonadi::Collection::root());
        Akonadi::Collection col2(43);
        col2.setParentCollection(Akonadi::Collection::root());
        auto collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2);

        // One project and two tasks in the first collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col1);
        auto project1 = Domain::Project::Ptr::create();
        Akonadi::Item item2(43);
        item2.setParentCollection(col1);
        auto task2 = Domain::Task::Ptr::create();
        Akonadi::Item item3(44);
        item3.setParentCollection(col1);
        auto task3 = Domain::Task::Ptr::create();
        auto itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << item1 << item2 << item3);

        // Two notes in the second collection
        Akonadi::Item item4(45);
        item4.setParentCollection(col2);
        auto note4 = Domain::Note::Ptr::create();
        Akonadi::Item item5(46);
        item5.setParentCollection(col2);
        auto note5 = Domain::Note::Ptr::create();
        auto itemFetchJob2 = new MockItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << item4 << item5);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks | Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col1)
                                                           .thenReturn(itemFetchJob1);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col2)
                                                           .thenReturn(itemFetchJob2);

        // Serializer mock returning the objects from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::objectUid).when(project1).thenReturn("1");
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project1).thenReturn(item1);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).thenReturn(task2);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item3).thenReturn(task3);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item4).thenReturn(Domain::Task::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item5).thenReturn(Domain::Task::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item4).thenReturn(note4);
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item5).thenReturn(note5);

        // Serializer mock returning if project1 is parent of items
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item3).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item4).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item5).thenReturn(false);

        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item1).thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).thenReturn("1");
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item3).thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item4).thenReturn("1");
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item5).thenReturn(QString());

        // WHEN
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   new MockMonitor(this)));
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findTopLevelArtifacts(project1);
        result->data();
        result = queries->findTopLevelArtifacts(project1); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col1).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col2).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0).objectCast<Domain::Task>(), task2);
        QCOMPARE(result->data().at(1).objectCast<Domain::Note>(), note4);

        // Should not change nothing
        result = queries->findTopLevelArtifacts(project1);

        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col1).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col2).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0).objectCast<Domain::Task>(), task2);
        QCOMPARE(result->data().at(1).objectCast<Domain::Note>(), note4);
    }

    void shouldNotCrashWhenWeAskAgainTheSameTopLevelArtifacts()
    {
        // GIVEN

        // One top level collections
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());

        // One task in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        auto project1 = Domain::Project::Ptr::create();

        // We'll make the same queries twice
        auto collectionFetchJob11 = new MockCollectionFetchJob(this);
        collectionFetchJob11->setCollections(Akonadi::Collection::List() << col);
        auto itemFetchJob21 = new MockItemFetchJob(this);
        itemFetchJob21->setItems(Akonadi::Item::List() << item1);
        auto collectionFetchJob12 = new MockCollectionFetchJob(this);
        collectionFetchJob12->setCollections(Akonadi::Collection::List() << col);
        auto itemFetchJob22 = new MockItemFetchJob(this);
        itemFetchJob22->setItems(Akonadi::Item::List() << item1);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks | Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob11)
                                                                 .thenReturn(collectionFetchJob12);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob21)
                                                           .thenReturn(itemFetchJob22);

        // Serializer mock
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::objectUid).when(project1).thenReturn("1");
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project1).thenReturn(item1);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item1).thenReturn(false);

        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   new MockMonitor(this)));

        // The bug we're trying to hit here is the following:
        //  - when findChildren is called the first time a provider is created internally
        //  - result is deleted at the end of the loop, no one holds the provider with
        //    a strong reference anymore so it is deleted as well
        //  - when findChildren is called the second time, there's a risk of a dangling
        //    pointer if the recycling of providers is wrongly implemented which can lead
        //    to a crash, if it is properly done no crash will occur
        for (int i = 0; i < 2; i++) {
            // WHEN * 2
            Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findTopLevelArtifacts(project1);

            // THEN * 2
            QVERIFY(result->data().isEmpty());
            QTest::qWait(150);
            QVERIFY(result->data().isEmpty());
        }
    }

    void shouldReactToItemAddsForTopLevelArtifact()
    {
        // GIVEN

        // One top level collection
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        auto collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // One project in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        auto project1 = Domain::Project::Ptr::create();
        auto itemFetchJob = new MockItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item1);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks | Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::objectUid).when(project1).thenReturn("1");
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project1).thenReturn(item1);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item1).thenReturn(false);

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   monitor));
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findTopLevelArtifacts(project1);
        QTest::qWait(150);
        QVERIFY(result->data().isEmpty());
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item1).exactly(1));

        // WHEN
        Akonadi::Item item2(43);
        item2.setParentCollection(col);
        auto task2 = Domain::Task::Ptr::create();
        Akonadi::Item item3(44);
        item3.setParentCollection(col);
        auto note3 = Domain::Note::Ptr::create();
        serializerMock(&Akonadi::SerializerInterface::objectUid).when(project1).thenReturn("1");
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item2).thenReturn(Domain::Project::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item3).thenReturn(Domain::Project::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).thenReturn(task2);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item3).thenReturn(Domain::Task::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item3).thenReturn(note3);
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).thenReturn("1");
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item3).thenReturn("1");

        monitor->addItem(item2);
        monitor->addItem(item3);

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item3).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).exactly(2));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item3).exactly(2));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data()[0].objectCast<Domain::Task>(), task2);
        QCOMPARE(result->data()[1].objectCast<Domain::Note>(), note3);
    }

    void shouldReactToItemChangesForTopLevelArtifacts()
    {
        // GIVEN

        // One top level collection
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        auto collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // One project and two tasks in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        auto project1 = Domain::Project::Ptr::create();
        Akonadi::Item item2(43);
        item2.setParentCollection(col);
        auto task2 = Domain::Task::Ptr::create();
        Akonadi::Item item3(44);
        item3.setParentCollection(col);
        auto note3 = Domain::Note::Ptr::create();
        auto itemFetchJob = new MockItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item1 << item2 << item3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks | Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock returning the tasks from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::objectUid).when(project1).thenReturn("1");
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project1).thenReturn(item1);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item2).thenReturn(Domain::Project::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).thenReturn(task2);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item3).thenReturn(Domain::Project::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item3).thenReturn(Domain::Task::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item3).thenReturn(note3);

        serializerMock(&Akonadi::SerializerInterface::representsItem).when(task2, item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(note3, item2).thenReturn(false);

        // Serializer mock returning if the item has a relatedItem
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).thenReturn("1");
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item3).thenReturn("1");

        // Serializer mock returning if task1 is parent of items
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item3).thenReturn(true);

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   monitor));
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findTopLevelArtifacts(project1);

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::Artifact::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 2);

        // WHEN
        serializerMock(&Akonadi::SerializerInterface::updateTaskFromItem).when(task2, item2).thenReturn();
        monitor->changeItem(item2);

        // Then
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0).objectCast<Domain::Task>(), task2);
        QCOMPARE(result->data().at(1).objectCast<Domain::Note>(), note3);

        QVERIFY(replaceHandlerCalled);
    }

    void shouldRemoveItemFromCorrespondingResultWhenRelatedItemChangesForTopLevelArtifact()
    {
        // GIVEN

        // One top level collection
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        auto collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // One project and two tasks in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        auto project1 = Domain::Project::Ptr::create();
        Akonadi::Item item2(43);
        item2.setParentCollection(col);
        auto task2 = Domain::Task::Ptr::create();
        Akonadi::Item item3(44);
        item3.setParentCollection(col);
        auto note3 = Domain::Note::Ptr::create();
        auto itemFetchJob = new MockItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item1 << item2 << item3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks | Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock returning the tasks from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::objectUid).when(project1).thenReturn("1");
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project1).thenReturn(item1);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item2).thenReturn(Domain::Project::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).thenReturn(task2);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item3).thenReturn(Domain::Project::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item3).thenReturn(Domain::Task::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item3).thenReturn(note3);

        serializerMock(&Akonadi::SerializerInterface::representsItem).when(task2, item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(note3, item2).thenReturn(false);

        // Serializer mock returning if the item has a relatedItem
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).thenReturn("1")
                                                                                     .thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item3).thenReturn("1");

        // Serializer mock returning if task1 is parent of items
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item3).thenReturn(true);

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   monitor));
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findTopLevelArtifacts(project1);

        QTest::qWait(150);
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0).objectCast<Domain::Task>(), task2);
        QCOMPARE(result->data().at(1).objectCast<Domain::Note>(), note3);

        // WHEN
        serializerMock(&Akonadi::SerializerInterface::updateTaskFromItem).when(task2, item2).thenReturn();
        monitor->changeItem(item2);

        // Then
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col).exactly(1));

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0).objectCast<Domain::Note>(), note3);
    }

    void shouldAddItemToCorrespondingResultWhenRelatedItemChangeForChildrenTask()
    {
        // GIVEN

        // One top level collection
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        auto collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // One project and two tasks in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        auto project1 = Domain::Project::Ptr::create();
        Akonadi::Item item2(43);
        item2.setParentCollection(col);
        auto task2 = Domain::Task::Ptr::create();
        Akonadi::Item item3(44);
        item3.setParentCollection(col);
        auto note3 = Domain::Note::Ptr::create();
        auto itemFetchJob = new MockItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item1 << item2 << item3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks | Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock returning the tasks from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::objectUid).when(project1).thenReturn("1");
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project1).thenReturn(item1);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item2).thenReturn(Domain::Project::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).thenReturn(task2);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item3).thenReturn(Domain::Project::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item3).thenReturn(Domain::Task::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item3).thenReturn(note3);

        serializerMock(&Akonadi::SerializerInterface::representsItem).when(task2, item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(note3, item2).thenReturn(false);

        // Serializer mock returning if the item has a relatedItem
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item3).thenReturn("1");

        // Serializer mock returning if task1 is parent of items
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item3).thenReturn(true);

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   monitor));
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findTopLevelArtifacts(project1);

        QTest::qWait(150);
        QCOMPARE(result->data().size(), 1);

        // WHEN
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).thenReturn("1");
        monitor->changeItem(item2);

        // Then
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0).objectCast<Domain::Note>(), note3);
        QCOMPARE(result->data().at(1).objectCast<Domain::Task>(), task2);
    }

    void shouldMoveItemToCorrespondingResultWhenRelatedItemChangeForTopLevelArtifacts()
    {
        // GIVEN

        // One top level collection
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());

        // Two projects and one task in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        auto project1 = Domain::Project::Ptr::create();
        Akonadi::Item item2(43);
        item2.setParentCollection(col);
        auto project2 = Domain::Project::Ptr::create();
        Akonadi::Item item3(44);
        item3.setParentCollection(col);
        auto task3 = Domain::Task::Ptr::create();

        // Jobs
        auto collectionFetchJob1 = new MockCollectionFetchJob(this);
        collectionFetchJob1->setCollections(Akonadi::Collection::List() << col);
        auto itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << item1 << item2 << item3);

        auto collectionFetchJob2 = new MockCollectionFetchJob(this);
        collectionFetchJob2->setCollections(Akonadi::Collection::List() << col);
        auto itemFetchJob2 = new MockItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << item1 << item2 << item3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks | Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob1)
                                                                 .thenReturn(collectionFetchJob2);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob1)
                                                           .thenReturn(itemFetchJob2);

        // Serializer mock returning the objects from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::objectUid).when(project1).thenReturn("1");
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project1).thenReturn(item1);

        serializerMock(&Akonadi::SerializerInterface::objectUid).when(project2).thenReturn("2");
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project2).thenReturn(item2);

        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item3).thenReturn(Domain::Project::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item3).thenReturn(task3);

        serializerMock(&Akonadi::SerializerInterface::representsItem).when(task3, item3).thenReturn(true);

        // Serializer mock returning if the item has a relatedItem
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item1).thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item3).thenReturn("1")
                                                                                     .thenReturn("2");

        // Serializer mock returning if project1 is parent of items
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item3).thenReturn(true)
                                                                                           .thenReturn(false);

        // Serializer mock returning if project2 is parent of items
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project2, item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project2, item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project2, item3).thenReturn(false)
                                                                                           .thenReturn(true);

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   monitor));
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result1 = queries->findTopLevelArtifacts(project1);
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result2 = queries->findTopLevelArtifacts(project2);

        QTest::qWait(150);
        QCOMPARE(result1->data().size(), 1);
        QCOMPARE(result1->data().at(0).objectCast<Domain::Task>(), task3);
        QCOMPARE(result2->data().size(), 0);

        // WHEN
        monitor->changeItem(item3); // Now gets a different related id

        // Then
        QCOMPARE(result1->data().size(), 0);
        QCOMPARE(result2->data().size(), 1);
        QCOMPARE(result2->data().at(0).objectCast<Domain::Task>(), task3);
    }

    void shouldReactToItemRemovesForTopLevelArtifacts()
    {
        // GIVEN

        // One top level collection
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        auto collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // One project and two tasks in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        auto project1 = Domain::Project::Ptr::create();
        Akonadi::Item item2(43);
        item2.setParentCollection(col);
        auto task2 = Domain::Task::Ptr::create();
        Akonadi::Item item3(44);
        item3.setParentCollection(col);
        auto note3 = Domain::Note::Ptr::create();
        auto itemFetchJob = new MockItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item1 << item2 << item3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks | Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock returning the tasks from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::objectUid).when(project1).thenReturn("1");
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project1).thenReturn(item1);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item2).thenReturn(Domain::Project::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).thenReturn(task2);
        serializerMock(&Akonadi::SerializerInterface::createProjectFromItem).when(item3).thenReturn(Domain::Project::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item3).thenReturn(Domain::Task::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item3).thenReturn(note3);

        serializerMock(&Akonadi::SerializerInterface::representsItem).when(task2, item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(note3, item2).thenReturn(false);

        // Serializer mock returning if the item has a relatedItem
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).thenReturn("1");
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item3).thenReturn("1");

        // Serializer mock returning if task1 is parent of items
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isProjectChild).when(project1, item3).thenReturn(true);

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   monitor));
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findTopLevelArtifacts(project1);

        QTest::qWait(150);
        QCOMPARE(result->data().size(), 2);

        // WHEN
        monitor->removeItem(item2);

        // Then
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col).exactly(1));

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0).objectCast<Domain::Note>(), note3);
    }
};

QTEST_MAIN(AkonadiProjectQueriesTest)

#include "akonadiprojectqueriestest.moc"
