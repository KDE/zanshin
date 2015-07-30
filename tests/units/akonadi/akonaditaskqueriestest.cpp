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

#include "akonadi/akonaditaskqueries.h"
#include "akonadi/akonadiserializer.h"

#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/gentodo.h"

#include "utils/datetime.h"
#include "utils/jobhandler.h"

using namespace Testlib;

class AkonadiTaskQueriesTest : public QObject
{
    Q_OBJECT

private:
    void waitForEmptyJobQueue()
    {
        while (Utils::JobHandler::jobCount() != 0) {
            QTest::qWait(20);
        }
    }

private slots:
    void shouldLookInAllReportedForAllTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withTaskContent());

        // One task in the first collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42"));

        // Two tasks in the second collection
        data.createItem(GenTodo().withId(43).withParent(43).withTitle("43"));
        data.createItem(GenTodo().withId(44).withParent(43).withTitle("44"));

        // WHEN
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        result->data();
        result = queries->findAll(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("43"));
        QCOMPARE(result->data().at(2)->title(), QString("44"));
    }

    void shouldIgnoreItemsWhichAreNotTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Two items in the collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42"));
        // One of them is not a task
        auto item = Akonadi::Item(43);
        item.setPayloadFromData("FooBar");
        item.setParentCollection(Akonadi::Collection(42));
        data.createItem(item);

        // WHEN
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();

        // THEN
        QVERIFY(result->data().isEmpty());
        waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
    }

    void shouldReactToItemAddsForTasksOnly()
    {
        // GIVEN
        AkonadiFakeData data;

        // One empty collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42"));
        auto item = Akonadi::Item(43);
        item.setPayloadFromData("FooBar");
        item.setParentCollection(Akonadi::Collection(42));
        data.createItem(item);

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first()->title(), QString("42"));
    }

    void shouldReactToItemRemovesForAllTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three task in the collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42"));
        data.createItem(GenTodo().withId(43).withParent(42).withTitle("43"));
        data.createItem(GenTodo().withId(44).withParent(42).withTitle("44"));

        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 3);

        // WHEN
        data.removeItem(Akonadi::Item(43));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("44"));
    }

    void shouldReactToItemChangesForAllTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three task in the collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42"));
        data.createItem(GenTodo().withId(43).withParent(42).withTitle("43"));
        data.createItem(GenTodo().withId(44).withParent(42).withTitle("44"));

        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        // Even though the pointer didn't change it's convenient to user if we call
        // the replace handlers
        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::Task::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 3);

        // WHEN
        data.modifyItem(GenTodo(data.item(43)).withTitle("43bis"));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("43bis"));
        QCOMPARE(result->data().at(2)->title(), QString("44"));
        QVERIFY(replaceHandlerCalled);
    }

    void shouldLookInAllChildrenReportedForAllChildrenTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection (two being children of the first one)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43")
                                 .withParentUid("uid-42"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        // WHEN
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             serializer,
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto task = serializer->createTaskFromItem(data.item(42));
        auto result = queries->findChildren(task);
        result->data();
        result = queries->findChildren(task); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("43"));
        QCOMPARE(result->data().at(1)->title(), QString("44"));

        // Should not change nothing
        result = queries->findChildren(task);

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("43"));
        QCOMPARE(result->data().at(1)->title(), QString("44"));
    }

    void shouldNotCrashWhenWeAskAgainTheSameChildrenList()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One task in the collection
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             serializer,
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto task = serializer->createTaskFromItem(data.item(42));

        // The bug we're trying to hit here is the following:
        //  - when findChildren is called the first time a provider is created internally
        //  - result is deleted at the end of the loop, no one holds the provider with
        //    a strong reference anymore so it is deleted as well
        //  - when findChildren is called the second time, there's a risk of a dangling
        //    pointer if the recycling of providers is wrongly implemented which can lead
        //    to a crash, if it is properly done no crash will occur
        for (int i = 0; i < 2; i++) {
            // WHEN * 2
            auto result = queries->findChildren(task);

            // THEN * 2
            QVERIFY(result->data().isEmpty());
            waitForEmptyJobQueue();
            QVERIFY(result->data().isEmpty());
        }
    }

    void shouldReactToItemAddsForChildrenTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One task in the collection
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             serializer,
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto task = serializer->createTaskFromItem(data.item(42));
        auto result = queries->findChildren(task);
        waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43")
                                 .withParentUid("uid-42"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("43"));
        QCOMPARE(result->data().at(1)->title(), QString("44"));
    }

    void shouldReactToItemChangesForChildrenTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection (two being children of the first one)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43")
                                 .withParentUid("uid-42"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             serializer,
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto task = serializer->createTaskFromItem(data.item(42));
        auto result = queries->findChildren(task);

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::Task::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyItem(GenTodo(data.item(43)).withTitle("43bis"));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("43bis"));
        QCOMPARE(result->data().at(1)->title(), QString("44"));

        QVERIFY(replaceHandlerCalled);
    }

    void shouldRemoveItemFromCorrespondingResultWhenRelatedItemChangeForChildrenTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection (two being children of the first one)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43")
                                 .withParentUid("uid-42"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             serializer,
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto task = serializer->createTaskFromItem(data.item(42));
        auto result = queries->findChildren(task);
        waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyItem(GenTodo(data.item(43)).withParentUid(""));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("44"));
    }

    void shouldAddItemToCorrespondingResultWhenRelatedItemChangeForChildrenTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection (two being top level)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             serializer,
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto task = serializer->createTaskFromItem(data.item(42));
        auto result = queries->findChildren(task);

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::Task::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);

        // WHEN
        data.modifyItem(GenTodo(data.item(43)).withParentUid("uid-42"));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("44"));
        QCOMPARE(result->data().at(1)->title(), QString("43"));

        QVERIFY(!replaceHandlerCalled);
    }

    void shouldMoveItemToCorrespondingResultWhenRelatedItemChangeForChildTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection (two being top level)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             serializer,
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto task1 = serializer->createTaskFromItem(data.item(42));
        auto task2 = serializer->createTaskFromItem(data.item(43));
        auto result1 = queries->findChildren(task1);
        auto result2 = queries->findChildren(task2);

        waitForEmptyJobQueue();
        QCOMPARE(result1->data().size(), 1);
        QCOMPARE(result1->data().at(0)->title(), QString("44"));
        QCOMPARE(result2->data().size(), 0);

        // WHEN
        data.modifyItem(GenTodo(data.item(44)).withParentUid("uid-43"));

        // THEN
        QCOMPARE(result1->data().size(), 0);
        QCOMPARE(result2->data().size(), 1);
        QCOMPARE(result2->data().at(0)->title(), QString("44"));
    }

    void shouldReactToItemRemovesForChildrenTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection (two being children of the first one)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43")
                                 .withParentUid("uid-42"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             serializer,
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto task = serializer->createTaskFromItem(data.item(42));
        auto result = queries->findChildren(task);
        waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.removeItem(Akonadi::Item(43));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("44"));
    }

    void shouldLookInAllReportedForTopLevelTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withTaskContent());

        // One task in the first collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42"));

        // Two tasks in the second collection
        data.createItem(GenTodo().withId(43).withParent(43).withTitle("43"));
        data.createItem(GenTodo().withId(44).withParent(43).withTitle("44").withParentUid("2"));

        // WHEN
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findTopLevel();
        result->data();
        result = queries->findTopLevel(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("43"));
    }

    void shouldReactToItemAddsForTopLevelTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findTopLevel();
        waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42"));
        data.createItem(GenTodo().withId(43).withParent(42).withTitle("43").withParentUid("2"));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first()->title(), QString("42"));
    }

    void shouldReactToItemRemovesForTopLevelTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection (one being child of the second one)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-43"));

        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findTopLevel();
        waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.removeItem(Akonadi::Item(43));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
    }

    void shouldReactToItemChangesForTopLevelTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection (one being child of the second one)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-43"));

        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findTopLevel();
        // Even though the pointer didn't change it's convenient to user if we call
        // the replace handlers
        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::Task::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyItem(GenTodo(data.item(43)).withTitle("43bis"));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("43bis"));
        QVERIFY(replaceHandlerCalled);
    }

    void shouldRemoveItemFromTopLevelResultWhenRelatedItemChangeForTopLevelTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection (one being child of the second one)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-43"));

        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findTopLevel();
        waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyItem(GenTodo(data.item(43)).withParentUid("uid-42"));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
    }

    void shouldAddItemToTopLevelResultWhenRelatedItemChangeForChildrenTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection (one being child of the second one)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43")
                                 .withParentUid("uid-42"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-43"));

        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findTopLevel();
        waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);

        // WHEN
        data.modifyItem(GenTodo(data.item(43)).withParentUid(QString()));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("43"));
    }

    void shouldRemoveParentNodeAndMoveChildrenInTopLevelResult()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection (one being child of the second one)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-43"));

        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findTopLevel();
        waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        auto resultChild = queries->findChildren(result->data().at(1));
        waitForEmptyJobQueue();
        QCOMPARE(resultChild->data().size(), 1);

        // WHEN
        data.removeItem(Akonadi::Item(43));

        // THEN
        QCOMPARE(resultChild->data().size(), 0);
        QCOMPARE(result->data().size(), 1); // FIXME: Should become 2 once we got a proper cache in place
    }

    void shouldNotCrashDuringFindChildrenWhenJobIsKilled()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection (two being children of the first one)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             serializer,
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        data.storageBehavior().setFetchItemErrorCode(42, KJob::KilledJobError);

        // WHEN
        auto task1 = serializer->createTaskFromItem(data.item(42));
        auto result = queries->findChildren(task1);

        // THEN
        QVERIFY(result->data().isEmpty());
        waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());
    }


    void shouldNotCrashDuringFindChildrenWhenItemsJobReceiveResult_data()
    {
        QTest::addColumn<int>("errorCode");
        QTest::addColumn<int>("fetchBehavior");
        QTest::addColumn<bool>("deleteQuery");

        QTest::newRow("No error with empty list") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::EmptyFetch) << false;
        QTest::newRow("Error with empty list") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::EmptyFetch) << true;
        QTest::newRow("Error with list") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::NormalFetch) << true;
    }

    void shouldNotCrashDuringFindChildrenWhenItemsJobReceiveResult()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection (two being children of the first one)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        auto storage = Akonadi::StorageInterface::Ptr(data.createStorage());
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        auto monitor = Akonadi::MonitorInterface::Ptr(data.createMonitor());
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(storage,
                                                                             serializer,
                                                                             monitor));

        QFETCH(int, errorCode);
        QFETCH(int, fetchBehavior);
        QFETCH(bool, deleteQuery);

        data.storageBehavior().setFetchItemsErrorCode(42, errorCode);
        data.storageBehavior().setFetchItemsBehavior(42, AkonadiFakeStorageBehavior::FetchBehavior(fetchBehavior));

        // WHEN
        auto task1 = serializer->createTaskFromItem(data.item(42));
        auto result = queries->findChildren(task1);

        if (deleteQuery)
            delete queries.take();

        // THEN
        QVERIFY(result->data().isEmpty());
        waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());
    }

    void shouldNotCrashDuringFindAllWhenFetchJobFailedOrEmpty_data()
    {
        QTest::addColumn<int>("colErrorCode");
        QTest::addColumn<int>("colFetchBehavior");
        QTest::addColumn<int>("itemsErrorCode");
        QTest::addColumn<int>("itemsFetchBehavior");
        QTest::addColumn<bool>("deleteQuery");

        QTest::newRow("No error with empty collection list") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                             << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                             << false;

        QTest::newRow("Error with empty collection list") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                          << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                          << true;

        QTest::newRow("Error with collection list") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                    << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                    << true;

        QTest::newRow("No error with empty item list") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                       << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                       << false;

        QTest::newRow("Error with empty item list") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                    << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                    << false;

        QTest::newRow("Error with item list") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                              << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                              << false;
    }

    void shouldNotCrashDuringFindAllWhenFetchJobFailedOrEmpty()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44"));

        auto storage = Akonadi::StorageInterface::Ptr(data.createStorage());
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        auto monitor = Akonadi::MonitorInterface::Ptr(data.createMonitor());
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(storage,
                                                                             serializer,
                                                                             monitor));

        QFETCH(int, colErrorCode);
        QFETCH(int, colFetchBehavior);
        data.storageBehavior().setFetchCollectionsErrorCode(Akonadi::Collection::root().id(), colErrorCode);
        data.storageBehavior().setFetchCollectionsBehavior(Akonadi::Collection::root().id(),
                                                           AkonadiFakeStorageBehavior::FetchBehavior(colFetchBehavior));

        QFETCH(int, itemsErrorCode);
        QFETCH(int, itemsFetchBehavior);
        data.storageBehavior().setFetchItemsErrorCode(42, itemsErrorCode);
        data.storageBehavior().setFetchItemsBehavior(42, AkonadiFakeStorageBehavior::FetchBehavior(itemsFetchBehavior));

        QFETCH(bool, deleteQuery);


        // WHEN
        auto result = queries->findAll();

        if (deleteQuery)
            delete queries.take();

        // THEN
        QVERIFY(result->data().isEmpty());
        waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());
    }

    void shouldNotCrashDuringFindTopLevelWhenFetchJobFailedOrEmpty_data()
    {
        QTest::addColumn<int>("colErrorCode");
        QTest::addColumn<int>("colFetchBehavior");
        QTest::addColumn<int>("itemsErrorCode");
        QTest::addColumn<int>("itemsFetchBehavior");
        QTest::addColumn<bool>("deleteQuery");

        QTest::newRow("No error with empty collection list") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                             << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                             << false;

        QTest::newRow("Error with empty collection list") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                          << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                          << true;

        QTest::newRow("Error with collection list") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                    << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                    << true;

        QTest::newRow("No error with empty item list") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                       << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                       << false;

        QTest::newRow("Error with empty item list") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                    << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                    << false;

        QTest::newRow("Error with item list") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                              << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                              << false;
    }

    void shouldNotCrashDuringFindTopLevelWhenFetchJobFailedOrEmpty()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection (third one being child of the first one)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        auto storage = Akonadi::StorageInterface::Ptr(data.createStorage());
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        auto monitor = Akonadi::MonitorInterface::Ptr(data.createMonitor());
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(storage,
                                                                             serializer,
                                                                             monitor));

        QFETCH(int, colErrorCode);
        QFETCH(int, colFetchBehavior);
        data.storageBehavior().setFetchCollectionsErrorCode(Akonadi::Collection::root().id(), colErrorCode);
        data.storageBehavior().setFetchCollectionsBehavior(Akonadi::Collection::root().id(),
                                                           AkonadiFakeStorageBehavior::FetchBehavior(colFetchBehavior));

        QFETCH(int, itemsErrorCode);
        QFETCH(int, itemsFetchBehavior);
        data.storageBehavior().setFetchItemsErrorCode(42, itemsErrorCode);
        data.storageBehavior().setFetchItemsBehavior(42, AkonadiFakeStorageBehavior::FetchBehavior(itemsFetchBehavior));

        QFETCH(bool, deleteQuery);


        // WHEN
        auto result = queries->findTopLevel();

        if (deleteQuery)
            delete queries.take();

        // THEN
        QVERIFY(result->data().isEmpty());
        waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());
    }

    void shouldIgnoreProjectsWhenReportingTopLevelTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withTaskContent());

        // One task in the first collection
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42"));

        // Two tasks and one project in the second collection
        data.createItem(GenTodo().withId(43).withParent(43)
                                 .withTitle("43").withUid("uid-43"));
        data.createItem(GenTodo().withId(44).withParent(43)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-43"));
        data.createItem(GenTodo().withId(45).withParent(43)
                                 .withTitle("45").withUid("uid-45")
                                 .asProject());

        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findTopLevel();
        result->data();
        result = queries->findTopLevel(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("43"));
    }

    void shouldLookInAllWorkdayReportedForAllTasks_data()
    {
        QTest::addColumn<bool>("isExpectedInWorkday");
        QTest::addColumn<Akonadi::Item>("item2");

        const auto today = Utils::DateTime::currentDateTime();

        QTest::newRow("todayTask") << true
                                   << Akonadi::Item(GenTodo()
                                                    .withStartDate(today)
                                                    .withDueDate(today));

        QTest::newRow("pastTask") << true
                                  << Akonadi::Item(GenTodo()
                                                   .withStartDate(today.addDays(-42))
                                                   .withDueDate(today.addDays(-41)));

        QTest::newRow("startTodayTask") << true
                                        << Akonadi::Item(GenTodo()
                                                         .withStartDate(today));

        QTest::newRow("endTodayTask") << true
                                      << Akonadi::Item(GenTodo()
                                                       .withStartDate(today));

        QTest::newRow("futureTask") << false
                                    << Akonadi::Item(GenTodo()
                                                     .withStartDate(today.addDays(41))
                                                     .withDueDate(today.addDays(42)));

        QTest::newRow("pastDoneTask") << false
                                      << Akonadi::Item(GenTodo()
                                                       .withStartDate(today.addDays(-42))
                                                       .withDueDate(today.addDays(-41))
                                                       .done()
                                                       .withDoneDate(today.addDays(-30)));

        QTest::newRow("todayDoneTask") << true
                                       << Akonadi::Item(GenTodo()
                                                        .withStartDate(today)
                                                        .withDueDate(today)
                                                        .done()
                                                        .withDoneDate(QDateTime(today.date(), QTime(12, 00))));

        QTest::newRow("startTodayDoneTask") << true
                                            << Akonadi::Item(GenTodo()
                                                             .withStartDate(today)
                                                             .done()
                                                             .withDoneDate(QDateTime(today.date(), QTime(12, 00))));

        QTest::newRow("endTodayDoneTask") << true
                                          << Akonadi::Item(GenTodo()
                                                           .withDueDate(today)
                                                           .done()
                                                           .withDoneDate(QDateTime(today.date(), QTime(12, 00))));
    }

    void shouldLookInAllWorkdayReportedForAllTasks()
    {
        // GIVEN
        const auto today = Utils::DateTime::currentDateTime();
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withTaskContent());

        // One task in the first collection
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42")
                                 .withStartDate(today.addSecs(300)));

        // One task in the second collection (from data driven set)
        QFETCH(Akonadi::Item, item2);
        data.createItem(GenTodo(item2).withId(43).withParent(43)
                                      .withTitle("43").withUid("uid-43"));

        // WHEN
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findWorkdayTopLevel();
        result->data();
        result = queries->findWorkdayTopLevel(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        waitForEmptyJobQueue();

        QFETCH(bool, isExpectedInWorkday);

        const int sizeExpected = (isExpectedInWorkday) ? 2 : 1;

        QCOMPARE(result->data().size(), sizeExpected);
        QCOMPARE(result->data().at(0)->title(), QString("42"));

        if (isExpectedInWorkday)
            QCOMPARE(result->data().at(1)->title(), QString("43"));
    }

    void shouldLookInAllWorkdayReportedForAllTasksWhenOverrideDate()
    {
        // GIVEN
        qputenv("ZANSHIN_OVERRIDE_DATETIME", "2015-03-10");
        const auto today = Utils::DateTime::currentDateTime();
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withTaskContent());

        // One task in the first collection
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42")
                                 .withStartDate(today));

        // One task in the second collection
        data.createItem(GenTodo().withId(43).withParent(43)
                                 .withTitle("43").withUid("uid-43")
                                 .withStartDate(today.addSecs(3600)));

        // WHEN
        QScopedPointer<Domain::TaskQueries> queries(new Akonadi::TaskQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                             Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                             Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findWorkdayTopLevel();
        result->data();
        result = queries->findWorkdayTopLevel(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("43"));
    }

};

QTEST_MAIN(AkonadiTaskQueriesTest)

#include "akonaditaskqueriestest.moc"
