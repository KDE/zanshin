/*
 * SPDX-FileCopyrightText: 2014 Franck Arrecot <franck.arrecot@gmail.com>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "akonadi/akonadicachingstorage.h"
#include "akonadi/akonadicontextqueries.h"
#include "akonadi/akonadiserializer.h"

#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/gentodo.h"
#include "testlib/testhelpers.h"

using namespace Testlib;

class AkonadiContextQueriesTest : public QObject
{
    Q_OBJECT
private:
    Akonadi::StorageInterface::Ptr createCachingStorage(AkonadiFakeData &data, const Akonadi::Cache::Ptr &cache)
    {
        auto storage = Akonadi::StorageInterface::Ptr(data.createStorage());
        return Akonadi::StorageInterface::Ptr(new Akonadi::CachingStorage(cache, storage));
    }

private slots:
    void shouldDealWithEmptyContextList()
    {
        // GIVEN
        AkonadiFakeData data;

        // WHEN
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   Akonadi::Cache::Ptr()));
        auto result = queries->findAll();

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());
        QCOMPARE(result->data().size(), 0);
    }

    void shouldLookInAllReportedForAllContexts()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Two contexts
        data.createItem(GenTodo().withParent(42).withId(42).withUid("ctx-42").withTitle(QStringLiteral("42")).asContext());
        data.createItem(GenTodo().withParent(42).withId(43).withUid("ctx-43").withTitle(QStringLiteral("43")).asContext());

        // WHEN
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   Akonadi::Cache::Ptr()));
        auto result = queries->findAll();
        result->data();
        result = queries->findAll(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43"));
    }

    void shouldIgnoreNonContexts()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Two items, one of which not a context
        data.createItem(GenTodo().withParent(42).withId(42).withUid("ctx-42").withTitle(QStringLiteral("42")).asContext());
        data.createItem(GenTodo().withParent(42).withId(43).withUid("ctx-43").withTitle(QStringLiteral("43")));

        // WHEN
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   Akonadi::Cache::Ptr()));
        auto result = queries->findAll();
        result->data();
        result = queries->findAll(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
    }

    void shouldReactToContextAdded()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   Akonadi::Cache::Ptr()));
        auto result = queries->findAll();
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createItem(GenTodo().withParent(42).withId(42).withUid("ctx-42").withTitle(QStringLiteral("42")).asContext());
        data.createItem(GenTodo().withParent(42).withId(43).withUid("ctx-43").withTitle(QStringLiteral("43")).asContext());

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43"));
    }

    void shouldReactToContextRemoved()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Two contexts
        data.createItem(GenTodo().withParent(42).withId(42).withUid("ctx-42").withTitle(QStringLiteral("42")).asContext());
        data.createItem(GenTodo().withParent(42).withId(43).withUid("ctx-43").withTitle(QStringLiteral("43")).asContext());

        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   Akonadi::Cache::Ptr()));
        auto result = queries->findAll();
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.removeItem(Akonadi::Item(43));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
    }

    void shouldReactToContextChanges()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Two contexts
        data.createItem(GenTodo().withParent(42).withId(42).withUid("ctx-42").withTitle(QStringLiteral("42")).asContext());
        data.createItem(GenTodo().withParent(42).withId(43).withUid("ctx-43").withTitle(QStringLiteral("43")).asContext());

        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   Akonadi::Cache::Ptr()));
        auto result = queries->findAll();
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyItem(GenTodo(data.item(43)).withTitle(QStringLiteral("43bis")));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43bis"));
    }

    void shouldLookForAllContextTopLevelTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One context
        data.createItem(GenTodo().withParent(42).withId(40).withUid("ctx").withTitle(QStringLiteral("Context")).asContext());

        // Three tasks in the collection, two related to context, one not
        data.createItem(GenTodo().withParent(42).withId(42).withTitle(QStringLiteral("42")).withContexts({"ctx"}));
        data.createItem(GenTodo().withParent(42).withId(43).withTitle(QStringLiteral("43")));
        data.createItem(GenTodo().withParent(42).withId(44).withTitle(QStringLiteral("44")).withContexts({"ctx"}));

        // WHEN
        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context = serializer->createContextFromItem(data.item(40));
        auto result = queries->findTopLevelTasks(context);
        result->data();
        result = queries->findTopLevelTasks(context); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42"));
        QCOMPARE(result->data().at(1)->title(), QStringLiteral("44"));

        // Should not change nothing
        result = queries->findTopLevelTasks(context);
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42"));
        QCOMPARE(result->data().at(1)->title(), QStringLiteral("44"));
    }

    void shouldNotListContextTasksTwiceIfTheyHaveAParentInTheSameContext()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One context item
        data.createItem(GenTodo().withParent(42).withId(40).withUid("ctx").withTitle(QStringLiteral("Context")).asContext());

        // Five tasks in the collection, two related to context, three not, all forming an ancestry line
        data.createItem(GenTodo().withParent(42).withId(42).withTitle(QStringLiteral("42")).withUid("42"));
        data.createItem(GenTodo().withParent(42).withId(43).withTitle(QStringLiteral("43")).withUid("43").withParentUid("42").withContexts({"ctx"}));
        data.createItem(GenTodo().withParent(42).withId(44).withTitle(QStringLiteral("44")).withUid("44").withParentUid("43"));
        data.createItem(GenTodo().withParent(42).withId(45).withTitle(QStringLiteral("45")).withUid("45").withParentUid("44").withContexts({"ctx"}));
        data.createItem(GenTodo().withParent(42).withId(46).withTitle(QStringLiteral("46")).withUid("46").withParentUid("45"));

        // WHEN
        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context = serializer->createContextFromItem(data.item(40));
        auto result = queries->findTopLevelTasks(context);
        result->data();
        result = queries->findTopLevelTasks(context); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("43"));

        // Should not change anything
        result = queries->findTopLevelTasks(context);
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("43"));
    }

    void shouldReactToItemAddsForTopLevelTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One context item
        data.createItem(GenTodo().withParent(42).withId(40).withUid("ctx").withTitle(QStringLiteral("Context")).asContext());

        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context = serializer->createContextFromItem(data.item(40));
        auto result = queries->findTopLevelTasks(context);
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createItem(GenTodo().withParent(42).withId(42).withTitle(QStringLiteral("42")).withContexts({"ctx"}));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42"));
    }

    void shoudlReactToItemChangesForTopLevelTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One context
        data.createItem(GenTodo().withParent(42).withId(40).withUid("ctx").withTitle(QStringLiteral("Context")).asContext());

        // One task related to the context
        data.createItem(GenTodo().withParent(42).withId(42).withTitle(QStringLiteral("42")).withContexts({"ctx"}));

        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context = serializer->createContextFromItem(data.item(40));
        auto result = queries->findTopLevelTasks(context);

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::Task::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);

        // WHEN
        data.modifyItem(GenTodo(data.item(42)).withTitle(QStringLiteral("42bis")));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42bis"));

        QVERIFY(replaceHandlerCalled);
    }

    void shouldAddItemToCorrespondingResultWhenContextAddedToTopLevelTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One context item
        data.createItem(GenTodo().withParent(42).withId(40).withUid("ctx").withTitle(QStringLiteral("Context")).asContext());

        // One task not related to the context
        data.createItem(GenTodo().withParent(42).withId(42).withTitle(QStringLiteral("42")));

        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context = serializer->createContextFromItem(data.item(40));
        auto result = queries->findTopLevelTasks(context);
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.modifyItem(GenTodo(data.item(42)).withContexts({"ctx"}));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42"));
    }

    void shouldRemoveItemFromCorrespondingResultWhenContextRemovedFromTopLevelTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One context item
        data.createItem(GenTodo().withParent(42).withId(40).withUid("ctx").withTitle(QStringLiteral("Context")).asContext());

        // One task related to the context
        data.createItem(GenTodo().withParent(42).withId(42).withTitle(QStringLiteral("42")).withContexts({"ctx"}));

        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context = serializer->createContextFromItem(data.item(40));
        QVERIFY(context);
        auto result = queries->findTopLevelTasks(context);
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);

        // WHEN
        data.modifyItem(GenTodo(data.item(42)).withContexts({}));

        // THEN
        QVERIFY(result->data().isEmpty());
    }

    void shouldMoveItemToCorrespondingResultWhenContextChangedOnTopLevelTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Two context items
        data.createItem(GenTodo().withParent(42).withId(1).withUid("ctx-1").withTitle(QStringLiteral("Context 1")).asContext());
        data.createItem(GenTodo().withParent(42).withId(2).withUid("ctx-2").withTitle(QStringLiteral("Context 2")).asContext());

        // One task related to the first context
        data.createItem(GenTodo().withParent(42).withId(42).withTitle(QStringLiteral("42")).withContexts({"ctx-1"}));

        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context1 = serializer->createContextFromItem(data.item(1));
        auto result1 = queries->findTopLevelTasks(context1);
        auto context2 = serializer->createContextFromItem(data.item(2));
        auto result2 = queries->findTopLevelTasks(context2);
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result1->data().size(), 1);
        QCOMPARE(result1->data().at(0)->title(), QStringLiteral("42"));
        QVERIFY(result2->data().isEmpty());

        // WHEN
        data.modifyItem(GenTodo(data.item(42)).withContexts({"ctx-2"}));

        // THEN
        QVERIFY(result1->data().isEmpty());
        QCOMPARE(result2->data().size(), 1);
        QCOMPARE(result2->data().at(0)->title(), QStringLiteral("42"));
    }

    void shoudlReactToItemRemovesForTopLevelTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One context item
        data.createItem(GenTodo().withParent(42).withId(1).withUid("ctx-1").withTitle(QStringLiteral("Context 1")).asContext());

        // A task related to the context
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")).withContexts({"ctx-1"}));

        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context = serializer->createContextFromItem(data.item(1));
        QVERIFY(context);
        auto result = queries->findTopLevelTasks(context);

        bool removeHandlerCalled = false;
        result->addPostRemoveHandler([&removeHandlerCalled](const Domain::Task::Ptr &, int) {
                                          removeHandlerCalled = true;
                                      });

        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42"));

        // WHEN
        data.removeItem(Akonadi::Item(42));

        // THEN
        QVERIFY(result->data().isEmpty());
        QVERIFY(removeHandlerCalled);
    }
};

ZANSHIN_TEST_MAIN(AkonadiContextQueriesTest)

#include "akonadicontextqueriestest.moc"
