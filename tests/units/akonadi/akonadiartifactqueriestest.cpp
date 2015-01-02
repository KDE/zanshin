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

#include "utils/mockobject.h"

#include "testlib/akonadifakejobs.h"
#include "testlib/akonadifakemonitor.h"

#include "domain/note.h"
#include "domain/task.h"

#include "akonadi/akonadiapplicationselectedattribute.h"
#include "akonadi/akonadiartifactqueries.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;

Q_DECLARE_METATYPE(Testlib::AkonadiFakeItemFetchJob*)
Q_DECLARE_METATYPE(Testlib::AkonadiFakeCollectionFetchJob*)

class AkonadiArtifactQueriesTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldLookInAllSelectedCollectionsForInboxArtifacts()
    {
        // GIVEN

        // Three top level collections
        Akonadi::Collection col1(42);
        col1.setParentCollection(Akonadi::Collection::root());
        Akonadi::Collection col2(43);
        col2.setParentCollection(Akonadi::Collection::root());
        Akonadi::Collection col3(44); // This one won't be selected
        col3.setParentCollection(Akonadi::Collection::root());
        Testlib::AkonadiFakeCollectionFetchJob *collectionFetchJob = new Testlib::AkonadiFakeCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2 << col3);

        // One note in the first collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col1);
        Domain::Note::Ptr note(new Domain::Note);
        Testlib::AkonadiFakeItemFetchJob *itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << item1);

        // Two tasks in the second collection
        Akonadi::Item item2(43);
        item2.setParentCollection(col2);
        Domain::Task::Ptr task1(new Domain::Task);
        Akonadi::Item item3(44);
        item3.setParentCollection(col2);
        Domain::Task::Ptr task2(new Domain::Task);
        Testlib::AkonadiFakeItemFetchJob *itemFetchJob2 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << item2 << item3);

        // Storage mock returning the fetch jobs
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks|Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col1)
                                                           .thenReturn(itemFetchJob1);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col2)
                                                           .thenReturn(itemFetchJob2);

        // Serializer mock returning the artifacts from the items
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::isSelectedCollection).when(col1).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isSelectedCollection).when(col2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isSelectedCollection).when(col3).thenReturn(false);

        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item1).thenReturn(Domain::Task::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item1).thenReturn(note);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).thenReturn(task1);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item3).thenReturn(task2);

        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item1).thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item3).thenReturn(QString());

        serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item3).thenReturn(false);

        serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item3).thenReturn(false);

        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item3).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item1).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item3).thenReturn(false);

        // WHEN
        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(storageMock.getInstance(),
                                                                                     serializerMock.getInstance(),
                                                                                     Testlib::AkonadiFakeMonitor::Ptr::create()));
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findInboxTopLevel();
        result->data();
        result = queries->findInboxTopLevel(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Tasks|Akonadi::StorageInterface::Notes)
                                                                         .exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col1).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item3).exactly(1));

        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0).dynamicCast<Domain::Note>(), note);
        QCOMPARE(result->data().at(1).dynamicCast<Domain::Task>(), task1);
        QCOMPARE(result->data().at(2).dynamicCast<Domain::Task>(), task2);
    }

    void shouldIgnoreItemsWhichAreNotTasksOrNotesInInbox()
    {
        // GIVEN

        // Two top level collections
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        Testlib::AkonadiFakeCollectionFetchJob *collectionFetchJob = new Testlib::AkonadiFakeCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // Two items in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        Domain::Task::Ptr task1(new Domain::Task);
        // One of them is not a task or a note
        Akonadi::Item item2(43);
        item2.setParentCollection(col);
        Domain::Task::Ptr task2;
        Domain::Note::Ptr note2;
        Testlib::AkonadiFakeItemFetchJob *itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item1 << item2);

        // Storage mock returning the fetch jobs
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks|Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock returning the tasks from the items
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::isSelectedCollection).when(col).thenReturn(true);

        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item1).thenReturn(task1);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).thenReturn(task2);
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item2).thenReturn(note2);

        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item1).thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).thenReturn(QString());

        serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item2).thenReturn(false);

        serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item2).thenReturn(false);

        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item1).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item2).thenReturn(false);

        // WHEN
        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(storageMock.getInstance(),
                                                                                     serializerMock.getInstance(),
                                                                                     Testlib::AkonadiFakeMonitor::Ptr::create()));
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findInboxTopLevel();

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Tasks|Akonadi::StorageInterface::Notes)
                                                                         .exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).exactly(0));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item2).exactly(0));

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0).dynamicCast<Domain::Task>(), task1);
    }

    void shouldNotHaveArtifactsWithParentsInInbox()
    {
        // TODO: Note that this specification is kind of an over simplification which
        // assumes that all the underlying data is correct. Ideally it should be checked
        // that the uid refered to actually points to a todo which exists in a proper
        // collection. We will need a cache to be able to implement that properly though.

        // GIVEN

        // One top level collection
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        Testlib::AkonadiFakeCollectionFetchJob *collectionFetchJob = new Testlib::AkonadiFakeCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // Three items in the collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col);
        Domain::Task::Ptr task1(new Domain::Task);
        // Two of them will have a parent
        Akonadi::Item item2(43);
        item2.setParentCollection(col);
        Domain::Task::Ptr task2(new Domain::Task);
        Akonadi::Item item3(44);
        item3.setParentCollection(col);
        Domain::Note::Ptr note3(new Domain::Note);
        Testlib::AkonadiFakeItemFetchJob *itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item1 << item2 << item3);

        // Storage mock returning the fetch jobs
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks|Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock returning the artifacts from the items
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::isSelectedCollection).when(col).thenReturn(true);

        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item1).thenReturn(task1);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).thenReturn(task2);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item3).thenReturn(Domain::Task::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item3).thenReturn(note3);

        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item1).thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).thenReturn("foo");
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item3).thenReturn("bar");

        serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item3).thenReturn(false);

        serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item3).thenReturn(false);

        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item1).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item3).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item3).thenReturn(true);

        // WHEN
        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(storageMock.getInstance(),
                                                                                     serializerMock.getInstance(),
                                                                                     Testlib::AkonadiFakeMonitor::Ptr::create()));
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findInboxTopLevel();

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Tasks|Akonadi::StorageInterface::Notes)
                                                                         .exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).exactly(0));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item3).exactly(0));

        QVERIFY(serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item3).exactly(1));

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0).dynamicCast<Domain::Task>(), task1);
    }

    void shouldNotHaveArtifactsWithContextsOrTagsInInbox_data()
    {
        QTest::addColumn<Domain::Artifact::Ptr>("artifact");
        QTest::addColumn<bool>("hasContexts");
        QTest::addColumn<bool>("hasTags");
        QTest::addColumn<bool>("isExpectedInInbox");

        Domain::Artifact::Ptr task(new Domain::Task);
        QTest::newRow("task with no tags") << task << false << false << true;
        QTest::newRow("task with tags") << task << false << true << false;
        QTest::newRow("task with contexts") << task << true << false << false;
        QTest::newRow("task with both") << task << true << true << false;

        Domain::Artifact::Ptr note(new Domain::Note);
        QTest::newRow("note with no tags") << note << false << false << true;
        QTest::newRow("note with tags") << note << false << true << false;
        QTest::newRow("note with contexts") << note << true << false << true;
        QTest::newRow("note with both") << note << true << true << false;
    }

    void shouldNotHaveArtifactsWithContextsOrTagsInInbox()
    {
        // GIVEN

        // One top level collection
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        Testlib::AkonadiFakeCollectionFetchJob *collectionFetchJob = new Testlib::AkonadiFakeCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // One item in the collection
        Akonadi::Item item(42);
        item.setParentCollection(col);
        QFETCH(Domain::Artifact::Ptr, artifact);
        QFETCH(bool, hasContexts);
        QFETCH(bool, hasTags);
        QFETCH(bool, isExpectedInInbox);
        Testlib::AkonadiFakeItemFetchJob *itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item);

        // Storage mock returning the fetch jobs
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks|Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock returning the artifact from the item
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::isSelectedCollection).when(col).thenReturn(true);

        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item).thenReturn(artifact.dynamicCast<Domain::Task>());
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item).thenReturn(artifact.dynamicCast<Domain::Note>());

        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item).thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item).thenReturn(hasContexts);
        serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item).thenReturn(hasTags);

        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item).thenReturn(!artifact.dynamicCast<Domain::Task>().isNull());
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item).thenReturn(!artifact.dynamicCast<Domain::Note>().isNull());

        // WHEN
        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(storageMock.getInstance(),
                                                                                     serializerMock.getInstance(),
                                                                                     Testlib::AkonadiFakeMonitor::Ptr::create()));
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findInboxTopLevel();

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);

        if (isExpectedInInbox) {
            QCOMPARE(result->data().size(), 1);
            QCOMPARE(result->data().at(0), artifact);
        } else {
            QVERIFY(result->data().isEmpty());
        }
    }

    void shouldReactToItemAddsForInbox_data()
    {
        QTest::addColumn<bool>("reactionExpected");
        QTest::addColumn<Domain::Artifact::Ptr>("artifact");
        QTest::addColumn<QString>("relatedUid");
        QTest::addColumn<bool>("hasContexts");
        QTest::addColumn<bool>("hasTags");

        Domain::Artifact::Ptr task(new Domain::Task);
        QTest::newRow("task which should be in inbox") << true << task << QString() << false << false;
        QTest::newRow("task with related uid") << false << task << "foo" << false << false;
        QTest::newRow("task with context") << false << task << QString() << true << false;
        QTest::newRow("task with tag") << false << task << QString() << false << true;

        Domain::Artifact::Ptr note(new Domain::Note);
        QTest::newRow("note which should be in inbox") << true << note << QString() << false << false;
        QTest::newRow("note with related uid") << false << note << "foo" << false << false;
        QTest::newRow("note with tag") << false << note << QString() << false << true;
        QTest::newRow("note with context") << true << note << QString() << true << false;
    }

    void shouldReactToItemAddsForInbox()
    {
        // GIVEN

        // One top level collection
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        Testlib::AkonadiFakeCollectionFetchJob *collectionFetchJob = new Testlib::AkonadiFakeCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // Storage mock returning the fetch jobs
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks|Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(new Testlib::AkonadiFakeItemFetchJob(this));

        // Serializer mock
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::isSelectedCollection).when(col).thenReturn(true);

        // Monitor mock
        auto monitor = Testlib::AkonadiFakeMonitor::Ptr::create();

        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(storageMock.getInstance(),
                                                                                     serializerMock.getInstance(),
                                                                                     monitor));
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findInboxTopLevel();
        QTest::qWait(150);
        QVERIFY(result->data().isEmpty());

        // WHEN
        QFETCH(bool, reactionExpected);
        QFETCH(Domain::Artifact::Ptr, artifact);
        QFETCH(QString, relatedUid);
        QFETCH(bool, hasContexts);
        QFETCH(bool, hasTags);

        Akonadi::Item item;

        // Serializer mock returning the artifact from the item
        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item).thenReturn(!artifact.dynamicCast<Domain::Task>().isNull());
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item).thenReturn(!artifact.dynamicCast<Domain::Note>().isNull());
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item).thenReturn(artifact.dynamicCast<Domain::Task>());
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item).thenReturn(artifact.dynamicCast<Domain::Note>());
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item).thenReturn(relatedUid);
        serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item).thenReturn(hasContexts);
        serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item).thenReturn(hasTags);

        monitor->addItem(item);

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item).atMost(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item).atMost(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item).atMost(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item).atMost(1));

        if (reactionExpected) {
            QCOMPARE(result->data().size(), 1);
            QCOMPARE(result->data().at(0), artifact);
        } else {
            QCOMPARE(result->data().size(), 0);
        }
    }

    void shouldReactToItemRemovesForInbox()
    {
        // GIVEN

        // One top level collection
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        Testlib::AkonadiFakeCollectionFetchJob *collectionFetchJob = new Testlib::AkonadiFakeCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // One item in the collection
        Akonadi::Item item(42);
        item.setParentCollection(col);
        Domain::Task::Ptr task(new Domain::Task);
        Testlib::AkonadiFakeItemFetchJob *itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item);

        // Storage mock returning the fetch jobs
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks|Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::isSelectedCollection).when(col).thenReturn(true);

        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item).thenReturn(task);
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item).thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(task, item).thenReturn(true);

        // Monitor mock
        auto monitor = Testlib::AkonadiFakeMonitor::Ptr::create();

        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(storageMock.getInstance(),
                                                                                     serializerMock.getInstance(),
                                                                                     monitor));
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findInboxTopLevel();
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first().dynamicCast<Domain::Task>(), task);

        // WHEN
        monitor->removeItem(item);

        // THEN
        QVERIFY(result->data().isEmpty());
    }

    void shouldReactToItemChangesForInbox_data()
    {
        QTest::addColumn<bool>("inListAfterChange");
        QTest::addColumn<Domain::Artifact::Ptr>("artifact");
        QTest::addColumn<QString>("relatedUidBefore");
        QTest::addColumn<QString>("relatedUidAfter");
        QTest::addColumn<bool>("hasContextsBefore");
        QTest::addColumn<bool>("hasContextsAfter");
        QTest::addColumn<bool>("hasTagsBefore");
        QTest::addColumn<bool>("hasTagsAfter");

        Domain::Artifact::Ptr task(new Domain::Task);
        QTest::newRow("task appears in inbox (related uid)") << true << task << "foo" << QString() << false << false << false << false;
        QTest::newRow("task disappears from inbox (related uid)") << false << task << QString() << "foo" << false << false << false << false;
        QTest::newRow("task appears in inbox (context)") << true << task << QString() << QString() << true << false << false << false;
        QTest::newRow("task disappears from inbox (context)") << false << task << QString() << QString() << false << true << false << false;

        Domain::Artifact::Ptr note(new Domain::Note);
        QTest::newRow("note appears in inbox (related uid)") << true << note << "foo" << QString() << false << false << false << false;
        QTest::newRow("note disappears from inbox (related uid)") << false << note << QString() << "foo" << false << false << false << false;
        QTest::newRow("note appears in inbox (tag)") << true << note << QString() << QString() << false << false << true << false;
        QTest::newRow("note disappears from inbox (tag)") << false << note << QString() << QString() << false << false << false << true;
    }

    void shouldReactToItemChangesForInbox()
    {
        // GIVEN

        // One top level collection
        Akonadi::Collection col(42);
        col.setParentCollection(Akonadi::Collection::root());
        Testlib::AkonadiFakeCollectionFetchJob *collectionFetchJob = new Testlib::AkonadiFakeCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col);

        // Artifact data
        QFETCH(bool, inListAfterChange);
        QFETCH(Domain::Artifact::Ptr, artifact);
        QFETCH(QString, relatedUidBefore);
        QFETCH(QString, relatedUidAfter);
        QFETCH(bool, hasContextsBefore);
        QFETCH(bool, hasContextsAfter);
        QFETCH(bool, hasTagsBefore);
        QFETCH(bool, hasTagsAfter);

        Akonadi::Item item;
        Testlib::AkonadiFakeItemFetchJob *itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item);

        // Storage mock returning the fetch jobs
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks|Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col)
                                                           .thenReturn(itemFetchJob);

        // Serializer mock
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::isSelectedCollection).when(col).thenReturn(true);

        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item).thenReturn(!artifact.dynamicCast<Domain::Task>().isNull());
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item).thenReturn(!artifact.dynamicCast<Domain::Note>().isNull());
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item).thenReturn(artifact.dynamicCast<Domain::Task>());
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item).thenReturn(artifact.dynamicCast<Domain::Note>());
        serializerMock(&Akonadi::SerializerInterface::updateTaskFromItem).when(artifact.dynamicCast<Domain::Task>(), item).thenReturn();
        serializerMock(&Akonadi::SerializerInterface::updateNoteFromItem).when(artifact.dynamicCast<Domain::Note>(), item).thenReturn();
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item).thenReturn(relatedUidBefore)
                                                                                    .thenReturn(relatedUidAfter);
        serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item).thenReturn(hasContextsBefore)
                                                                                .thenReturn(hasContextsAfter);
        serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item).thenReturn(hasTagsBefore)
                                                                              .thenReturn(hasTagsAfter);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(artifact, item).thenReturn(true);

        // Monitor mock
        auto monitor = Testlib::AkonadiFakeMonitor::Ptr::create();

        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(storageMock.getInstance(),
                                                                                     serializerMock.getInstance(),
                                                                                     monitor));
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findInboxTopLevel();
        QTest::qWait(150);
        if (inListAfterChange) {
            QVERIFY(result->data().isEmpty());
        } else {
            QCOMPARE(result->data().size(), 1);
        }

        // WHEN
        monitor->changeItem(item);

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item).atMost(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item).atMost(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::updateTaskFromItem).when(artifact.dynamicCast<Domain::Task>(), item).atMost(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::updateNoteFromItem).when(artifact.dynamicCast<Domain::Note>(), item).atMost(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item).atMost(2));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item).atMost(2));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item).atMost(2));

        if (inListAfterChange) {
            QCOMPARE(result->data().size(), 1);
        } else {
            QVERIFY(result->data().isEmpty());
        }
    }

    void shouldReactToCollectionSelectionChangesForInbox()
    {
        // GIVEN

        // Two top level collections
        Akonadi::Collection col1(42);
        col1.setParentCollection(Akonadi::Collection::root());
        Akonadi::Collection col2(43);
        col2.setParentCollection(Akonadi::Collection::root());
        Testlib::AkonadiFakeCollectionFetchJob *collectionFetchJob1 = new Testlib::AkonadiFakeCollectionFetchJob(this);
        collectionFetchJob1->setCollections(Akonadi::Collection::List() << col1 << col2);
        Testlib::AkonadiFakeCollectionFetchJob *collectionFetchJob2 = new Testlib::AkonadiFakeCollectionFetchJob(this);
        collectionFetchJob2->setCollections(Akonadi::Collection::List() << col1 << col2);

        // One item in each collection
        Akonadi::Item item1(42);
        item1.setParentCollection(col1);
        Domain::Task::Ptr task1(new Domain::Task);
        Testlib::AkonadiFakeItemFetchJob *itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << item1);
        Testlib::AkonadiFakeItemFetchJob *itemFetchJob2 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << item1);

        Akonadi::Item item2(43);
        item1.setParentCollection(col2);
        Domain::Task::Ptr task2(new Domain::Task);
        Testlib::AkonadiFakeItemFetchJob *itemFetchJob3 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob3->setItems(Akonadi::Item::List() << item2);


        // Storage mock returning the fetch jobs
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks|Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob1)
                                                                 .thenReturn(collectionFetchJob2);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col1)
                                                           .thenReturn(itemFetchJob1)
                                                           .thenReturn(itemFetchJob2);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(col2)
                                                           .thenReturn(itemFetchJob3);

        // Serializer mock
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::isSelectedCollection).when(col1).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isSelectedCollection).when(col2).thenReturn(true)
                                                                                      .thenReturn(false);

        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item1).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item1).thenReturn(task1);
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item1).thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item1).thenReturn(false);

        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).thenReturn(task2);
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(item2).thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::hasContextTags).when(item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::hasAkonadiTags).when(item2).thenReturn(false);

        serializerMock(&Akonadi::SerializerInterface::representsItem).when(task1, item1).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(task1, item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(task2, item1).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::representsItem).when(task2, item2).thenReturn(true);

        // Monitor mock
        auto monitor = Testlib::AkonadiFakeMonitor::Ptr::create();

        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(storageMock.getInstance(),
                                                                                     serializerMock.getInstance(),
                                                                                     monitor));
        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findInboxTopLevel();
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().first().dynamicCast<Domain::Task>(), task1);
        QCOMPARE(result->data().last().dynamicCast<Domain::Task>(), task2);

        // WHEN
        monitor->changeCollectionSelection(col2);
        QTest::qWait(150);

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first().dynamicCast<Domain::Task>(), task1);
    }
};

QTEST_MAIN(AkonadiArtifactQueriesTest)

#include "akonadiartifactqueriestest.moc"
