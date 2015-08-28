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

#include "akonadi/qtest_akonadi.h"

#include <Akonadi/EntityDisplayAttribute>

#include "akonadi/akonadiapplicationselectedattribute.h"
#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadicollectionsearchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonadimonitorimpl.h"
#include "akonadi/akonadistorage.h"
#include "akonadi/akonadistoragesettings.h"
#include "akonadi/akonaditagfetchjobinterface.h"
#include "akonadi/akonaditimestampattribute.h"

using namespace Testlib;

AkonadiStorageTestBase::AkonadiStorageTestBase(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<Akonadi::Collection>();
    qRegisterMetaType<Akonadi::Item>();
    qRegisterMetaType<Akonadi::Tag>();
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
    QTest::addColumn<int>("contentTypes");
    QTest::addColumn<bool>("referenceCalendar1");
    QTest::addColumn<bool>("enableCalendar1");

    QTest::newRow("all") << Akonadi::Collection::root()
                         << QStringList({ "Calendar1", "Calendar2", "Calendar3", "Change me!", "Destroy me!", "Notes" })
                         << Akonadi::Storage::Recursive
                         << int(Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                         << false << true;

    QTest::newRow("include referenced") << Akonadi::Collection::root()
                                        << QStringList({ "Calendar1", "Calendar2", "Calendar3", "Change me!", "Destroy me!", "Notes" })
                                        << Akonadi::Storage::Recursive
                                        << int(Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                        << true << false;

    QTest::newRow("include referenced + enabled") << Akonadi::Collection::root()
                                                  << QStringList({ "Calendar1", "Calendar2", "Calendar3", "Change me!", "Destroy me!", "Notes" })
                                                  << Akonadi::Storage::Recursive
                                                  << int(Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                                  << true << true;

    QTest::newRow("exclude !referenced + !enabled") << Akonadi::Collection::root()
                                                    << QStringList({ "Change me!", "Destroy me!", "Notes" })
                                                    << Akonadi::Storage::Recursive
                                                    << int(Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                                    << false << false;

    QTest::newRow("notes") << Akonadi::Collection::root()
                           << QStringList({ "Notes" })
                           << Akonadi::Storage::Recursive
                           << int(Akonadi::StorageInterface::Notes)
                           << false << true;

    QTest::newRow("tasks") << Akonadi::Collection::root()
                           << QStringList({ "Calendar1", "Calendar2", "Calendar3", "Change me!", "Destroy me!" })
                           << Akonadi::Storage::Recursive
                           << int(Akonadi::StorageInterface::Tasks)
                           << false << true;

    QTest::newRow("base type") << calendar2()
                               << QStringList({"Calendar2"})
                               << Akonadi::Storage::Base
                               << int(Akonadi::StorageInterface::Tasks)
                               << false << true;

    QTest::newRow("firstLevel type") << calendar1()
                                     << QStringList({"Calendar2"})
                                     << Akonadi::Storage::FirstLevel
                                     << int(Akonadi::StorageInterface::Tasks)
                                     << false << true;

    QTest::newRow("recursive type") << calendar1()
                                    << QStringList({"Calendar2", "Calendar3"})
                                    << Akonadi::Storage::Recursive
                                    << int(Akonadi::StorageInterface::Tasks)
                                    << false << true;
}

void AkonadiStorageTestBase::shouldListCollections()
{
    // GIVEN
    QFETCH(Akonadi::Collection, collection);
    QFETCH(QStringList, expectedNames);
    QFETCH(Akonadi::StorageInterface::FetchDepth, depth);
    QFETCH(int, contentTypes);
    QFETCH(bool, referenceCalendar1);
    QFETCH(bool, enableCalendar1);

    auto storage = createStorage();

    // Default is not referenced and enabled
    // no need to feedle with the collection in that case
    if (referenceCalendar1 || !enableCalendar1) {
        Akonadi::Collection cal1 = calendar1();
        cal1.setReferenced(referenceCalendar1);
        cal1.setEnabled(enableCalendar1);
        auto update = storage->updateCollection(cal1);
        AKVERIFYEXEC(update);
    }


    // WHEN
    auto job = storage->fetchCollections(collection, depth,
                                         Akonadi::StorageInterface::FetchContentTypes(contentTypes));
    AKVERIFYEXEC(job->kjob());

    // THEN
    auto collections = job->collections();
    QStringList collectionNames;
    for (const auto &collection : collections) {
        collectionNames << collection.name();
    }
    collectionNames.sort();

    // Restore proper DB state
    if (referenceCalendar1 || !enableCalendar1) {
        Akonadi::Collection cal1 = calendar1();
        cal1.setReferenced(false);
        cal1.setEnabled(true);
        auto update = storage->updateCollection(cal1);
        AKVERIFYEXEC(update);
    }

    QCOMPARE(collectionNames, expectedNames);
}

void AkonadiStorageTestBase::shouldRetrieveAllCollectionAncestors()
{
    // GIVEN
    auto storage = createStorage();

    // WHEN
    auto job = storage->fetchCollections(Akonadi::Collection::root(),
                                         Akonadi::Storage::Recursive,
                                         Akonadi::Storage::Tasks|Akonadi::Storage::Notes);
    AKVERIFYEXEC(job->kjob());

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

void AkonadiStorageTestBase::shouldListFullItemsInACollection()
{
    // GIVEN
    auto storage = createStorage();
    const QStringList expectedRemoteIds = { "{1d33862f-f274-4c67-ab6c-362d56521ff4}",
                                            "{1d33862f-f274-4c67-ab6c-362d56521ff5}",
                                            "{1d33862f-f274-4c67-ab6c-362d56521ff6}",
                                            "{7824df00-2fd6-47a4-8319-52659dc82005}",
                                            "{7824df00-2fd6-47a4-8319-52659dc82006}" };

    // WHEN
    auto job = storage->fetchItems(calendar2());
    AKVERIFYEXEC(job->kjob());

    // THEN
    auto items = job->items();
    QStringList itemRemoteIds;
    for (const auto &item : items) {
        itemRemoteIds << item.remoteId();
        QVERIFY(item.loadedPayloadParts().contains(Akonadi::Item::FullPayload));
        QVERIFY(!item.attributes().isEmpty());
        QVERIFY(item.modificationTime().isValid());
        QVERIFY(!item.flags().isEmpty());

        Akonadi::Tag::List tags = item.tags();
        QVERIFY(!item.tags().isEmpty());
        for (const auto &tag : tags) {
            QVERIFY(tag.isValid());
            QVERIFY(!tag.name().isEmpty());
            QVERIFY(!tag.type().isEmpty());
        }

        auto parent = item.parentCollection();
        while (parent != Akonadi::Collection::root()) {
            QVERIFY(parent.isValid());
            parent = parent.parentCollection();
        }
    }
    itemRemoteIds.sort();

    QCOMPARE(itemRemoteIds, expectedRemoteIds);
}

void AkonadiStorageTestBase::shouldListTags()
{
    // GIVEN
    auto storage = createStorage();
    const QStringList expectedGids = { "change-me",
                                       "delete-me",
                                       "errands-context",
                                       "online-context",
                                       "philosophy-tag",
                                       "physics-tag" };

    // WHEN
    auto job = storage->fetchTags();
    AKVERIFYEXEC(job->kjob());

    // THEN
    auto tags = job->tags();
    QStringList tagGids;
    for (const Akonadi::Tag &tag : tags) {
        tagGids << tag.gid();
        QVERIFY(!tag.name().isEmpty());
        QVERIFY(!tag.type().isEmpty());
    }
    tagGids.sort();

    QCOMPARE(tagGids, expectedGids);
}

void AkonadiStorageTestBase::shouldListItemsAssociatedWithTag()
{
    // GIVEN
    auto storage = createStorage();
    Akonadi::Tag tag = fetchTagByGID("errands-context");
    const QStringList expectedRemoteIds = { "{1d33862f-f274-4c67-ab6c-362d56521ff4}",
                                            "{7824df00-2fd6-47a4-8319-52659dc82005}"
                                          };

    // WHEN
    auto job = storage->fetchTagItems(tag);
    AKVERIFYEXEC(job->kjob());

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

void AkonadiStorageTestBase::shouldNotifyCollectionAdded()
{
    // GIVEN

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(collectionAdded(Akonadi::Collection)));
    MonitorSpy monitorSpy(monitor.data());

    // A collection
    Akonadi::Collection collection;
    collection.setParentCollection(calendar2());
    collection.setName("Foo!");
    collection.setContentMimeTypes(QStringList() << "application/x-vnd.akonadi.calendar.todo");

    // WHEN
    auto storage = createStorage();
    auto job = storage->createCollection(collection);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

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

void AkonadiStorageTestBase::shouldNotifyCollectionRemoved()
{
    // GIVEN

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(collectionRemoved(Akonadi::Collection)));
    MonitorSpy monitorSpy(monitor.data());

    // An existing item (if we trust the test data)
    Akonadi::Collection collection = fetchCollectionByRID("{1f78b360-a01b-4785-9187-75450190342c}");
    QVERIFY(collection.isValid());

    // WHEN
    auto storage = createStorage();
    auto job = storage->removeCollection(collection);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedCollection= spy.takeFirst().takeFirst().value<Akonadi::Collection>();
    QCOMPARE(notifiedCollection.id(), collection.id());
}

void AkonadiStorageTestBase::shouldNotifyCollectionChanged()
{
    // GIVEN

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(collectionChanged(Akonadi::Collection)));
    MonitorSpy monitorSpy(monitor.data());

    // A colection with an existing id (if we trust the test data)
    Akonadi::Collection collection = fetchCollectionByRID("{28ef9f03-4ebc-4e33-970f-f379775894f9}");
    QVERIFY(collection.isValid());
    collection.setName("Bar!");

    // WHEN
    auto storage = createStorage();
    auto job = storage->updateCollection(collection);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

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

void AkonadiStorageTestBase::shouldNotifyItemAdded()
{
    // GIVEN

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(itemAdded(Akonadi::Item)));
    MonitorSpy monitorSpy(monitor.data());

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
    auto storage = createStorage();
    auto job = storage->createItem(item, calendar2());
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

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

void AkonadiStorageTestBase::shouldNotifyItemRemoved()
{
    // GIVEN

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(itemRemoved(Akonadi::Item)));
    MonitorSpy monitorSpy(monitor.data());

    const Akonadi::Collection notesCol = fetchCollectionByRID("{f5e3f1be-b998-4c56-aa3d-e3a6e7e5493a}");
    Akonadi::Item item = fetchItemByRID("{d0159c99-0d23-41fa-bb5f-436570140f8b}", notesCol);
    QVERIFY(item.isValid());

    // WHEN
    auto storage = createStorage();
    auto job = storage->removeItem(item);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedItem = spy.takeFirst().takeFirst().value<Akonadi::Item>();
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
    QSignalSpy spy(monitor.data(), SIGNAL(itemChanged(Akonadi::Item)));
    MonitorSpy monitorSpy(monitor.data());

    // A todo...
    KCalCore::Todo::Ptr todo(new KCalCore::Todo);
    todo->setSummary("summary");
    todo->setDescription("content");
    todo->setCompleted(false);
    todo->setDtStart(KDateTime(QDate(2013, 11, 24)));
    todo->setDtDue(KDateTime(QDate(2014, 03, 01)));

    // ... as payload of an existing item (if we trust the test data)...
    Akonadi::Item item = fetchItemByRID("{1d33862f-f274-4c67-ab6c-362d56521ff6}", calendar2());
    QVERIFY(item.isValid());
    item.setMimeType("application/x-vnd.akonadi.calendar.todo");
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
    auto notifiedItem = spy.takeFirst().takeFirst().value<Akonadi::Item>();
    QCOMPARE(notifiedItem.id(), item.id());
    QCOMPARE(*notifiedItem.payload<KCalCore::Todo::Ptr>(), *todo);
    QVERIFY(notifiedItem.hasAttribute<Akonadi::EntityDisplayAttribute>());

    auto parent = notifiedItem.parentCollection();
    while (parent != Akonadi::Collection::root()) {
        QVERIFY(parent.isValid());
        parent = parent.parentCollection();
    }
}

void AkonadiStorageTestBase::shouldNotifyItemTagAdded()
{
    // GIVEN

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(itemChanged(Akonadi::Item)));
    MonitorSpy monitorSpy(monitor.data());

    // An existing item (if we trust the test data)...
    Akonadi::Item item = fetchItemByRID("{1d33862f-f274-4c67-ab6c-362d56521ff5}", calendar2());
    QVERIFY(item.isValid());
    item.setMimeType("application/x-vnd.akonadi.calendar.todo");

    // An existing tag (if we trust the test data)
    Akonadi::Tag tag(5);

    // WHEN
    item.setTag(tag);
    auto storage = createStorage();
    auto job = storage->updateItem(item);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedItem = spy.takeFirst().takeFirst().value<Akonadi::Item>();
    QCOMPARE(notifiedItem.id(), item.id());
    QVERIFY(notifiedItem.hasPayload<KCalCore::Todo::Ptr>());

    Akonadi::Tag::List notifiedTags = notifiedItem.tags();

    QVERIFY(notifiedTags.contains(tag));
    for (const auto &tag : notifiedTags) {
        QVERIFY(tag.isValid());
        QVERIFY(!tag.name().isEmpty());
        QVERIFY(!tag.type().isEmpty());
    }

    auto parent = notifiedItem.parentCollection();
    while (parent != Akonadi::Collection::root()) {
        QVERIFY(parent.isValid());
        parent = parent.parentCollection();
    }
}

void AkonadiStorageTestBase::shouldNotifyItemTagRemoved() // aka dissociate
{
    // GIVEN
    auto storage = createStorage();
    Akonadi::Tag tag = fetchTagByGID("philosophy-tag");
    const QString expectedRemoteIds = {"{7824df00-2fd6-47a4-8319-52659dc82006}"};
    auto job = storage->fetchTagItems(tag);
    AKVERIFYEXEC(job->kjob());

    auto item = job->items().first();
    QCOMPARE(item.remoteId(), expectedRemoteIds);

    QVERIFY(item.loadedPayloadParts().contains(Akonadi::Item::FullPayload));
    QVERIFY(!item.attributes().isEmpty());
    QVERIFY(item.modificationTime().isValid());
    QVERIFY(!item.flags().isEmpty());
    QVERIFY(!item.tags().isEmpty());

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(itemChanged(Akonadi::Item)));
    MonitorSpy monitorSpy(monitor.data());

    // WHEN
    item.clearTag(tag);
    auto jobUpdate = storage->updateItem(item);
    AKVERIFYEXEC(jobUpdate);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedItem = spy.takeFirst().takeFirst().value<Akonadi::Item>();
    QCOMPARE(notifiedItem.id(), item.id());
    QVERIFY(!notifiedItem.tags().contains(tag));
}

void AkonadiStorageTestBase::shouldNotifyTagAdded()
{
    // GIVEN

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(tagAdded(Akonadi::Tag)));
    MonitorSpy monitorSpy(monitor.data());

    // A tag
    Akonadi::Tag tag;
    tag.setGid("gid");
    tag.setName("name");
    tag.setType("type");

    // WHEN
    auto storage = createStorage();
    auto job = storage->createTag(tag);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedTag = spy.takeFirst().takeFirst().value<Akonadi::Tag>();
    QCOMPARE(notifiedTag.gid(), tag.gid());
    QCOMPARE(notifiedTag.name(), tag.name());
    QCOMPARE(notifiedTag.type(), tag.type());
}

void AkonadiStorageTestBase::shouldNotifyTagRemoved()
{
    // GIVEN

    // An existing tag (if we trust the test data) connected to an existing item tagged to it
    auto storage = createStorage();
    Akonadi::Tag tag = fetchTagByGID("delete-me");
    // NOTE : this item was linked to the delete-me tag during test time
    const QString expectedRemoteIds = {"{1d33862f-f274-4c67-ab6c-362d56521ff5}"};
    auto job = storage->fetchTagItems(tag);
    AKVERIFYEXEC(job->kjob());

    QCOMPARE(job->items().size(), 1);
    auto itemTagged = job->items().first();
    QCOMPARE(itemTagged.remoteId(), expectedRemoteIds);

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(tagRemoved(Akonadi::Tag)));
    QSignalSpy spyItemChanged(monitor.data(), SIGNAL(itemChanged(Akonadi::Item)));
    MonitorSpy monitorSpy(monitor.data());

    // WHEN
    auto jobDelete = storage->removeTag(tag);
    AKVERIFYEXEC(jobDelete);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());
    QTRY_VERIFY(!spyItemChanged.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedTag = spy.takeFirst().takeFirst().value<Akonadi::Tag>();
    QCOMPARE(notifiedTag.id(), tag.id());

    QCOMPARE(spyItemChanged.size(), 1);
    auto notifiedItem = spyItemChanged.takeFirst().takeFirst().value<Akonadi::Item>();
    QCOMPARE(notifiedItem.id(), itemTagged.id());
}

