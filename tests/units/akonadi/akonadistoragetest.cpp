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

#include <KCalCore/Todo>
#include <KCalCore/ICalFormat>

#include <Akonadi/Collection>
#include <Akonadi/CollectionCreateJob>
#include <Akonadi/CollectionDeleteJob>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/CollectionStatistics>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemModifyJob>

#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonadimonitorimpl.h"
#include "akonadi/akonadistorage.h"
#include "akonadi/akonadistoragesettings.h"

Q_DECLARE_METATYPE(Akonadi::StorageInterface::FetchDepth);

class AkonadiStorageTest : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiStorageTest(QObject *parent = 0)
        : QObject(parent)
    {
        qRegisterMetaType<Akonadi::Collection>();
        qRegisterMetaType<Akonadi::Item>();
    }

private slots:
    void shouldListCollections_data()
    {
        QTest::addColumn<Akonadi::Collection>("collection");
        QTest::addColumn<QStringList>("expectedNames");
        QTest::addColumn<Akonadi::StorageInterface::FetchDepth>("depth");
        QTest::addColumn<int>("contentTypes");

        QTest::newRow("all") << Akonadi::Collection::root()
                             << QStringList({ "Calendar1", "Calendar2", "Calendar3", "Change me!", "Destroy me!", "Notes" })
                             << Akonadi::Storage::Recursive
                             << int(Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks);
        QTest::newRow("notes") << Akonadi::Collection::root()
                               << QStringList({ "Notes" })
                               << Akonadi::Storage::Recursive
                               << int(Akonadi::StorageInterface::Notes);
        QTest::newRow("tasks") << Akonadi::Collection::root()
                               << QStringList({ "Calendar1", "Calendar2", "Calendar3", "Change me!", "Destroy me!" })
                               << Akonadi::Storage::Recursive
                               << int(Akonadi::StorageInterface::Tasks);
        QTest::newRow("base type") << calendar2()
                                   << QStringList({"Calendar2"})
                                   << Akonadi::Storage::Base
                                   << int(Akonadi::StorageInterface::Tasks);
        QTest::newRow("firstLevel type") << calendar1()
                                   << QStringList({"Calendar2"})
                                   << Akonadi::Storage::FirstLevel
                                   << int(Akonadi::StorageInterface::Tasks);
        QTest::newRow("recursive type") << calendar1()
                                        << QStringList({"Calendar2", "Calendar3"})
                                        << Akonadi::Storage::Recursive
                                        << int(Akonadi::StorageInterface::Tasks);
    }

    void shouldListCollections()
    {
        // GIVEN
        Akonadi::Storage storage;
        QFETCH(Akonadi::Collection, collection);
        QFETCH(QStringList, expectedNames);
        QFETCH(Akonadi::StorageInterface::FetchDepth, depth);
        QFETCH(int, contentTypes);

        // WHEN
        auto job = storage.fetchCollections(collection, depth,
                                            Akonadi::StorageInterface::FetchContentTypes(contentTypes));
        job->kjob()->exec();

        // THEN
        auto collections = job->collections();
        QStringList collectionNames;
        for (const auto &collection : collections) {
            collectionNames << collection.name();
        }
        collectionNames.sort();

        QCOMPARE(collectionNames, expectedNames);
    }

    void shouldRetrieveAllCollectionAncestors()
    {
        // GIVEN
        Akonadi::Storage storage;

        // WHEN
        auto job = storage.fetchCollections(Akonadi::Collection::root(),
                                            Akonadi::Storage::Recursive,
                                            Akonadi::Storage::Tasks|Akonadi::Storage::Notes);
        job->kjob()->exec();

        // THEN
        auto collections = job->collections();
        for (const auto &collection : collections) {
            auto parent = collection.parentCollection();
            while (parent != Akonadi::Collection::root()) {
                QVERIFY(parent.isValid());
                QVERIFY(!parent.displayName().isEmpty());
                parent = parent.parentCollection();
            }
        }
    }


    void shouldListFullItemsInACollection()
    {
        // GIVEN
        Akonadi::Storage storage;
        const QStringList expectedRemoteIds = { "{1d33862f-f274-4c67-ab6c-362d56521ff4}",
                                                "{1d33862f-f274-4c67-ab6c-362d56521ff5}",
                                                "{1d33862f-f274-4c67-ab6c-362d56521ff6}",
                                                "{7824df00-2fd6-47a4-8319-52659dc82005}" };

        // WHEN
        auto job = storage.fetchItems(calendar2());
        job->kjob()->exec();

        // THEN
        auto items = job->items();
        QStringList itemRemoteIds;
        for (const auto &item : items) {
            itemRemoteIds << item.remoteId();
            QVERIFY(item.loadedPayloadParts().contains(Akonadi::Item::FullPayload));
            QVERIFY(!item.attributes().isEmpty());
            QVERIFY(item.modificationTime().isValid());
            QVERIFY(!item.flags().isEmpty());

            auto parent = item.parentCollection();
            while (parent != Akonadi::Collection::root()) {
                QVERIFY(parent.isValid());
                parent = parent.parentCollection();
            }
        }
        itemRemoteIds.sort();

        QCOMPARE(itemRemoteIds, expectedRemoteIds);
    }

    void shouldNotifyCollectionAdded()
    {
        // GIVEN

        // A spied monitor
        Akonadi::MonitorImpl monitor;
        QSignalSpy spy(&monitor, SIGNAL(collectionAdded(Akonadi::Collection)));

        // A collection
        Akonadi::Collection collection;
        collection.setParentCollection(calendar2());
        collection.setName("Foo!");
        collection.setContentMimeTypes(QStringList() << "application/x-vnd.akonadi.calendar.todo");

        // WHEN
        (new Akonadi::CollectionCreateJob(collection))->exec();
        // Give some time for the backend to signal back
        for (int i = 0; i < 10; i++) {
            if (!spy.isEmpty()) break;
            QTest::qWait(50);
        }

        // THEN
        QCOMPARE(spy.size(), 1);
        auto notifiedCollection = spy.takeFirst().takeFirst().value<Akonadi::Collection>();
        QCOMPARE(notifiedCollection.name(), collection.name());

        auto parent = notifiedCollection.parentCollection();
        while (parent != Akonadi::Collection::root()) {
            QVERIFY(parent.isValid());
            parent = parent.parentCollection();
        }
    }

    void shouldNotifyCollectionRemoved()
    {
        // GIVEN

        // A spied monitor
        Akonadi::MonitorImpl monitor;
        QSignalSpy spy(&monitor, SIGNAL(collectionRemoved(Akonadi::Collection)));

        // An existing item (if we trust the test data)
        Akonadi::Collection collection(6);

        // WHEN
        (new Akonadi::CollectionDeleteJob(collection))->exec();
        // Give some time for the backend to signal back
        for (int i = 0; i < 10; i++) {
            if (!spy.isEmpty()) break;
            QTest::qWait(50);
        }

        // THEN
        QCOMPARE(spy.size(), 1);
        auto notifiedCollection= spy.takeFirst().takeFirst().value<Akonadi::Collection>();
        QCOMPARE(notifiedCollection.id(), collection.id());
    }

    void shouldNotifyCollectionChanged()
    {
        // GIVEN

        // A spied monitor
        Akonadi::MonitorImpl monitor;
        QSignalSpy spy(&monitor, SIGNAL(collectionChanged(Akonadi::Collection)));

        // A colection with an existing id (if we trust the test data)
        Akonadi::Collection collection(7);
        collection.setName("Bar!");

        // WHEN
        (new Akonadi::CollectionModifyJob(collection))->exec();
        // Give some time for the backend to signal back
        for (int i = 0; i < 10; i++) {
            if (!spy.isEmpty()) break;
            QTest::qWait(50);
        }

        // THEN
        QCOMPARE(spy.size(), 1);
        auto notifiedCollection = spy.takeFirst().takeFirst().value<Akonadi::Collection>();
        QCOMPARE(notifiedCollection.id(), collection.id());
        QCOMPARE(notifiedCollection.name(), collection.name());

        auto parent = notifiedCollection.parentCollection();
        while (parent != Akonadi::Collection::root()) {
            QVERIFY(parent.isValid());
            parent = parent.parentCollection();
        }
    }


    void shouldNotifyItemAdded()
    {
        // GIVEN

        // A spied monitor
        Akonadi::MonitorImpl monitor;
        QSignalSpy spy(&monitor, SIGNAL(itemAdded(Akonadi::Item)));

        // A todo...
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setSummary("summary");
        todo->setDescription("content");
        todo->setCompleted(false);
        todo->setDtStart(KDateTime(QDate(2013, 11, 24)));
        todo->setDtDue(KDateTime(QDate(2014, 03, 01)));

        // ... as payload of an item...
        Akonadi::Item item;
        item.setMimeType("application/x-vnd.akonadi.calendar.todo");
        item.setPayload<KCalCore::Todo::Ptr>(todo);
        item.addAttribute(new Akonadi::EntityDisplayAttribute);

        // WHEN
        (new Akonadi::ItemCreateJob(item, calendar2()))->exec();
        // Give some time for the backend to signal back
        for (int i = 0; i < 10; i++) {
            if (!spy.isEmpty()) break;
            QTest::qWait(50);
        }

        // THEN
        QCOMPARE(spy.size(), 1);
        auto notifiedItem = spy.takeFirst().takeFirst().value<Akonadi::Item>();
        QCOMPARE(*notifiedItem.payload<KCalCore::Todo::Ptr>(), *todo);
        QVERIFY(notifiedItem.hasAttribute<Akonadi::EntityDisplayAttribute>());

        auto parent = notifiedItem.parentCollection();
        while (parent != Akonadi::Collection::root()) {
            QVERIFY(parent.isValid());
            parent = parent.parentCollection();
        }
    }

    void shouldNotifyItemRemoved()
    {
        // GIVEN

        // A spied monitor
        Akonadi::MonitorImpl monitor;
        QSignalSpy spy(&monitor, SIGNAL(itemRemoved(Akonadi::Item)));

        // An existing item (if we trust the test data)
        Akonadi::Item item(2);

        // WHEN
        (new Akonadi::ItemDeleteJob(item))->exec();
        // Give some time for the backend to signal back
        for (int i = 0; i < 10; i++) {
            if (!spy.isEmpty()) break;
            QTest::qWait(50);
        }

        // THEN
        QCOMPARE(spy.size(), 1);
        auto notifiedItem = spy.takeFirst().takeFirst().value<Akonadi::Item>();
        QCOMPARE(notifiedItem.id(), item.id());
    }

    void shouldNotifyItemChanged()
    {
        // GIVEN

        // A spied monitor
        Akonadi::MonitorImpl monitor;
        QSignalSpy spy(&monitor, SIGNAL(itemChanged(Akonadi::Item)));

        // A todo...
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setSummary("summary");
        todo->setDescription("content");
        todo->setCompleted(false);
        todo->setDtStart(KDateTime(QDate(2013, 11, 24)));
        todo->setDtDue(KDateTime(QDate(2014, 03, 01)));

        // ... as payload of an existing item (if we trust the test data)...
        Akonadi::Item item(1);
        item.setMimeType("application/x-vnd.akonadi.calendar.todo");
        item.setPayload<KCalCore::Todo::Ptr>(todo);
        item.addAttribute(new Akonadi::EntityDisplayAttribute);

        // WHEN
        (new Akonadi::ItemModifyJob(item))->exec();
        // Give some time for the backend to signal back
        for (int i = 0; i < 10; i++) {
            if (spy.size() == 2) break;
            QTest::qWait(50);
        }

        // THEN
        QCOMPARE(spy.size(), 2);
        for (int i = 0; i < spy.size(); i++) {
            auto notifiedItem = spy[i].takeFirst().value<Akonadi::Item>();
            QCOMPARE(notifiedItem.id(), item.id());
            QCOMPARE(*notifiedItem.payload<KCalCore::Todo::Ptr>(), *todo);
            QVERIFY(notifiedItem.hasAttribute<Akonadi::EntityDisplayAttribute>());

            auto parent = notifiedItem.parentCollection();
            while (parent != Akonadi::Collection::root()) {
                QVERIFY(parent.isValid());
                parent = parent.parentCollection();
            }
        }
    }

    void shouldReadDefaultTaskCollectionFromSettings()
    {
        // GIVEN

        // A storage implementation
        Akonadi::Storage storage;

        // WHEN
        Akonadi::StorageSettings::instance().setDefaultTaskCollection(Akonadi::Collection(24));

        // THEN
        QCOMPARE(storage.defaultTaskCollection(), Akonadi::Collection(24));
    }

    // This test must be run before shouldCreateItem because createItem
    // sometimes notifies an itemChanged with a delay. So this test might
    // receive this notification in addition to the itemChanged generated by updateItem.
    void shouldUpdateItem()
    {
        // GIVEN

        // A storage implementation
        Akonadi::Storage storage;

        // A spied monitor
        Akonadi::MonitorImpl monitor;
        QSignalSpy spy(&monitor, SIGNAL(itemChanged(Akonadi::Item)));

        // A todo...
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setSummary("new summary");
        todo->setDescription("new content");

        // ... as payload of an existing item (if we trust the test data)...
        Akonadi::Item item(1);
        item.setMimeType("application/x-vnd.akonadi.calendar.todo");
        item.setPayload<KCalCore::Todo::Ptr>(todo);

        // WHEN
        (storage.updateItem(item))->exec();
        // Give some time for the backend to signal back
        for (int i = 0; i < 10; i++) {
            if (!spy.isEmpty()) break;
            QTest::qWait(50);
        }

        // THEN
        QCOMPARE(spy.size(), 1);
        auto notifiedItem = spy.takeFirst().takeFirst().value<Akonadi::Item>();
        QCOMPARE(notifiedItem.id(), item.id());
        QCOMPARE(*notifiedItem.payload<KCalCore::Todo::Ptr>(), *todo);
    }

    void shouldCreateItem()
    {
        // GIVEN

        // A storage implementation
        Akonadi::Storage storage;

        // A spied monitor
        Akonadi::MonitorImpl monitor;
        QSignalSpy spy(&monitor, SIGNAL(itemAdded(Akonadi::Item)));

        // A todo...
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setSummary("summary");
        todo->setDescription("content");
        todo->setCompleted(false);
        todo->setDtStart(KDateTime(QDate(2013, 11, 24)));
        todo->setDtDue(KDateTime(QDate(2014, 03, 01)));

        // ... as payload of a new item
        Akonadi::Item item;
        item.setMimeType("application/x-vnd.akonadi.calendar.todo");
        item.setPayload<KCalCore::Todo::Ptr>(todo);

        // WHEN
        (storage.createItem(item, calendar2()))->exec();
        // Give some time for the backend to signal back
        for (int i = 0; i < 10; i++) {
            if (!spy.isEmpty()) break;
            QTest::qWait(50);
        }

        // THEN
        QCOMPARE(spy.size(), 1);
        auto notifiedItem = spy.takeFirst().takeFirst().value<Akonadi::Item>();
        QCOMPARE(notifiedItem.parentCollection(), calendar2());
        QCOMPARE(*notifiedItem.payload<KCalCore::Todo::Ptr>(), *todo);
    }

    void shouldRetrieveItem()
    {
        // GIVEN
        Akonadi::Storage storage;
        Akonadi::Item findItem(1);

        // WHEN
        auto job = storage.fetchItem(findItem);
        job->kjob()->exec();

        // THEN
        auto items = job->items();
        QCOMPARE(items.size(), 1);

        const auto &item = items[0];

        QCOMPARE(item.id(), 1LL);
        QVERIFY(item.loadedPayloadParts().contains(Akonadi::Item::FullPayload));
        QVERIFY(!item.attributes().isEmpty());
        QVERIFY(item.modificationTime().isValid());
        QVERIFY(!item.flags().isEmpty());

        auto parent = item.parentCollection();
        while (parent != Akonadi::Collection::root()) {
            QVERIFY(parent.isValid());
            parent = parent.parentCollection();
        }
    }

    void shouldMoveItem()
    {
        // GIVEN
        Akonadi::Storage storage;

        Akonadi::Item item(3);

        // A spied monitor
        Akonadi::MonitorImpl monitor;
        QSignalSpy spyMoved(&monitor, SIGNAL(itemMoved(Akonadi::Item)));

        (storage.moveItem(item, calendar1()))->exec();

        for (int i = 0; i < 10; i++) {
            if (!spyMoved.isEmpty()) break;
            QTest::qWait(50);
        }

        QCOMPARE(spyMoved.size(), 1);
        auto movedItem = spyMoved.takeFirst().takeFirst().value<Akonadi::Item>();
        QCOMPARE(movedItem.id(), item.id());
    }

    void shouldMoveItems()
    {
        // GIVEN
        Akonadi::Storage storage;

        Akonadi::Item item(4);
        Akonadi::Item::List list;
        list << item;

        // A spied monitor
        Akonadi::MonitorImpl monitor;
        QSignalSpy spyMoved(&monitor, SIGNAL(itemMoved(Akonadi::Item)));

        (storage.moveItems(list, calendar1()))->exec();

        for (int i = 0; i < 10; i++) {
            if (!spyMoved.isEmpty()) break;
            QTest::qWait(50);
        }

        QCOMPARE(spyMoved.size(), 1);
        auto movedItem = spyMoved.takeFirst().takeFirst().value<Akonadi::Item>();
        QCOMPARE(movedItem.id(), item.id());
    }

    void shouldUseTransaction()
    {
        // GIVEN
        Akonadi::Storage storage;

        Akonadi::Item item1(5);
        Akonadi::Item item2(6);
        // create wrong item
        Akonadi::Item item3(18);
        item3.setRemoteId("wrongId");

        // A spied monitor
        Akonadi::MonitorImpl monitor;
        QSignalSpy spyUpdated(&monitor, SIGNAL(itemChanged(Akonadi::Item)));

        auto job = storage.fetchItem(item1);
        job->kjob()->exec();
        QCOMPARE(job->items().size(), 1);
        item1 = job->items()[0];

        job = storage.fetchItem(item2);
        job->kjob()->exec();
        QCOMPARE(job->items().size(), 1);
        item2 = job->items()[0];

        auto todo = item1.payload<KCalCore::Todo::Ptr>();
        todo->setSummary("Buy tomatoes");

        todo = item2.payload<KCalCore::Todo::Ptr>();
        todo->setSummary("Buy chocolate");

        auto transaction = storage.createTransaction();
        storage.updateItem(item1, transaction);
        storage.updateItem(item3, transaction); // this job should failed
        storage.updateItem(item2, transaction);
        transaction->exec();

        for (int i = 0; i < 10; i++) {
            if (spyUpdated.size() == 3) break;
            QTest::qWait(50);
        }

        // Then
        QCOMPARE(spyUpdated.size(), 0);
        job = storage.fetchItem(item1);
        job->kjob()->exec();
        QCOMPARE(job->items().size(), 1);
        item1 = job->items()[0];

        job = storage.fetchItem(item2);
        job->kjob()->exec();
        QCOMPARE(job->items().size(), 1);
        item2 = job->items()[0];

        QCOMPARE(item1.payload<KCalCore::Todo::Ptr>()->summary(), QString("Buy kiwis"));
        QCOMPARE(item2.payload<KCalCore::Todo::Ptr>()->summary(), QString("Buy cheese"));
    }

    void shouldDeleteItem()
    {
        //GIVEN
        Akonadi::Storage storage;

        // A spied monitor
        Akonadi::MonitorImpl monitor;
        QSignalSpy spy(&monitor, SIGNAL(itemRemoved(Akonadi::Item)));

        // An existing item (if we trust the test data)
        Akonadi::Item item(1);

        //When
        (storage.removeItem(item))->exec();
        // Give some time for the backend to signal back
        for (int i = 0; i < 10; i++) {
            if (!spy.isEmpty()) break;
            QTest::qWait(50);
        }

        // THEN
        QCOMPARE(spy.size(), 1);
        auto notifiedItem = spy.takeFirst().takeFirst().value<Akonadi::Item>();
        QCOMPARE(notifiedItem.id(), item.id());
    }

private:
    Akonadi::Collection calendar1()
    {
        // Calendar1 is supposed to get this id, hopefully this won't be too fragile
        return Akonadi::Collection(3);
    }

    Akonadi::Collection calendar2()
    {
        // Calendar2 is supposed to get this id, hopefully this won't be too fragile
        return Akonadi::Collection(8);
    }
};

QTEST_MAIN(AkonadiStorageTest)

#include "akonadistoragetest.moc"
