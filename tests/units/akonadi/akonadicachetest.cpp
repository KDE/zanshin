/*
 * SPDX-FileCopyrightText: 2017 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "akonadi/akonadicache.h"
#include "akonadi/akonadiserializer.h"

#include "testlib/akonadifakemonitor.h"
#include "testlib/gencollection.h"
#include "testlib/gentodo.h"

using namespace Testlib;

class AkonadiCacheTest : public QObject
{
    Q_OBJECT

private slots:
    void shouldHaveDefaultState()
    {
        // GIVEN
        auto monitor = AkonadiFakeMonitor::Ptr::create();
        auto cache = Akonadi::Cache::Ptr::create(Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer), monitor);

        // THEN
        QVERIFY(!cache->isCollectionListPopulated());
        QVERIFY(cache->collections().isEmpty());
    }

    void shouldStoreCollectionsAndUpdate()
    {
        // GIVEN
        const auto noteCollection = Akonadi::Collection(GenCollection().withRootAsParent()
                                                                       .withId(1)
                                                                       .withName("notes")
                                                                       .withNoteContent());
        const auto taskCollection = Akonadi::Collection(GenCollection().withRootAsParent()
                                                                       .withId(2)
                                                                       .withName("tasks")
                                                                       .withTaskContent());
        const auto noteTaskCollection = Akonadi::Collection(GenCollection().withRootAsParent()
                                                                           .withId(3)
                                                                           .withName("tasks+notes")
                                                                           .withTaskContent()
                                                                           .withNoteContent());
        const auto stuffCollection = Akonadi::Collection(GenCollection().withRootAsParent()
                                                                        .withId(4)
                                                                        .withName("stuff"));

        auto monitor = AkonadiFakeMonitor::Ptr::create();
        auto cache = Akonadi::Cache::Ptr::create(Akonadi::Serializer::Ptr(new Akonadi::Serializer), monitor);

        // WHEN
        cache->setCollections(Akonadi::Collection::List() << stuffCollection << noteTaskCollection
                                                          << taskCollection << noteCollection);

        // THEN
        QVERIFY(cache->isCollectionListPopulated());
        QVERIFY(cache->isCollectionKnown(stuffCollection.id()));
        QVERIFY(!cache->isCollectionPopulated(stuffCollection.id()));
        QVERIFY(cache->items(stuffCollection).isEmpty());
        QCOMPARE(cache->collections(),
                 Akonadi::Collection::List() << noteTaskCollection << taskCollection);
        QCOMPARE(cache->allCollections(),
                 Akonadi::Collection::List() << stuffCollection << noteTaskCollection
                 << taskCollection << noteCollection);
        QCOMPARE(cache->collection(stuffCollection.id()), stuffCollection);
        QCOMPARE(cache->collection(stuffCollection.id()).name(), stuffCollection.name());

        // WHEN
        monitor->changeCollection(GenCollection(stuffCollection).withName("stuff2"));

        // THEN
        QCOMPARE(cache->collection(stuffCollection.id()).name(), QStringLiteral("stuff2"));

        // WHEN
        monitor->changeCollection(GenCollection(noteTaskCollection).withName("note+task2"));

        // THEN
        QCOMPARE(cache->collection(noteTaskCollection.id()).name(), QStringLiteral("note+task2"));

        // WHEN
        monitor->changeCollection(GenCollection(taskCollection).withName("task2"));

        // THEN
        QCOMPARE(cache->collection(taskCollection.id()).name(), QStringLiteral("task2"));
    }

    void shouldHandleCollectionAdds_data()
    {
        QTest::addColumn<Akonadi::Collection>("collection");
        QTest::addColumn<bool>("seen");

        const auto none = Akonadi::Collection(GenCollection().withRootAsParent()
                                                             .withId(2)
                                                             .withName("collection"));
        const auto task = Akonadi::Collection(GenCollection(none).withTaskContent());
        const auto note = Akonadi::Collection(GenCollection(none).withNoteContent());
        const auto taskNote = Akonadi::Collection(GenCollection(none).withNoteContent().withTaskContent());

        QTest::newRow("tasks vs none") << none << false;
        QTest::newRow("tasks vs task") << task << true;
        QTest::newRow("tasks vs note") << note << false;
        QTest::newRow("tasks vs taskNote") << taskNote << true;
    }

    void shouldHandleCollectionAdds()
    {
        // GIVEN
        QFETCH(Akonadi::Collection, collection);

        auto monitor = AkonadiFakeMonitor::Ptr::create();
        auto cache = Akonadi::Cache::Ptr::create(Akonadi::Serializer::Ptr(new Akonadi::Serializer), monitor);

        // WHEN
        monitor->addCollection(collection);

        // THEN
        QVERIFY(!cache->isCollectionListPopulated());
        QVERIFY(cache->collections().isEmpty());
        QCOMPARE(cache->collection(collection.id()), Akonadi::Collection());

        // WHEN
        cache->setCollections(Akonadi::Collection::List());
        monitor->addCollection(collection);

        // THEN
        QVERIFY(cache->isCollectionListPopulated());
        QFETCH(bool, seen);
        if (seen) {
            QVERIFY(!cache->collections().isEmpty());
            QCOMPARE(cache->collection(collection.id()), collection);
            QCOMPARE(cache->collection(collection.id()).name(), collection.name());
        } else {
            QVERIFY(cache->collections().isEmpty());
            QCOMPARE(cache->collection(collection.id()), Akonadi::Collection());
        }
    }

    void shouldHandleCollectionChanges()
    {
        // GIVEN
        const auto collection = Akonadi::Collection(GenCollection().withRootAsParent()
                                                                   .withId(2)
                                                                   .withName("tasks")
                                                                   .withTaskContent());

        auto monitor = AkonadiFakeMonitor::Ptr::create();
        auto cache = Akonadi::Cache::Ptr::create(Akonadi::Serializer::Ptr(new Akonadi::Serializer), monitor);
        cache->setCollections(Akonadi::Collection::List() << collection);

        // WHEN
        const auto collection2 = Akonadi::Collection(GenCollection().withRootAsParent()
                                                                    .withId(2)
                                                                    .withName("tasks2")
                                                                    .withTaskContent());
        monitor->changeCollection(collection2);

        // THEN
        QCOMPARE(cache->collection(collection.id()).name(), QStringLiteral("tasks2"));
    }

    void shouldPopulateCollectionsWithItems()
    {
        // GIVEN
        const auto taskCollection1 = Akonadi::Collection(GenCollection().withRootAsParent()
                                                                        .withId(1)
                                                                        .withName("tasks1")
                                                                        .withTaskContent());
        const auto items1 = Akonadi::Item::List() << Akonadi::Item(GenTodo().withId(1).withTitle("item1"))
                                                  << Akonadi::Item(GenTodo().withId(2).withTitle("item2"));
        const auto taskCollection2 = Akonadi::Collection(GenCollection().withRootAsParent()
                                                                        .withId(2)
                                                                        .withName("tasks2")
                                                                        .withTaskContent());
        const auto items2 = Akonadi::Item::List() << Akonadi::Item(GenTodo().withId(3).withTitle("item3"))
                                                  << Akonadi::Item(GenTodo().withId(4).withTitle("item4"));

        auto monitor = AkonadiFakeMonitor::Ptr::create();
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, monitor);
        cache->setCollections(Akonadi::Collection::List() << taskCollection1 << taskCollection2);

        // WHEN
        cache->populateCollection(taskCollection1, items1);

        // THEN
        QVERIFY(cache->isCollectionPopulated(taskCollection1.id()));
        QCOMPARE(cache->items(taskCollection1), items1);
        QCOMPARE(cache->item(items1.at(0).id()), items1.at(0));
        QCOMPARE(cache->item(items1.at(1).id()), items1.at(1));

        // WHEN
        cache->populateCollection(taskCollection2, items2);

        // THEN
        QVERIFY(cache->isCollectionPopulated(taskCollection2.id()));
        QCOMPARE(cache->items(taskCollection2), items2);
        QCOMPARE(cache->item(items2.at(0).id()), items2.at(0));
        QCOMPARE(cache->item(items2.at(1).id()), items2.at(1));
    }

    void shouldHandleCollectionRemoves()
    {
        // GIVEN
        const auto collection1 = Akonadi::Collection(GenCollection().withRootAsParent()
                                                                    .withId(1)
                                                                    .withName("tasks1")
                                                                    .withTaskContent());
        const auto items1 = Akonadi::Item::List() << Akonadi::Item(GenTodo().withId(1).withTitle("item1"))
                                                  << Akonadi::Item(GenTodo().withId(2).withTitle("item2"));
        const auto collection2 = Akonadi::Collection(GenCollection().withRootAsParent()
                                                                    .withId(2)
                                                                    .withName("tasks2")
                                                                    .withTaskContent());
        const auto items2 = Akonadi::Item::List() << Akonadi::Item(GenTodo().withId(3).withTitle("item3"))
                                                  << Akonadi::Item(GenTodo().withId(4).withTitle("item4"));

        auto monitor = AkonadiFakeMonitor::Ptr::create();
        auto cache = Akonadi::Cache::Ptr::create(Akonadi::Serializer::Ptr(new Akonadi::Serializer), monitor);
        cache->setCollections(Akonadi::Collection::List() << collection1 << collection2);
        cache->populateCollection(collection1, items1);
        cache->populateCollection(collection2, items2);

        // WHEN
        monitor->removeCollection(collection1);

        // THEN
        QVERIFY(!cache->isCollectionPopulated(collection1.id()));
        QVERIFY(cache->items(collection1).isEmpty());
        QCOMPARE(cache->item(items1.at(0).id()), Akonadi::Item());
        QCOMPARE(cache->item(items1.at(1).id()), Akonadi::Item());

        QVERIFY(cache->isCollectionPopulated(collection2.id()));
        QCOMPARE(cache->items(collection2), items2);
        QCOMPARE(cache->item(items2.at(0).id()), items2.at(0));
        QCOMPARE(cache->item(items2.at(1).id()), items2.at(1));
    }

    void shouldHandleItemChanges()
    {
        // GIVEN
        const auto collection1 = Akonadi::Collection(GenCollection().withRootAsParent()
                                                                    .withId(1)
                                                                    .withName("tasks1")
                                                                    .withTaskContent());
        const auto collection2 = Akonadi::Collection(GenCollection().withRootAsParent()
                                                                    .withId(2)
                                                                    .withName("tasks2")
                                                                    .withTaskContent());
        const auto collection3 = Akonadi::Collection(GenCollection().withRootAsParent()
                                                                    .withId(3)
                                                                    .withName("tasks3")
                                                                    .withTaskContent());
        const auto items = Akonadi::Item::List() << Akonadi::Item(GenTodo().withId(1).withParent(1).withContexts({"ctx-1"}).withTitle("item1"))
                                                 << Akonadi::Item(GenTodo().withId(2).withParent(1).withContexts({"ctx-1"}).withTitle("item2"));
        const auto item3 = Akonadi::Item(GenTodo().withId(3).withParent(1).withContexts({"ctx-1"}).withTitle("item3"));


        auto monitor = AkonadiFakeMonitor::Ptr::create();
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, monitor);
        cache->setCollections(Akonadi::Collection::List() << collection1 << collection2 << collection3);
        cache->populateCollection(collection1, items);
        cache->populateCollection(collection2, Akonadi::Item::List());

        // WHEN
        monitor->changeItem(GenTodo(items.at(0)).withTitle("item1bis"));

        // THEN
        auto todo = serializer->createTaskFromItem(cache->item(items.at(0).id()));
        QCOMPARE(todo->title(), QStringLiteral("item1bis"));

        QVERIFY(cache->isCollectionPopulated(collection1.id()));
        QVERIFY(cache->items(collection1).contains(items.at(0)));
        QVERIFY(cache->isCollectionPopulated(collection2.id()));
        QVERIFY(!cache->items(collection2).contains(items.at(0)));
        QVERIFY(!cache->isCollectionPopulated(collection3.id()));
        QVERIFY(!cache->items(collection3).contains(items.at(0)));

        // WHEN
        monitor->changeItem(GenTodo(items.at(0)).withParent(2));

        // THEN
        QVERIFY(cache->isCollectionPopulated(collection1.id()));
        QVERIFY(!cache->items(collection1).contains(items.at(0)));
        QVERIFY(cache->isCollectionPopulated(collection2.id()));
        QVERIFY(cache->items(collection2).contains(items.at(0)));
        QVERIFY(!cache->isCollectionPopulated(collection3.id()));
        QVERIFY(!cache->items(collection3).contains(items.at(0)));

        // WHEN
        monitor->changeItem(GenTodo(items.at(0)).withParent(3));

        // THEN
        QVERIFY(cache->isCollectionPopulated(collection1.id()));
        QVERIFY(!cache->items(collection1).contains(items.at(0)));
        QVERIFY(cache->isCollectionPopulated(collection2.id()));
        QVERIFY(!cache->items(collection2).contains(items.at(0)));
        QVERIFY(!cache->isCollectionPopulated(collection3.id()));
        QVERIFY(!cache->items(collection3).contains(items.at(0)));

        // WHEN
        monitor->changeItem(GenTodo().withId(1).withParent(2).withContexts({"ctx-2"}).withTitle("item1"));

        // THEN
        QVERIFY(cache->isCollectionPopulated(collection1.id()));
        QVERIFY(!cache->items(collection1).contains(items.at(0)));
        QVERIFY(cache->isCollectionPopulated(collection2.id()));
        QVERIFY(cache->items(collection2).contains(items.at(0)));
        QVERIFY(!cache->isCollectionPopulated(collection3.id()));
        QVERIFY(!cache->items(collection3).contains(items.at(0)));

        // WHEN
        monitor->changeItem(item3);

        // THEN
        QVERIFY(cache->items(collection1).contains(item3));
    }

    void shouldHandleItemAdds()
    {
        // GIVEN
        const auto collection1 = Akonadi::Collection(GenCollection().withRootAsParent()
                                                                    .withId(1)
                                                                    .withName("tasks1")
                                                                    .withTaskContent());
        const auto collection2 = Akonadi::Collection(GenCollection().withRootAsParent()
                                                     .withId(2)
                                                     .withName("tasks2")
                                                     .withTaskContent());
        const auto item1 = Akonadi::Item(GenTodo().withId(1).withParent(1).withContexts({"ctx-1"}).withTitle("item1"));
        const auto item2 = Akonadi::Item(GenTodo().withId(2).withParent(2).withContexts({"ctx-2"}).withTitle("item2"));


        auto monitor = AkonadiFakeMonitor::Ptr::create();
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        auto cache = Akonadi::Cache::Ptr::create(serializer, monitor);
        cache->setCollections(Akonadi::Collection::List() << collection1 << collection2);
        cache->populateCollection(collection1, Akonadi::Item::List());

        // WHEN
        monitor->addItem(item1);

        // THEN
        QVERIFY(cache->isCollectionPopulated(collection1.id()));
        QVERIFY(!cache->items(collection1).isEmpty());
        QCOMPARE(cache->items(collection1), Akonadi::Item::List() << item1);
        QVERIFY(!cache->isCollectionPopulated(collection2.id()));
        QVERIFY(cache->items(collection2).isEmpty());

        // WHEN
        monitor->addItem(item2);

        // THEN
        QVERIFY(cache->isCollectionPopulated(collection1.id()));
        QVERIFY(!cache->items(collection1).isEmpty());
        QCOMPARE(cache->items(collection1), Akonadi::Item::List() << item1);
        QVERIFY(!cache->isCollectionPopulated(collection2.id()));
        QVERIFY(cache->items(collection2).isEmpty());
    }

    void shouldHandleItemRemoves()
    {
        // GIVEN
        const auto collection = Akonadi::Collection(GenCollection().withRootAsParent()
                                                                   .withId(1)
                                                                   .withName("tasks")
                                                                   .withTaskContent());
        const auto items = Akonadi::Item::List() << Akonadi::Item(GenTodo().withId(1).withTitle("item1"))
                                                  << Akonadi::Item(GenTodo().withId(2).withTitle("item2"));

        auto monitor = AkonadiFakeMonitor::Ptr::create();
        auto cache = Akonadi::Cache::Ptr::create(Akonadi::Serializer::Ptr(new Akonadi::Serializer), monitor);
        cache->setCollections(Akonadi::Collection::List() << collection);
        cache->populateCollection(collection, items);

        // WHEN
        monitor->removeItem(items.at(0));

        // THEN
        QVERIFY(cache->isCollectionPopulated(collection.id()));
        QCOMPARE(cache->items(collection), Akonadi::Item::List() << items.at(1));
        QCOMPARE(cache->item(items.at(0).id()), Akonadi::Item());
        QCOMPARE(cache->item(items.at(1).id()), items.at(1));
    }
};

ZANSHIN_TEST_MAIN(AkonadiCacheTest)

#include "akonadicachetest.moc"