void AkonadiStorageTestBase::shouldNotifyTagChanged()
{
    // GIVEN

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(tagChanged(Akonadi::Tag)));
    MonitorSpy monitorSpy(monitor.data());

    // An existing tag (if we trust the test data)
    Akonadi::Tag tag(6);
    tag.setName("Oh it changed!");

    // WHEN
    auto storage = createStorage();
    auto job = storage->updateTag(tag);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedTag = spy.takeFirst().takeFirst().value<Akonadi::Tag>();
    QCOMPARE(notifiedTag.id(), tag.id());
    QCOMPARE(notifiedTag.name(), tag.name());
}

void AkonadiStorageTestBase::shouldReadDefaultNoteCollectionFromSettings()
{
    // GIVEN

    // A storage implementation
    auto storage = createStorage();

    // WHEN
    Akonadi::StorageSettings::instance().setDefaultNoteCollection(Akonadi::Collection(24));

    // THEN
    QCOMPARE(storage->defaultNoteCollection(), Akonadi::Collection(24));
}

void AkonadiStorageTestBase::shouldReadDefaultTaskCollectionFromSettings()
{
    // GIVEN

    // A storage implementation
    auto storage = createStorage();

    // WHEN
    Akonadi::StorageSettings::instance().setDefaultTaskCollection(Akonadi::Collection(24));

    // THEN
    QCOMPARE(storage->defaultTaskCollection(), Akonadi::Collection(24));
}

