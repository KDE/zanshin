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

#include <testlib/qtest_zanshin.h>

#include "akonadi/akonadicachingstorage.h"
#include "akonadi/akonadicontextqueries.h"
#include "akonadi/akonadiserializer.h"

#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/gentodo.h"
#include "testlib/gentag.h"
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
    void shouldDealWithEmptyTagList()
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

        // Two context tags
        data.createTag(GenTag().withId(42).withName(QStringLiteral("42")).asContext());
        data.createTag(GenTag().withId(43).withName(QStringLiteral("43")).asContext());

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

    void shouldIgnoreWrongTagType()
    {
        // GIVEN
        AkonadiFakeData data;

        // One context tag and one plain tag
        data.createTag(GenTag().withId(42).withName(QStringLiteral("42")).asContext());
        data.createTag(GenTag().withId(43).withName(QStringLiteral("43")).asPlain());

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

    void shouldReactToTagAdded()
    {
        // GIVEN
        AkonadiFakeData data;

        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   Akonadi::Cache::Ptr()));
        auto result = queries->findAll();
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createTag(GenTag().withId(42).withName(QStringLiteral("42")).asContext());
        data.createTag(GenTag().withId(43).withName(QStringLiteral("43")).asContext());

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43"));
    }

    void shouldReactToTagRemoved()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two context tags
        data.createTag(GenTag().withId(42).withName(QStringLiteral("42")).asContext());
        data.createTag(GenTag().withId(43).withName(QStringLiteral("43")).asContext());

        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   Akonadi::Cache::Ptr()));
        auto result = queries->findAll();
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.removeTag(Akonadi::Tag(43));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
    }

    void shouldReactToTagChanges()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two context tags
        data.createTag(GenTag().withId(42).withName(QStringLiteral("42")).asContext());
        data.createTag(GenTag().withId(43).withName(QStringLiteral("43")).asContext());

        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   Akonadi::Cache::Ptr()));
        auto result = queries->findAll();
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyTag(GenTag(data.tag(43)).withName(QStringLiteral("43bis")));

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

        // One context tag
        data.createTag(GenTag().withId(42).withName(QStringLiteral("42")).asContext());

        // Three tasks in the collection, two related to context, one not
        data.createItem(GenTodo().withParent(42).withId(42).withTitle(QStringLiteral("42")).withTags({42}));
        data.createItem(GenTodo().withParent(42).withId(43).withTitle(QStringLiteral("43")));
        data.createItem(GenTodo().withParent(42).withId(44).withTitle(QStringLiteral("44")).withTags({42}));

        // WHEN
        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context = serializer->createContextFromTag(data.tag(42));
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

        // One context tag
        data.createTag(GenTag().withId(42).withName(QStringLiteral("42")).asContext());

        // Five tasks in the collection, two related to context, three not, all forming an ancestry line
        data.createItem(GenTodo().withParent(42).withId(42).withTitle(QStringLiteral("42")).withUid("42"));
        data.createItem(GenTodo().withParent(42).withId(43).withTitle(QStringLiteral("43")).withUid("43").withParentUid("42").withTags({42}));
        data.createItem(GenTodo().withParent(42).withId(44).withTitle(QStringLiteral("44")).withUid("44").withParentUid("43"));
        data.createItem(GenTodo().withParent(42).withId(45).withTitle(QStringLiteral("45")).withUid("45").withParentUid("44").withTags({42}));
        data.createItem(GenTodo().withParent(42).withId(46).withTitle(QStringLiteral("46")).withUid("46").withParentUid("45"));

        // WHEN
        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context = serializer->createContextFromTag(data.tag(42));
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

        // One context tag
        data.createTag(GenTag().withId(42).withName(QStringLiteral("42")).asContext());

        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context = serializer->createContextFromTag(data.tag(42));
        auto result = queries->findTopLevelTasks(context);
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createItem(GenTodo().withParent(42).withId(42).withTitle(QStringLiteral("42")).withTags({42}));

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

        // One context tag
        data.createTag(GenTag().withId(42).withName(QStringLiteral("42")).asContext());

        // One task related to the context
        data.createItem(GenTodo().withParent(42).withId(42).withTitle(QStringLiteral("42")).withTags({42}));

        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context = serializer->createContextFromTag(data.tag(42));
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

    void shouldAddItemToCorrespondingResultWhenTagAddedToTopLevelTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One context tag
        data.createTag(GenTag().withId(42).withName(QStringLiteral("42")).asContext());

        // One task not related to the context
        data.createItem(GenTodo().withParent(42).withId(42).withTitle(QStringLiteral("42")));

        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context = serializer->createContextFromTag(data.tag(42));
        auto result = queries->findTopLevelTasks(context);
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.modifyItem(GenTodo(data.item(42)).withTags({42}));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42"));
    }

    void shouldRemoveItemFromCorrespondingResultWhenTagRemovedFromTopLevelTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One context tag
        data.createTag(GenTag().withId(42).withName(QStringLiteral("42")).asContext());

        // One task related to the context
        data.createItem(GenTodo().withParent(42).withId(42).withTitle(QStringLiteral("42")).withTags({42}));

        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context = serializer->createContextFromTag(data.tag(42));
        auto result = queries->findTopLevelTasks(context);
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);

        // WHEN
        data.modifyItem(GenTodo(data.item(42)).withTags({}));

        // THEN
        QVERIFY(result->data().isEmpty());
    }

    void shouldMoveItemToCorrespondingResultWhenTagChangedOnTopLevelTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Two context tags
        data.createTag(GenTag().withId(42).withName(QStringLiteral("42")).asContext());
        data.createTag(GenTag().withId(43).withName(QStringLiteral("43")).asContext());

        // One task related to the first context
        data.createItem(GenTodo().withParent(42).withId(42).withTitle(QStringLiteral("42")).withTags({42}));

        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context1 = serializer->createContextFromTag(data.tag(42));
        auto result1 = queries->findTopLevelTasks(context1);
        auto context2 = serializer->createContextFromTag(data.tag(43));
        auto result2 = queries->findTopLevelTasks(context2);
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result1->data().size(), 1);
        QCOMPARE(result1->data().at(0)->title(), QStringLiteral("42"));
        QVERIFY(result2->data().isEmpty());

        // WHEN
        data.modifyItem(GenTodo(data.item(42)).withTags({43}));

        // THEN
        QVERIFY(result1->data().isEmpty());
        QCOMPARE(result2->data().size(), 1);
        QCOMPARE(result2->data().at(0)->title(), QStringLiteral("42"));
    }

    void shoudlReactToItemRemovesForTopLevelTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One context tag
        data.createTag(GenTag().withId(42).withName(QStringLiteral("tag-42")).asContext());

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // A task related to the context
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")).withTags({42}));


        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        QScopedPointer<Domain::ContextQueries> queries(new Akonadi::ContextQueries(createCachingStorage(data, cache),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor()),
                                                                                   cache));

        auto context = serializer->createContextFromTag(data.tag(42));
        auto result = queries->findTopLevelTasks(context);

        bool removeHandlerCalled = false;
        result->addPostRemoveHandler([&removeHandlerCalled](const Domain::Artifact::Ptr &, int) {
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
