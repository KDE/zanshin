/* This file is part of Zanshin

   Copyright 2014-2015 Kevin Ottens <ervin@kde.org>

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

#include "akonadistoragetestbase.h"

#include <QTest>
#include <QSignalSpy>

#include <testlib/akonadidebug.h>
#include <testlib/monitorspy.h>

#include <KCalCore/Todo>
#include <KCalCore/ICalFormat>

#include "utils/mem_fn.h"

#include "AkonadiCore/qtest_akonadi.h"

#include <AkonadiCore/EntityDisplayAttribute>

#include "akonadi/akonadiapplicationselectedattribute.h"
#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonadimonitorimpl.h"
#include "akonadi/akonadistorage.h"
#include "akonadi/akonadistoragesettings.h"
#include "akonadi/akonaditimestampattribute.h"

using namespace Testlib;

AkonadiStorageTestBase::AkonadiStorageTestBase(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<Akonadi::Collection>();
    qRegisterMetaType<Akonadi::Item>();
}

void AkonadiStorageTestBase::cleanupTestCase()
{
    // Give a chance for jobs still waiting for an event loop
    // run to be deleted through deleteLater()
    QTest::qWait(10);
}

void AkonadiStorageTestBase::dumpTree()
{
    TestLib::AkonadiDebug::dumpTree(createStorage());
}

void AkonadiStorageTestBase::shouldListCollections_data()
{
    QTest::addColumn<Akonadi::Collection>("collection");
    QTest::addColumn<QStringList>("expectedNames");
    QTest::addColumn<Akonadi::StorageInterface::FetchDepth>("depth");

    QTest::newRow("all") << Akonadi::Collection::root()
                         << QStringList({ "Calendar1", "Calendar2", "Calendar3", "Change me!", "Destroy me!" })
                         << Akonadi::Storage::Recursive;

    QTest::newRow("base type") << calendar2()
                               << QStringList({"Calendar2"})
                               << Akonadi::Storage::Base;

    QTest::newRow("firstLevel type") << calendar1()
                                     << QStringList({"Calendar2"})
                                     << Akonadi::Storage::FirstLevel;

    QTest::newRow("recursive type") << calendar1()
                                    << QStringList({"Calendar2", "Calendar3"})
                                    << Akonadi::Storage::Recursive;
}

void AkonadiStorageTestBase::shouldListCollections()
{
    // GIVEN
    QFETCH(Akonadi::Collection, collection);
    QFETCH(QStringList, expectedNames);
    QFETCH(Akonadi::StorageInterface::FetchDepth, depth);

    auto storage = createStorage();

    // WHEN
    auto job = storage->fetchCollections(collection, depth);
    AKVERIFYEXEC(job->kjob());

    // THEN
    auto collections = job->collections();
    QStringList collectionNames;
    collectionNames.reserve(collections.size());
    foreach (const auto &collection, collections) {
        collectionNames << collection.name();
    }
    collectionNames.sort();

    QCOMPARE(collectionNames, expectedNames);
}

void AkonadiStorageTestBase::shouldRetrieveAllCollectionAncestors()
{
    // GIVEN
    auto storage = createStorage();

    // WHEN
    auto job = storage->fetchCollections(Akonadi::Collection::root(),
                                         Akonadi::Storage::Recursive);
    AKVERIFYEXEC(job->kjob());

    // THEN
    auto collections = job->collections();
    foreach (const auto &collection, collections) {
        auto parent = collection.parentCollection();
        while (parent != Akonadi::Collection::root()) {
            QVERIFY(parent.isValid());
            QVERIFY(!parent.displayName().isEmpty());
            parent = parent.parentCollection();
        }
    }
}

void AkonadiStorageTestBase::shouldListFullItemsInACollection()
{
    // GIVEN
    auto storage = createStorage();
    const QStringList expectedRemoteIds = { "{1d33862f-f274-4c67-ab6c-362d56521ff4}",
                                            "{1d33862f-f274-4c67-ab6c-362d56521ff5}",
                                            "{1d33862f-f274-4c67-ab6c-362d56521ff6}",
                                            "{7824df00-2fd6-47a4-8319-52659dc82005}",
                                            "{7824df00-2fd6-47a4-8319-52659dc82006}",
                                            "{d0159c99-0d23-41fa-bb5f-tasktoremove}",
                                          };

    // WHEN
    auto job = storage->fetchItems(calendar2());
    AKVERIFYEXEC(job->kjob());

    // THEN
    auto items = job->items();
    QStringList itemRemoteIds;
    itemRemoteIds.reserve(items.size());
    foreach (const auto &item, items) {
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

void AkonadiStorageTestBase::shouldNotifyCollectionAdded()
{
    // GIVEN

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::collectionAdded);
    MonitorSpy monitorSpy(monitor.data());

    // A collection
    Akonadi::Collection collection;
    collection.setParentCollection(calendar2());
    collection.setName(QStringLiteral("Foo!"));
    collection.setContentMimeTypes(QStringList() << QStringLiteral("application/x-vnd.akonadi.calendar.todo"));

    // WHEN
    auto storage = createStorage();
    auto job = storage->createCollection(collection);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedCollection = spy.takeFirst().at(0).value<Akonadi::Collection>();
    QCOMPARE(notifiedCollection.name(), collection.name());

    auto parent = notifiedCollection.parentCollection();
    while (parent != Akonadi::Collection::root()) {
        QVERIFY(parent.isValid());
        parent = parent.parentCollection();
    }
}

void AkonadiStorageTestBase::shouldNotifyCollectionRemoved()
{
    // GIVEN

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::collectionRemoved);
    MonitorSpy monitorSpy(monitor.data());

    // An existing item (if we trust the test data)
    Akonadi::Collection collection = fetchCollectionByRID(QStringLiteral("{1f78b360-a01b-4785-9187-75450190342c}"));
    QVERIFY(collection.isValid());

    // WHEN
    auto storage = createStorage();
    auto job = storage->removeCollection(collection);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedCollection= spy.takeFirst().at(0).value<Akonadi::Collection>();
    QCOMPARE(notifiedCollection.id(), collection.id());
}

void AkonadiStorageTestBase::shouldNotifyCollectionChanged()
{
    // GIVEN

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::collectionChanged);
    MonitorSpy monitorSpy(monitor.data());

    // A collection with an existing id (if we trust the test data)
    Akonadi::Collection collection = fetchCollectionByRID(QStringLiteral("{28ef9f03-4ebc-4e33-970f-f379775894f9}"));
    QVERIFY(collection.isValid());
    collection.setName(QStringLiteral("Bar!"));

    // WHEN
    auto storage = createStorage();
    auto job = storage->updateCollection(collection);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedCollection = spy.takeFirst().at(0).value<Akonadi::Collection>();
    QCOMPARE(notifiedCollection.id(), collection.id());
    QCOMPARE(notifiedCollection.name(), collection.name());

    auto parent = notifiedCollection.parentCollection();
    while (parent != Akonadi::Collection::root()) {
        QVERIFY(parent.isValid());
        parent = parent.parentCollection();
    }
}

void AkonadiStorageTestBase::shouldNotifyItemAdded()
{
    // GIVEN

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemAdded);
    MonitorSpy monitorSpy(monitor.data());

    // A todo...
    KCalCore::Todo::Ptr todo(new KCalCore::Todo);
    todo->setSummary(QStringLiteral("summary"));
    todo->setDescription(QStringLiteral("content"));
    todo->setCompleted(false);
    todo->setDtStart(QDateTime(QDate(2013, 11, 24)));
    todo->setDtDue(QDateTime(QDate(2014, 03, 01)));

    // ... as payload of an item...
    Akonadi::Item item;
    item.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
    item.setPayload<KCalCore::Todo::Ptr>(todo);
    item.addAttribute(new Akonadi::EntityDisplayAttribute);

    // WHEN
    auto storage = createStorage();
    auto job = storage->createItem(item, calendar2());
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedItem = spy.takeFirst().at(0).value<Akonadi::Item>();
    QCOMPARE(*notifiedItem.payload<KCalCore::Todo::Ptr>(), *todo);
    QVERIFY(notifiedItem.hasAttribute<Akonadi::EntityDisplayAttribute>());

    auto parent = notifiedItem.parentCollection();
    while (parent != Akonadi::Collection::root()) {
        QVERIFY(parent.isValid());
        parent = parent.parentCollection();
    }
}

void AkonadiStorageTestBase::shouldNotifyItemRemoved()
{
    // GIVEN

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemRemoved);
    MonitorSpy monitorSpy(monitor.data());

    Akonadi::Item item = fetchItemByRID(QStringLiteral("{d0159c99-0d23-41fa-bb5f-tasktoremove}"), calendar2());
    QVERIFY(item.isValid());

    // WHEN
    auto storage = createStorage();
    auto job = storage->removeItem(item);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedItem = spy.takeFirst().at(0).value<Akonadi::Item>();
    QCOMPARE(notifiedItem.id(), item.id());

    auto parent = notifiedItem.parentCollection();
    while (parent != Akonadi::Collection::root()) {
        QVERIFY(parent.isValid());
        parent = parent.parentCollection();
    }
}

void AkonadiStorageTestBase::shouldNotifyItemChanged()
{
    // GIVEN

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemChanged);
    MonitorSpy monitorSpy(monitor.data());

    // A todo...
    KCalCore::Todo::Ptr todo(new KCalCore::Todo);
    todo->setSummary(QStringLiteral("summary"));
    todo->setDescription(QStringLiteral("content"));
    todo->setCompleted(false);
    todo->setDtStart(QDateTime(QDate(2013, 11, 24)));
    todo->setDtDue(QDateTime(QDate(2014, 03, 01)));

    // ... as payload of an existing item (if we trust the test data)...
    Akonadi::Item item = fetchItemByRID(QStringLiteral("{1d33862f-f274-4c67-ab6c-362d56521ff6}"), calendar2());
    QVERIFY(item.isValid());
    item.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
    item.setPayload<KCalCore::Todo::Ptr>(todo);
    item.addAttribute(new Akonadi::EntityDisplayAttribute);

    // WHEN
    auto storage = createStorage();
    auto job = storage->updateItem(item);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedItem = spy.takeFirst().at(0).value<Akonadi::Item>();
    QCOMPARE(notifiedItem.id(), item.id());
    QCOMPARE(*notifiedItem.payload<KCalCore::Todo::Ptr>(), *todo);
    QVERIFY(notifiedItem.hasAttribute<Akonadi::EntityDisplayAttribute>());

    auto parent = notifiedItem.parentCollection();
    while (parent != Akonadi::Collection::root()) {
        QVERIFY(parent.isValid());
        parent = parent.parentCollection();
    }
}

void AkonadiStorageTestBase::shouldReadDefaultCollectionFromSettings()
{
    // GIVEN

    // A storage implementation
    auto storage = createStorage();

    // WHEN
    Akonadi::StorageSettings::instance().setDefaultCollection(Akonadi::Collection(24));

    // THEN
    QCOMPARE(storage->defaultCollection(), Akonadi::Collection(24));
}

void AkonadiStorageTestBase::shouldUpdateItem()
{
    // GIVEN

    // A storage implementation
    auto storage = createStorage();

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemChanged);
    MonitorSpy monitorSpy(monitor.data());

    // A todo...
    KCalCore::Todo::Ptr todo(new KCalCore::Todo);
    todo->setSummary(QStringLiteral("new summary"));
    todo->setDescription(QStringLiteral("new content"));

    // ... as payload of an existing item (if we trust the test data)...
    Akonadi::Item item = fetchItemByRID(QStringLiteral("{1d33862f-f274-4c67-ab6c-362d56521ff4}"), calendar2());
    item.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
    item.setPayload<KCalCore::Todo::Ptr>(todo);

    // WHEN
    auto job = storage->updateItem(item);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedItem = spy.takeFirst().at(0).value<Akonadi::Item>();
    QCOMPARE(notifiedItem.id(), item.id());

    QCOMPARE(*notifiedItem.payload<KCalCore::Todo::Ptr>(), *todo);
}

void AkonadiStorageTestBase::shouldUseTransaction()
{
    // GIVEN
    auto storage = createStorage();

    Akonadi::Item item1 = fetchItemByRID(QStringLiteral("{0aa4dc30-a2c2-4e08-8241-033b3344debc}"), calendar1());
    QVERIFY(item1.isValid());
    Akonadi::Item item2 = fetchItemByRID(QStringLiteral("{5dc1aba7-eead-4254-ba7a-58e397de1179}"), calendar1());
    QVERIFY(item2.isValid());
    // create wrong item
    Akonadi::Item item3(10000);
    item3.setRemoteId(QStringLiteral("wrongId"));

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spyUpdated(monitor.data(), &Akonadi::MonitorInterface::itemChanged);
    MonitorSpy monitorSpy(monitor.data());

    // WHEN
    auto todo = item1.payload<KCalCore::Todo::Ptr>();
    todo->setSummary(QStringLiteral("Buy tomatoes"));

    todo = item2.payload<KCalCore::Todo::Ptr>();
    todo->setSummary(QStringLiteral("Buy chocolate"));

    auto transaction = storage->createTransaction();
    storage->updateItem(item1, transaction);
    storage->updateItem(item3, transaction); // this job should fail
    storage->updateItem(item2, transaction);
    QVERIFY(!transaction->exec());
    monitorSpy.waitForStableState();

    // THEN
    QCOMPARE(spyUpdated.size(), 0);
    auto job = storage->fetchItem(item1);
    AKVERIFYEXEC(job->kjob());
    QCOMPARE(job->items().size(), 1);
    item1 = job->items().at(0);

    job = storage->fetchItem(item2);
    AKVERIFYEXEC(job->kjob());
    QCOMPARE(job->items().size(), 1);
    item2 = job->items().at(0);

    QCOMPARE(item1.payload<KCalCore::Todo::Ptr>()->summary(), QStringLiteral("Buy kiwis"));
    QCOMPARE(item2.payload<KCalCore::Todo::Ptr>()->summary(), QStringLiteral("Buy cheese"));
}

void AkonadiStorageTestBase::shouldCreateItem()
{
    // GIVEN

    // A storage implementation
    auto storage = createStorage();

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemAdded);
    MonitorSpy monitorSpy(monitor.data());

    // A todo...
    KCalCore::Todo::Ptr todo(new KCalCore::Todo);
    todo->setSummary(QStringLiteral("summary"));
    todo->setDescription(QStringLiteral("content"));
    todo->setCompleted(false);
    todo->setDtStart(QDateTime(QDate(2013, 11, 24)));
    todo->setDtDue(QDateTime(QDate(2014, 03, 01)));

    // ... as payload of a new item
    Akonadi::Item item;
    item.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
    item.setPayload<KCalCore::Todo::Ptr>(todo);

    // WHEN
    auto job = storage->createItem(item, calendar2());
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedItem = spy.takeFirst().at(0).value<Akonadi::Item>();
    QCOMPARE(notifiedItem.parentCollection(), calendar2());
    QCOMPARE(*notifiedItem.payload<KCalCore::Todo::Ptr>(), *todo);
}

void AkonadiStorageTestBase::shouldRetrieveItem()
{
    // GIVEN
    auto storage = createStorage();
    Akonadi::Item findItem = fetchItemByRID(QStringLiteral("{7824df00-2fd6-47a4-8319-52659dc82005}"), calendar2());
    QVERIFY(findItem.isValid());

    // WHEN
    auto job = storage->fetchItem(findItem);
    AKVERIFYEXEC(job->kjob());

    // THEN
    auto items = job->items();
    QCOMPARE(items.size(), 1);

    const auto &item = items[0];

    QCOMPARE(item.id(), findItem.id());
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

void AkonadiStorageTestBase::shouldMoveItem()
{
    // GIVEN
    auto storage = createStorage();

    Akonadi::Item item = fetchItemByRID(QStringLiteral("{7824df00-2fd6-47a4-8319-52659dc82005}"), calendar2());
    QVERIFY(item.isValid());

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spyMoved(monitor.data(), &Akonadi::MonitorInterface::itemMoved);
    MonitorSpy monitorSpy(monitor.data());

    auto job = storage->moveItem(item, calendar1());
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spyMoved.isEmpty());

    QCOMPARE(spyMoved.size(), 1);
    auto movedItem = spyMoved.takeFirst().at(0).value<Akonadi::Item>();
    QCOMPARE(movedItem.id(), item.id());
}

void AkonadiStorageTestBase::shouldMoveItems()
{
    // GIVEN
    auto storage = createStorage();

    Akonadi::Item item = fetchItemByRID(QStringLiteral("{1d33862f-f274-4c67-ab6c-362d56521ff4}"), calendar2());
    QVERIFY(item.isValid());
    Akonadi::Item::List list;
    list << item;

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spyMoved(monitor.data(), &Akonadi::MonitorInterface::itemMoved);
    MonitorSpy monitorSpy(monitor.data());

    auto job = storage->moveItems(list, calendar1());
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spyMoved.isEmpty());

    QCOMPARE(spyMoved.size(), 1);
    auto movedItem = spyMoved.takeFirst().at(0).value<Akonadi::Item>();
    QCOMPARE(movedItem.id(), item.id());
}

void AkonadiStorageTestBase::shouldDeleteItem()
{
    //GIVEN
    auto storage = createStorage();

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemRemoved);
    MonitorSpy monitorSpy(monitor.data());

    // An existing item (if we trust the test data)
    Akonadi::Item item = fetchItemByRID(QStringLiteral("{0aa4dc30-a2c2-4e08-8241-033b3344debc}"), calendar1());
    QVERIFY(item.isValid());

    //When
    auto job = storage->removeItem(item);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedItem = spy.takeFirst().at(0).value<Akonadi::Item>();
    QCOMPARE(notifiedItem.id(), item.id());
}

void AkonadiStorageTestBase::shouldDeleteItems()
{
    //GIVEN
    auto storage = createStorage();

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemRemoved);
    MonitorSpy monitorSpy(monitor.data());

    // An existing item (if we trust the test data)
    Akonadi::Item item = fetchItemByRID(QStringLiteral("{6c7bf5b9-4136-4203-9f45-54e32ea0eacb}"), calendar1());
    QVERIFY(item.isValid());
    Akonadi::Item item2 = fetchItemByRID(QStringLiteral("{83cf0b15-8d61-436b-97ae-4bd88fb2fef9}"), calendar1());
    QVERIFY(item2.isValid());

    Akonadi::Item::List list;
    list << item << item2;

    //When
    auto job = storage->removeItems(list);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 2);
    auto notifiedItem = spy.takeFirst().at(0).value<Akonadi::Item>();
    QCOMPARE(notifiedItem.id(), item.id());
    notifiedItem = spy.takeFirst().at(0).value<Akonadi::Item>();
    QCOMPARE(notifiedItem.id(), item2.id());
}

void AkonadiStorageTestBase::shouldUpdateCollection()
{
    // GIVEN

    // A storage implementation
    auto storage = createStorage();

    // An existing collection
    Akonadi::Collection collection = calendar2();

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy changeSpy(monitor.data(), &Akonadi::MonitorInterface::collectionChanged);
    QSignalSpy selectionSpy(monitor.data(), &Akonadi::MonitorInterface::collectionSelectionChanged);
    MonitorSpy monitorSpy(monitor.data());

    // WHEN
    auto attr = new Akonadi::EntityDisplayAttribute;
    attr->setDisplayName(QStringLiteral("Foo"));
    collection.addAttribute(attr);
    auto job = storage->updateCollection(collection);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!changeSpy.isEmpty());

    // THEN
    QCOMPARE(changeSpy.size(), 1);
    QCOMPARE(selectionSpy.size(), 0);
    auto notifiedCollection = changeSpy.takeFirst().at(0).value<Akonadi::Collection>();
    QCOMPARE(notifiedCollection.id(), collection.id());
    QVERIFY(notifiedCollection.hasAttribute<Akonadi::EntityDisplayAttribute>());
    QCOMPARE(notifiedCollection.attribute<Akonadi::EntityDisplayAttribute>()->displayName(), attr->displayName());
}

void AkonadiStorageTestBase::shouldNotifyCollectionTimestampChanges()
{
    // GIVEN

    // A storage implementation
    auto storage = createStorage();

    // An existing collection
    Akonadi::Collection collection = calendar2();

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy changeSpy(monitor.data(), &Akonadi::MonitorInterface::collectionChanged);
    MonitorSpy monitorSpy(monitor.data());

    // WHEN
    collection.attribute<Akonadi::TimestampAttribute>(Akonadi::Collection::AddIfMissing)->refreshTimestamp();
    auto job = storage->updateCollection(collection);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!changeSpy.isEmpty());

    // THEN
    QCOMPARE(changeSpy.size(), 1);

    auto notifiedCollection = changeSpy.takeFirst().at(0).value<Akonadi::Collection>();
    QCOMPARE(notifiedCollection.id(), collection.id());
    QVERIFY(notifiedCollection.hasAttribute<Akonadi::TimestampAttribute>());
}

void AkonadiStorageTestBase::shouldNotifyCollectionSelectionChanges()
{
    // GIVEN

    // A storage implementation
    auto storage = createStorage();

    // An existing collection
    Akonadi::Collection collection = calendar2();

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy changeSpy(monitor.data(), &Akonadi::MonitorInterface::collectionChanged);
    QSignalSpy selectionSpy(monitor.data(), &Akonadi::MonitorInterface::collectionSelectionChanged);
    MonitorSpy monitorSpy(monitor.data());

    // WHEN
    auto attr = new Akonadi::ApplicationSelectedAttribute;
    attr->setSelected(false);
    collection.addAttribute(attr);
    auto job = storage->updateCollection(collection);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!changeSpy.isEmpty());

    // THEN
    QCOMPARE(changeSpy.size(), 1);
    QCOMPARE(selectionSpy.size(), 1);

    auto notifiedCollection = changeSpy.takeFirst().at(0).value<Akonadi::Collection>();
    QCOMPARE(notifiedCollection.id(), collection.id());
    QVERIFY(notifiedCollection.hasAttribute<Akonadi::ApplicationSelectedAttribute>());
    QVERIFY(!notifiedCollection.attribute<Akonadi::ApplicationSelectedAttribute>()->isSelected());

    notifiedCollection = selectionSpy.takeFirst().at(0).value<Akonadi::Collection>();
    QCOMPARE(notifiedCollection.id(), collection.id());
    QVERIFY(notifiedCollection.hasAttribute<Akonadi::ApplicationSelectedAttribute>());
    QVERIFY(!notifiedCollection.attribute<Akonadi::ApplicationSelectedAttribute>()->isSelected());
}

Akonadi::Item AkonadiStorageTestBase::fetchItemByRID(const QString &remoteId, const Akonadi::Collection &collection)
{
    Akonadi::Item item;
    item.setRemoteId(remoteId);

    auto job = createStorage()->fetchItem(item);
    job->setCollection(collection);
    if (!job->kjob()->exec()) {
        qWarning() << job->kjob()->errorString();
        return Akonadi::Item();
    }

    if (job->items().count() != 1) {
        qWarning() << "Received unexpected amount of items: " << job->items().count();
        return Akonadi::Item();
    }

    return job->items().at(0);
}

Akonadi::Collection AkonadiStorageTestBase::fetchCollectionByRID(const QString &remoteId)
{
    Akonadi::Collection collection;
    collection.setRemoteId(remoteId);

    auto job = createStorage()->fetchCollections(collection, Akonadi::StorageInterface::Base);
    job->setResource(QStringLiteral("akonadi_knut_resource_0"));
    if (!job->kjob()->exec()) {
        qWarning() << job->kjob()->errorString() << remoteId;
        return Akonadi::Collection();
    }

    if (job->collections().count() != 1) {
        qWarning() << "Received unexpected amount of collections: " << job->collections().count();
        return Akonadi::Collection();
    }

    return job->collections().at(0);
}

Akonadi::Collection AkonadiStorageTestBase::calendar1()
{
    return fetchCollectionByRID(QStringLiteral("{cdc229c7-a9b5-4d37-989d-a28e372be2a9}"));
}

Akonadi::Collection AkonadiStorageTestBase::calendar2()
{
    return fetchCollectionByRID(QStringLiteral("{e682b8b5-b67c-4538-8689-6166f64177f0}"));
}

Akonadi::Collection AkonadiStorageTestBase::emails()
{
    return fetchCollectionByRID(QStringLiteral("{14096930-7bfe-46ca-8fba-7c04d3b62ec8}"));
}