void AkonadiStorageTestBase::shouldUpdateItem()
{
    // GIVEN

    // A storage implementation
    auto storage = createStorage();

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(itemChanged(Akonadi::Item)));
    MonitorSpy monitorSpy(monitor.data());

    // A todo...
    KCalCore::Todo::Ptr todo(new KCalCore::Todo);
    todo->setSummary("new summary");
    todo->setDescription("new content");

    // ... as payload of an existing item (if we trust the test data)...
    Akonadi::Item item = fetchItemByRID("{1d33862f-f274-4c67-ab6c-362d56521ff4}", calendar2());
    item.setMimeType("application/x-vnd.akonadi.calendar.todo");
    item.setPayload<KCalCore::Todo::Ptr>(todo);

    // WHEN
    auto job = storage->updateItem(item);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedItem = spy.takeFirst().takeFirst().value<Akonadi::Item>();
    QCOMPARE(notifiedItem.id(), item.id());
    QCOMPARE(*notifiedItem.payload<KCalCore::Todo::Ptr>(), *todo);
}

void AkonadiStorageTestBase::shouldUseTransaction()
{
    // GIVEN
    auto storage = createStorage();

    Akonadi::Item item1 = fetchItemByRID("{0aa4dc30-a2c2-4e08-8241-033b3344debc}", calendar1());
    QVERIFY(item1.isValid());
    Akonadi::Item item2 = fetchItemByRID("{5dc1aba7-eead-4254-ba7a-58e397de1179}", calendar1());
    QVERIFY(item2.isValid());
    // create wrong item
    Akonadi::Item item3(10000);
    item3.setRemoteId("wrongId");

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spyUpdated(monitor.data(), SIGNAL(itemChanged(Akonadi::Item)));
    MonitorSpy monitorSpy(monitor.data());

    // WHEN
    auto todo = item1.payload<KCalCore::Todo::Ptr>();
    todo->setSummary("Buy tomatoes");

    todo = item2.payload<KCalCore::Todo::Ptr>();
    todo->setSummary("Buy chocolate");

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
    item1 = job->items()[0];

    job = storage->fetchItem(item2);
    AKVERIFYEXEC(job->kjob());
    QCOMPARE(job->items().size(), 1);
    item2 = job->items()[0];

    QCOMPARE(item1.payload<KCalCore::Todo::Ptr>()->summary(), QString("Buy kiwis"));
    QCOMPARE(item2.payload<KCalCore::Todo::Ptr>()->summary(), QString("Buy cheese"));
}

