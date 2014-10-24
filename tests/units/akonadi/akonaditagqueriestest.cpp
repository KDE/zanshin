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

#include "akonadi/akonaditagqueries.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;

Q_DECLARE_METATYPE(MockItemFetchJob*)
Q_DECLARE_METATYPE(MockCollectionFetchJob*)

class AkonadiTagQueriesTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldLookInAllReportedForAllTag()
    {
        // GIVEN

        // Two tag with their corresponding akonadi plain tag !
        Akonadi::Tag akonadiTag1("tag42");
        akonadiTag1.setId(Akonadi::Tag::Id(42));
        // domain tag
        Domain::Tag::Ptr tag1(new Domain::Tag);
        tag1->setName("tag42");

        Akonadi::Tag akonadiTag2("tag43");
        akonadiTag2.setId(Akonadi::Tag::Id(43));
        // domain tag
        Domain::Tag::Ptr tag2(new Domain::Tag);
        tag2->setName("tag43");

        MockTagFetchJob *tagFetchJob = new MockTagFetchJob(this);
        tagFetchJob->setTags(Akonadi::Tag::List() << akonadiTag1 << akonadiTag2);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchTags).when().thenReturn(tagFetchJob);

        // Serializer mock returning tags from tags
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag1).thenReturn(tag1);
        serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag2).thenReturn(tag2);

        // WHEN
        QScopedPointer<Domain::TagQueries> queries(new Akonadi::TagQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   new MockMonitor(this)));

        Domain::QueryResult<Domain::Tag::Ptr>::Ptr result = queries->findAll();
        result->data();
        result = queries->findAll(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);

        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchTags).when().exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag2).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), tag1);
        QCOMPARE(result->data().at(1), tag2);

    }

    void shouldReactToTagAdded()
    {
        // GIVEN

        MockTagFetchJob *tagFetchJob = new MockTagFetchJob(this);
        tagFetchJob->setTags(Akonadi::Tag::List()); // empty tag list

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchTags).when().thenReturn(tagFetchJob);

        // Serializer mock returning tags from tags
        mock_object<Akonadi::SerializerInterface> serializerMock;

        // Mocked Monitor
        MockMonitor* monitor = new MockMonitor(this);

        QScopedPointer<Domain::TagQueries> queries(new Akonadi::TagQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   monitor));

        Domain::QueryResult<Domain::Tag::Ptr>::Ptr result = queries->findAll();
        QTest::qWait(150);
        QVERIFY(result->data().isEmpty());

        // WHEN
        Akonadi::Tag akonadiTag1("tag42");
        akonadiTag1.setType(QByteArray(Akonadi::Tag::PLAIN));
        akonadiTag1.setId(Akonadi::Tag::Id(42));
        Domain::Tag::Ptr tag1(new Domain::Tag);
        tag1->setName("tag42");

        Akonadi::Tag akonadiTag2("tag43");
        akonadiTag2.setId(Akonadi::Tag::Id(43));
        akonadiTag2.setType(QByteArray(Akonadi::Tag::PLAIN));
        Domain::Tag::Ptr tag2(new Domain::Tag);
        tag2->setName("tag43");

        serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag1).thenReturn(tag1);
        serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag2).thenReturn(tag2);

        monitor->addTag(akonadiTag1);
        monitor->addTag(akonadiTag2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchTags).when().exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag2).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), tag1);
        QCOMPARE(result->data().at(1), tag2);
    }

    void shouldReactToTagRemoved()
    {
        // GIVEN

        // Two tags
        Akonadi::Tag akonadiTag1("tag42");
        akonadiTag1.setType(QByteArray(Akonadi::Tag::PLAIN));
        akonadiTag1.setId(Akonadi::Tag::Id(42));
        Domain::Tag::Ptr tag1(new Domain::Tag);
        tag1->setName("tag42");

        Akonadi::Tag akonadiTag2("tag43");
        akonadiTag2.setId(Akonadi::Tag::Id(43));
        akonadiTag2.setType(QByteArray(Akonadi::Tag::PLAIN));
        Domain::Tag::Ptr tag2(new Domain::Tag);
        tag2->setName("tag43");

        MockTagFetchJob *tagFetchJob = new MockTagFetchJob(this);
        tagFetchJob->setTags(Akonadi::Tag::List() << akonadiTag1 << akonadiTag2);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchTags).when().thenReturn(tagFetchJob);

        MockMonitor* monitor = new MockMonitor(this);

        // Serializer mock returning tags from akonadi tags
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag1).thenReturn(tag1);
        serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag2).thenReturn(tag2);
        serializerMock(&Akonadi::SerializerInterface::representsAkonadiTag).when(tag1, akonadiTag2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::representsAkonadiTag).when(tag2, akonadiTag2).thenReturn(true);

        QScopedPointer<Domain::TagQueries> queries(new Akonadi::TagQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   monitor));

        Domain::QueryResult<Domain::Tag::Ptr>::Ptr result = queries->findAll();

        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 2);

        // WHEN
        monitor->removeTag(akonadiTag2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchTags).when().exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag2).exactly(1));
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0), tag1);
    }

    void shouldReactToTagChanges()
    {
        // GIVEN

        // Two tags
        Akonadi::Tag akonadiTag1("tag42");
        akonadiTag1.setType(QByteArray(Akonadi::Tag::PLAIN));
        akonadiTag1.setId(Akonadi::Tag::Id(42));
        Domain::Tag::Ptr tag1(new Domain::Tag);
        tag1->setName("tag42");

        Akonadi::Tag akonadiTag2("tag43");
        akonadiTag2.setId(Akonadi::Tag::Id(43));
        akonadiTag2.setType(QByteArray(Akonadi::Tag::PLAIN));
        Domain::Tag::Ptr tag2(new Domain::Tag);
        tag2->setName("tag43");

        MockTagFetchJob *tagFetchJob = new MockTagFetchJob(this);
        tagFetchJob->setTags(Akonadi::Tag::List() << akonadiTag1 << akonadiTag2);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchTags).when().thenReturn(tagFetchJob);

        MockMonitor* monitor = new MockMonitor(this);

        // Serializer mock returning domain tags from akonadi tags
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag1).thenReturn(tag1);
        serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag2).thenReturn(tag2);
        serializerMock(&Akonadi::SerializerInterface::representsAkonadiTag).when(tag1, akonadiTag2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::representsAkonadiTag).when(tag2, akonadiTag2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::updateTagFromAkonadiTag).when(tag2, akonadiTag2).thenReturn();

        QScopedPointer<Domain::TagQueries> queries(new Akonadi::TagQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   monitor));

        Domain::QueryResult<Domain::Tag::Ptr>::Ptr result = queries->findAll();

        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 2);

        // WHEN
        akonadiTag2.setName("newTag43");
        monitor->changeTag(akonadiTag2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchTags).when().exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::updateTagFromAkonadiTag).when(tag2, akonadiTag2).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), tag1);
        QCOMPARE(result->data().at(1), tag2);
        QCOMPARE(result->data().at(1)->name(), tag2->name());
    }

    void shouldLookInAllCollectionsForTagTopLevelArtifacts()
    {
        // GIVEN

        // Two top level collections
        Akonadi::Collection col1(42);
        col1.setParentCollection(Akonadi::Collection::root());
        Akonadi::Collection col2(43);
        col2.setParentCollection(Akonadi::Collection::root());
        auto collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2);

        // One domain Tag and it's corresponding akonadiTag
        auto tag = Domain::Tag::Ptr::create();
        Akonadi::Tag akonadiTag(42);

        //two tasks in the first collection
        Akonadi::Item item1(43);
        item1.setParentCollection(col1);
        auto task1 = Domain::Task::Ptr::create();
        Akonadi::Item item2(44);
        item2.setParentCollection(col1);
        auto task2 = Domain::Task::Ptr::create();
        auto itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << item1 << item2);

        // Two notes in the second collection
        Akonadi::Item item3(45);
        item3.setParentCollection(col2);
        auto note3 = Domain::Note::Ptr::create();
        Akonadi::Item item4(46);
        item4.setParentCollection(col2);
        auto note4 = Domain::Note::Ptr::create();
        auto itemFetchJob2 = new MockItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << item3 << item4);

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
        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item1).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item3).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isTaskItem).when(item4).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item3).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isNoteItem).when(item4).thenReturn(true);

        serializerMock(&Akonadi::SerializerInterface::createAkonadiTagFromTag).when(tag).thenReturn(akonadiTag);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item1).thenReturn(task1);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).thenReturn(task2);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item3).thenReturn(Domain::Task::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item4).thenReturn(Domain::Task::Ptr());
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item3).thenReturn(note3);
        serializerMock(&Akonadi::SerializerInterface::createNoteFromItem).when(item4).thenReturn(note4);

        // Serializer mock returning if tag is hold by the items
        serializerMock(&Akonadi::SerializerInterface::isTagChild).when(tag, item1).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isTagChild).when(tag, item2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isTagChild).when(tag, item3).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isTagChild).when(tag, item4).thenReturn(false);

        // WHEN
        QScopedPointer<Domain::TagQueries> queries(new Akonadi::TagQueries(&storageMock.getInstance(),
                                                                           &serializerMock.getInstance(),
                                                                           new MockMonitor(this)));

        Domain::QueryResult<Domain::Artifact::Ptr>::Ptr result = queries->findTopLevelArtifacts(tag);
        result->data();
        result = queries->findTopLevelArtifacts(tag); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col1).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col2).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0).objectCast<Domain::Task>(), task1);
        QCOMPARE(result->data().at(1).objectCast<Domain::Note>(), note3);

        // Should not change nothing
        result = queries->findTopLevelArtifacts(tag);

        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col1).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(col2).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0).objectCast<Domain::Task>(), task1);
        QCOMPARE(result->data().at(1).objectCast<Domain::Note>(), note3);
    }
};

QTEST_MAIN(AkonadiTagQueriesTest)

#include "akonaditagqueriestest.moc"
