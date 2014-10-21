/* This file is part of Zanshin

   Copyright 2014 Franck Arrecot <franck.arrecot@gmail.com>

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

#include "akonadi/akonadicontextqueries.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;

class AkonadiContextQueriesTest : public QObject
{
    Q_OBJECT

private slots:
    void shouldDealWithEmptyTagList()
    {
        // GIVEN

        // Empty TagFetch mock
        MockTagFetchJob *tagFetchJob = new MockTagFetchJob(this);
        tagFetchJob->setTags(Akonadi::Tag::List());
        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchTags).when().thenReturn(tagFetchJob);

        // Serializer mock returning contexts from tags
        mock_object<Akonadi::SerializerInterface> serializerMock;

        // WHEN
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   new MockMonitor(this)));

        Domain::QueryResult<Domain::Context::Ptr>::Ptr result = queries->findAll();

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchTags).when().exactly(1));
        QCOMPARE(result->data().size(), 0);
    }

    void shouldLookInAllReportedForAllContexts()
    {
        // GIVEN

        // Two contexts
        Akonadi::Tag tag1("context42");
        tag1.setId(Akonadi::Tag::Id(42));
        tag1.setType(Akonadi::SerializerInterface::contextTagType());
        Domain::Context::Ptr context1(new Domain::Context);
        context1->setName("context42");

        Akonadi::Tag tag2("context43");
        tag2.setId(Akonadi::Tag::Id(43));
        tag2.setType(Akonadi::SerializerInterface::contextTagType());
        Domain::Context::Ptr context2(new Domain::Context);
        context2->setName("context43");

        MockTagFetchJob *tagFetchJob = new MockTagFetchJob(this);
        tagFetchJob->setTags(Akonadi::Tag::List() << tag1 << tag2);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchTags).when().thenReturn(tagFetchJob);

        // Serializer mock returning contexts from tags
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag1).thenReturn(context1);
        serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag2).thenReturn(context2);

        // WHEN
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   new MockMonitor(this)));

        Domain::QueryResult<Domain::Context::Ptr>::Ptr result = queries->findAll();
        result->data();
        result = queries->findAll(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);

        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchTags).when().exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag2).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), context1);
        QCOMPARE(result->data().at(1), context2);

    }

    void shouldIgnoreWrongTagType()
    {
        // GIVEN

        // Two contexts
        Akonadi::Tag tag1("context42");
        tag1.setId(Akonadi::Tag::Id(42));
        tag1.setType(Akonadi::SerializerInterface::contextTagType());
        Domain::Context::Ptr context1(new Domain::Context);
        context1->setName("context42");

        Akonadi::Tag tag2("context43");
        tag2.setId(Akonadi::Tag::Id(43));
        tag2.setType(QByteArray("wrongType")); // wrong type
        Domain::Context::Ptr context2; // null context

        MockTagFetchJob *tagFetchJob = new MockTagFetchJob(this);
        tagFetchJob->setTags(Akonadi::Tag::List() << tag1 << tag2);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchTags).when().thenReturn(tagFetchJob);

        // Serializer mock returning contexts from tags
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag1).thenReturn(context1);
        serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag2).thenReturn(context2);

        // WHEN
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   new MockMonitor(this)));

        Domain::QueryResult<Domain::Context::Ptr>::Ptr result = queries->findAll();

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);

        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchTags).when().exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag2).exactly(0));

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0), context1);
    }

    void shouldReactToTagAdded()
    {
        // GIVEN

        MockTagFetchJob *tagFetchJob = new MockTagFetchJob(this);
        tagFetchJob->setTags(Akonadi::Tag::List()); // empty tag list

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchTags).when().thenReturn(tagFetchJob);

        // Serializer mock returning contexts from tags
        mock_object<Akonadi::SerializerInterface> serializerMock;

        // Mocked Monitor
        MockMonitor* monitor = new MockMonitor(this);

        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   monitor));

        Domain::QueryResult<Domain::Context::Ptr>::Ptr result = queries->findAll();
        QTest::qWait(150);
        QVERIFY(result->data().isEmpty());

        // WHEN
        Akonadi::Tag tag1("context42");
        tag1.setType(Akonadi::SerializerInterface::contextTagType());
        tag1.setId(Akonadi::Tag::Id(42));
        Domain::Context::Ptr context1(new Domain::Context);
        context1->setName("context42");

        Akonadi::Tag tag2("context43");
        tag2.setId(Akonadi::Tag::Id(43));
        tag2.setType(Akonadi::SerializerInterface::contextTagType());
        Domain::Context::Ptr context2(new Domain::Context);
        context2->setName("context43");

        serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag1).thenReturn(context1);
        serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag2).thenReturn(context2);

        monitor->addTag(tag1);
        monitor->addTag(tag2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchTags).when().exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag2).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), context1);
        QCOMPARE(result->data().at(1), context2);
    }

    void shouldReactToTagRemoved()
    {
        // GIVEN

        // Two contexts
        Akonadi::Tag tag1("context42");
        tag1.setId(Akonadi::Tag::Id(42));
        tag1.setType(Akonadi::SerializerInterface::contextTagType());
        Domain::Context::Ptr context1(new Domain::Context);
        context1->setName("context42");

        Akonadi::Tag tag2("context43");
        tag2.setId(Akonadi::Tag::Id(43));
        tag2.setType(Akonadi::SerializerInterface::contextTagType());
        Domain::Context::Ptr context2(new Domain::Context);
        context2->setName("context43");

        MockTagFetchJob *tagFetchJob = new MockTagFetchJob(this);
        tagFetchJob->setTags(Akonadi::Tag::List() << tag1 << tag2);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchTags).when().thenReturn(tagFetchJob);

        MockMonitor* monitor = new MockMonitor(this);

        // Serializer mock returning contexts from tags
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag1).thenReturn(context1);
        serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag2).thenReturn(context2);
        serializerMock(&Akonadi::SerializerInterface::isContextTag).when(context1, tag2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isContextTag).when(context2, tag2).thenReturn(true);

        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   monitor));

        Domain::QueryResult<Domain::Context::Ptr>::Ptr result = queries->findAll();

        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 2);

        // WHEN
        monitor->removeTag(tag2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchTags).when().exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag2).exactly(1));
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0), context1);
    }

    void shouldReactToTagChanges()
    {
        // GIVEN

        // Two contexts
        Akonadi::Tag tag1("context42");
        tag1.setId(Akonadi::Tag::Id(42));
        tag1.setType(Akonadi::SerializerInterface::contextTagType());
        Domain::Context::Ptr context1(new Domain::Context);
        context1->setName("context42");

        Akonadi::Tag tag2("context43");
        tag2.setId(Akonadi::Tag::Id(43));
        tag2.setType(Akonadi::SerializerInterface::contextTagType());
        Domain::Context::Ptr context2(new Domain::Context);
        context2->setName("context43");

        MockTagFetchJob *tagFetchJob = new MockTagFetchJob(this);
        tagFetchJob->setTags(Akonadi::Tag::List() << tag1 << tag2);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchTags).when().thenReturn(tagFetchJob);

        MockMonitor* monitor = new MockMonitor(this);

        // Serializer mock returning contexts from tags
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag1).thenReturn(context1);
        serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag2).thenReturn(context2);
        serializerMock(&Akonadi::SerializerInterface::isContextTag).when(context1, tag2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isContextTag).when(context2, tag2).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::updateContextFromTag).when(context2, tag2).thenReturn();

        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   monitor));

        Domain::QueryResult<Domain::Context::Ptr>::Ptr result = queries->findAll();

        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 2);

        // WHEN
        tag2.setName("newContext43");
        monitor->changeTag(tag2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchTags).when().exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createContextFromTag).when(tag2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::updateContextFromTag).when(context2, tag2).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), context1);
        QCOMPARE(result->data().at(1), context2);
        QCOMPARE(result->data().at(1)->name(), context2->name());
    }

    void shouldLookForAllContextTopLevelTasks()
    {
        // GIVEN

        // A context
        Akonadi::Tag tag(43);
        auto context = Domain::Context::Ptr::create();

        // Two tasks related to context
        Akonadi::Item item1(44);
        auto task1 = Domain::Task::Ptr::create();
        Akonadi::Item item2(47);
        auto task2 = Domain::Task::Ptr::create();

        //A fetch job returning the items tagged with the context
        auto itemFetchJob = new MockItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item1 << item2);

        //Mock object returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchTagItems).when(tag).thenReturn(itemFetchJob);

        // Serializer mock returning the objects from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createTagFromContext).when(context).thenReturn(tag);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item1).thenReturn(task1);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(item2).thenReturn(task2);
        serializerMock(&Akonadi::SerializerInterface::isContextChild).when(context, item1).thenReturn(true);
        serializerMock(&Akonadi::SerializerInterface::isContextChild).when(context, item2).thenReturn(true);

        // WHEN
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   new MockMonitor(this)));
        Domain::QueryResult<Domain::Task::Ptr>::Ptr result = queries->findTopLevelTasks(context);
        result->data();
        result = queries->findTopLevelTasks(context); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchTagItems).when(tag).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), task1);
        QCOMPARE(result->data().at(1), task2);

        // Should not change anything

        result = queries->findTopLevelTasks(context);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchTagItems).when(tag).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), task1);
        QCOMPARE(result->data().at(1), task2);
    }
};

QTEST_MAIN(AkonadiContextQueriesTest)

#include "AkonadiContextQueriesTest.moc"