void AkonadiStorageTestBase::shouldCreateItem()
{
    // GIVEN

    // A storage implementation
    auto storage = createStorage();

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(itemAdded(Akonadi::Item)));
    MonitorSpy monitorSpy(monitor.data());

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
    auto job = storage->createItem(item, calendar2());
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedItem = spy.takeFirst().takeFirst().value<Akonadi::Item>();
    QCOMPARE(notifiedItem.parentCollection(), calendar2());
    QCOMPARE(*notifiedItem.payload<KCalCore::Todo::Ptr>(), *todo);
}

void AkonadiStorageTestBase::shouldRetrieveItem()
{
    // GIVEN
    auto storage = createStorage();
    Akonadi::Item findItem = fetchItemByRID("{7824df00-2fd6-47a4-8319-52659dc82005}", calendar2());
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

    Akonadi::Item item = fetchItemByRID("{7824df00-2fd6-47a4-8319-52659dc82005}", calendar2());
    QVERIFY(item.isValid());

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spyMoved(monitor.data(), SIGNAL(itemMoved(Akonadi::Item)));
    MonitorSpy monitorSpy(monitor.data());

    auto job = storage->moveItem(item, calendar1());
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spyMoved.isEmpty());

    QCOMPARE(spyMoved.size(), 1);
    auto movedItem = spyMoved.takeFirst().takeFirst().value<Akonadi::Item>();
    QCOMPARE(movedItem.id(), item.id());
}

