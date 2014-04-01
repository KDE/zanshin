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

#include <Akonadi/Collection>
#include <Akonadi/CollectionStatistics>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemModifyJob>

#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonadimonitorimpl.h"
#include "akonadi/akonadistorage.h"

class AkonadiStorageTest : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiStorageTest(QObject *parent = 0)
        : QObject(parent)
    {
        qRegisterMetaType<Akonadi::Item>();
    }

private slots:
    void shouldListOnlyNotesAndTodoCollections()
    {
        // GIVEN
        Akonadi::Storage storage;
        const QStringList expectedNames = { "Calendar1", "Calendar2", "Calendar3", "Notes" };

        // WHEN
        auto job = storage.fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive);
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
        auto job = storage.fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive);
        job->kjob()->exec();

        // THEN
        auto collections = job->collections();
        for (const auto &collection : collections) {
            auto parent = collection.parentCollection();
            while (parent != Akonadi::Collection::root()) {
                QVERIFY(parent.isValid());
                parent = parent.parentCollection();
            }
        }
    }


    void shouldListFullItemsInACollection()
    {
        // GIVEN
        Akonadi::Storage storage;
        const QStringList expectedRemoteIds = { "{1d33862f-f274-4c67-ab6c-362d56521ff4}",
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
            if (!spy.isEmpty()) break;
            QTest::qWait(50);
        }

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

private:
    Akonadi::Collection calendar2()
    {
        // Calendar2 is supposed to get this id, hopefully this won't be too fragile
        return Akonadi::Collection(6);
    }
};

QTEST_MAIN(AkonadiStorageTest)

#include "akonadistoragetest.moc"