void AkonadiStorageTestBase::shouldMoveItems()
{
    // GIVEN
    auto storage = createStorage();

    Akonadi::Item item = fetchItemByRID("{1d33862f-f274-4c67-ab6c-362d56521ff4}", calendar2());
    QVERIFY(item.isValid());
    Akonadi::Item::List list;
    list << item;

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spyMoved(monitor.data(), SIGNAL(itemMoved(Akonadi::Item)));
    MonitorSpy monitorSpy(monitor.data());

    auto job = storage->moveItems(list, calendar1());
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spyMoved.isEmpty());

    QCOMPARE(spyMoved.size(), 1);
    auto movedItem = spyMoved.takeFirst().takeFirst().value<Akonadi::Item>();
    QCOMPARE(movedItem.id(), item.id());
}

void AkonadiStorageTestBase::shouldDeleteItem()
{
    //GIVEN
    auto storage = createStorage();

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(itemRemoved(Akonadi::Item)));
    MonitorSpy monitorSpy(monitor.data());

    // An existing item (if we trust the test data)
    Akonadi::Item item = fetchItemByRID("{0aa4dc30-a2c2-4e08-8241-033b3344debc}", calendar1());
    QVERIFY(item.isValid());

    //When
    auto job = storage->removeItem(item);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedItem = spy.takeFirst().takeFirst().value<Akonadi::Item>();
    QCOMPARE(notifiedItem.id(), item.id());
}

void AkonadiStorageTestBase::shouldDeleteItems()
{
    //GIVEN
    auto storage = createStorage();

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(itemRemoved(Akonadi::Item)));
    MonitorSpy monitorSpy(monitor.data());

    // An existing item (if we trust the test data)
    Akonadi::Item item = fetchItemByRID("{6c7bf5b9-4136-4203-9f45-54e32ea0eacb}", calendar1());
    QVERIFY(item.isValid());
    Akonadi::Item item2 = fetchItemByRID("{83cf0b15-8d61-436b-97ae-4bd88fb2fef9}", calendar1());
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
    auto notifiedItem = spy.takeFirst().takeFirst().value<Akonadi::Item>();
    QCOMPARE(notifiedItem.id(), item.id());
    notifiedItem = spy.takeFirst().takeFirst().value<Akonadi::Item>();
    QCOMPARE(notifiedItem.id(), item2.id());
}

void AkonadiStorageTestBase::shouldCreateTag()
{
    // GIVEN

    // A storage implementation
    auto storage = createStorage();

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(tagAdded(Akonadi::Tag)));
    MonitorSpy monitorSpy(monitor.data());

    // A tag
    Akonadi::Tag tag;
    QString name = "Tag42";
    const QByteArray type = QByteArray("Zanshin-Context");
    const QByteArray gid = QByteArray(name.toLatin1());
    tag.setName(name);
    tag.setType(QByteArray("Zanshin-Context"));
    tag.setGid(gid);

    // WHEN
    auto job = storage->createTag(tag);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedTag = spy.takeFirst().takeFirst().value<Akonadi::Tag>();
    QCOMPARE(notifiedTag.name(), name);
    QCOMPARE(notifiedTag.type(), type);
    QCOMPARE(notifiedTag.gid(), gid);
}

void AkonadiStorageTestBase::shouldRemoveTag()
{

    // GIVEN
    auto storage = createStorage();

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(tagRemoved(Akonadi::Tag)));
    MonitorSpy monitorSpy(monitor.data());

    // An existing tag
    Akonadi::Tag tag = fetchTagByGID("errands-context");

    // WHEN
    auto job = storage->removeTag(tag);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedTag = spy.takeFirst().takeFirst().value<Akonadi::Tag>();
    QCOMPARE(notifiedTag.id(), tag.id());
}

void AkonadiStorageTestBase::shouldUpdateTag()
{
    // GIVEN
    auto storage = createStorage();

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy spy(monitor.data(), SIGNAL(tagChanged(Akonadi::Tag)));
    MonitorSpy monitorSpy(monitor.data());

    // An existing tag
    Akonadi::Tag tag = fetchTagByGID("change-me");

    // WHEN
    auto job = storage->updateTag(tag);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!spy.isEmpty());

    // THEN
    QCOMPARE(spy.size(), 1);
    auto notifiedTag = spy.takeFirst().takeFirst().value<Akonadi::Tag>();
    QCOMPARE(notifiedTag.id(), tag.id());
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
    QSignalSpy changeSpy(monitor.data(), SIGNAL(collectionChanged(Akonadi::Collection)));
    QSignalSpy selectionSpy(monitor.data(), SIGNAL(collectionSelectionChanged(Akonadi::Collection)));
    MonitorSpy monitorSpy(monitor.data());

    // WHEN
    auto attr = new Akonadi::EntityDisplayAttribute;
    attr->setDisplayName("Foo");
    collection.addAttribute(attr);
    auto job = storage->updateCollection(collection);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!changeSpy.isEmpty());

    // THEN
    QCOMPARE(changeSpy.size(), 1);
    QCOMPARE(selectionSpy.size(), 0);
    auto notifiedCollection = changeSpy.takeFirst().takeFirst().value<Akonadi::Collection>();
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
    QSignalSpy changeSpy(monitor.data(), SIGNAL(collectionChanged(Akonadi::Collection)));
    MonitorSpy monitorSpy(monitor.data());

    // WHEN
    collection.attribute<Akonadi::TimestampAttribute>(Akonadi::Collection::AddIfMissing)->refreshTimestamp();
    auto job = storage->updateCollection(collection);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!changeSpy.isEmpty());

    // THEN
    QCOMPARE(changeSpy.size(), 1);

    auto notifiedCollection = changeSpy.takeFirst().takeFirst().value<Akonadi::Collection>();
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
    QSignalSpy changeSpy(monitor.data(), SIGNAL(collectionChanged(Akonadi::Collection)));
    QSignalSpy selectionSpy(monitor.data(), SIGNAL(collectionSelectionChanged(Akonadi::Collection)));
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

    auto notifiedCollection = changeSpy.takeFirst().takeFirst().value<Akonadi::Collection>();
    QCOMPARE(notifiedCollection.id(), collection.id());
    QVERIFY(notifiedCollection.hasAttribute<Akonadi::ApplicationSelectedAttribute>());
    QVERIFY(!notifiedCollection.attribute<Akonadi::ApplicationSelectedAttribute>()->isSelected());

    notifiedCollection = selectionSpy.takeFirst().takeFirst().value<Akonadi::Collection>();
    QCOMPARE(notifiedCollection.id(), collection.id());
    QVERIFY(notifiedCollection.hasAttribute<Akonadi::ApplicationSelectedAttribute>());
    QVERIFY(!notifiedCollection.attribute<Akonadi::ApplicationSelectedAttribute>()->isSelected());
}

void AkonadiStorageTestBase::shouldNotNotifyCollectionSelectionChangesForIrrelevantCollections()
{
    // GIVEN

    // A storage implementation
    auto storage = createStorage();

    // An existing collection
    Akonadi::Collection collection = emails();

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy changeSpy(monitor.data(), SIGNAL(collectionChanged(Akonadi::Collection)));
    QSignalSpy selectionSpy(monitor.data(), SIGNAL(collectionSelectionChanged(Akonadi::Collection)));
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
    QVERIFY(selectionSpy.isEmpty());
}

void AkonadiStorageTestBase::shouldNotifyCollectionSubscriptionChanges_data()
{
    QTest::addColumn<bool>("isEnabled");
    QTest::addColumn<bool>("isReferenced");

    QTest::newRow("enabled and !referenced") << true << false;
    // Fails randomly due to an akonadi bug...
    //QTest::newRow("!enabled and referenced") << false << true;
    QTest::newRow("!enabled and !referenced") << false << false;
    QTest::newRow("!enabled and referenced (again)") << false << true;
    QTest::newRow("enabled and !referenced (again)") << true << false;
}

void AkonadiStorageTestBase::shouldNotifyCollectionSubscriptionChanges()
{
    // GIVEN
    QFETCH(bool, isEnabled);
    QFETCH(bool, isReferenced);

    // A storage implementation
    auto storage = createStorage();

    // An existing collection
    Akonadi::Collection collection(calendar2().id());

    // A spied monitor
    auto monitor = createMonitor();
    QSignalSpy changeSpy(monitor.data(), SIGNAL(collectionChanged(Akonadi::Collection)));
    MonitorSpy monitorSpy(monitor.data());

    // WHEN
    static int run = 1;
    collection.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Collection::AddIfMissing)
            ->setIconName(QString("folder-%1").arg(run++));
    collection.setEnabled(isEnabled);
    collection.setReferenced(isReferenced);
    auto job = storage->updateCollection(collection);
    AKVERIFYEXEC(job);
    monitorSpy.waitForStableState();
    QTRY_VERIFY(!changeSpy.isEmpty());

    // THEN
    QCOMPARE(changeSpy.size(), 1);

    auto notifiedCollection = changeSpy.takeFirst().takeFirst().value<Akonadi::Collection>();
    QCOMPARE(notifiedCollection.id(), collection.id());
    QCOMPARE(notifiedCollection.enabled(), isEnabled);
    QCOMPARE(notifiedCollection.referenced(), isReferenced);
}

void AkonadiStorageTestBase::shouldFindCollectionsByName_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QStringList>("expectedResults");
    QTest::addColumn<bool>("referenceCalendar1");
    QTest::addColumn<bool>("enableCalendar1");

    QStringList expectedResults;
    expectedResults << "Calendar1";
    QTest::newRow("get a collection") << "Calendar1" << expectedResults << false << true;

    expectedResults.clear();
    QTest::newRow("try with unknown name") << "toto" << expectedResults << false << true;

    expectedResults << "Calendar3" << "Calendar2" << "Calendar1";
    QTest::newRow("try with a part of a name") << "Calendar" << expectedResults << false << true;

    expectedResults.clear();
    expectedResults << "Calendar2";
    QTest::newRow("make sure it is case insensitive") << "calendar2" << expectedResults << false << true;

    expectedResults.clear();
    expectedResults << "Calendar1";
    QTest::newRow("include referenced") << "Calendar1" << expectedResults << true << false;
    QTest::newRow("include referenced + enabled") << "Calendar1" << expectedResults << true << true;
    QTest::newRow("include !referenced + !enabled") << "Calendar1" << expectedResults << false << false;
}

void AkonadiStorageTestBase::shouldFindCollectionsByName()
{
    // GIVEN
    auto storage = createStorage();

    QFETCH(QString, name);
    QFETCH(QStringList, expectedResults);
    QFETCH(bool, referenceCalendar1);
    QFETCH(bool, enableCalendar1);

    // A spied monitor
    auto monitor = createMonitor();
    MonitorSpy monitorSpy(monitor.data());

    // Default is not referenced and enabled
    // no need to feedle with the collection in that case
    if (referenceCalendar1 || !enableCalendar1) {
        Akonadi::Collection cal1 = calendar1();
        cal1.setReferenced(referenceCalendar1);
        cal1.setEnabled(enableCalendar1);
        auto update = storage->updateCollection(cal1);
        AKVERIFYEXEC(update);
        monitorSpy.waitForStableState();
    }

    // WHEN
    auto job = storage->searchCollections(name);
    AKVERIFYEXEC(job->kjob());
    monitorSpy.waitForStableState();

    // THEN
    auto collections = job->collections();

    // Restore proper DB state
    if (referenceCalendar1 || !enableCalendar1) {
        Akonadi::Collection cal1 = calendar1();
        cal1.setReferenced(false);
        cal1.setEnabled(true);
        auto update = storage->updateCollection(cal1);
        AKVERIFYEXEC(update);
        monitorSpy.waitForStableState();
    }

    auto collectionNames = QStringList();
    std::transform(collections.constBegin(), collections.constEnd(),
                   std::back_inserter(collectionNames),
                   Utils::mem_fn(&Akonadi::Collection::name));
    QCOMPARE(collectionNames.toSet(), expectedResults.toSet());
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

    return job->items().first();
}

Akonadi::Collection AkonadiStorageTestBase::fetchCollectionByRID(const QString &remoteId)
{
    Akonadi::Collection collection;
    collection.setRemoteId(remoteId);

    auto job = createStorage()->fetchCollections(collection, Akonadi::StorageInterface::Base, Akonadi::StorageInterface::AllContent);
    job->setResource("akonadi_knut_resource_0");
    job->setFiltered(false);
    if (!job->kjob()->exec()) {
        qWarning() << job->kjob()->errorString() << remoteId;
        return Akonadi::Collection();
    }

    if (job->collections().count() != 1) {
        qWarning() << "Received unexpected amount of collections: " << job->collections().count();
        return Akonadi::Collection();
    }

    return job->collections().first();
}

Akonadi::Tag AkonadiStorageTestBase::fetchTagByGID(const QString &gid)
{
    auto job = createStorage()->fetchTags();
    if (!job->kjob()->exec()) {
        qWarning() << job->kjob()->errorString();
        return Akonadi::Tag();
    }

    auto tags = job->tags();
    for (const Akonadi::Tag &tag : tags) {
        if (tag.gid() == gid)
            return tag;
    }

    return Akonadi::Tag();
}

Akonadi::Collection AkonadiStorageTestBase::calendar1()
{
    return fetchCollectionByRID("{cdc229c7-a9b5-4d37-989d-a28e372be2a9}");
}

Akonadi::Collection AkonadiStorageTestBase::calendar2()
{
    return fetchCollectionByRID("{e682b8b5-b67c-4538-8689-6166f64177f0}");
}

Akonadi::Collection AkonadiStorageTestBase::emails()
{
    return fetchCollectionByRID("{14096930-7bfe-46ca-8fba-7c04d3b62ec8}");
}

