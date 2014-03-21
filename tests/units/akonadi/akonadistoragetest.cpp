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

#include <KJob>

#include <Akonadi/Collection>
#include <Akonadi/CollectionStatistics>

#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonadistorage.h"

class AkonadiStorageTest : public QObject
{
    Q_OBJECT
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
            qDebug() << collection.id() << collection.name();
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
        // Calendar2 is supposed to get this id, hopefully this won't be too fragile
        Akonadi::Collection collection(6);
        const QStringList expectedRemoteIds = { "{1d33862f-f274-4c67-ab6c-362d56521ff4}",
                                                "{7824df00-2fd6-47a4-8319-52659dc82005}" };

        // WHEN
        auto job = storage.fetchItems(collection);
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
};

QTEST_MAIN(AkonadiStorageTest)

#include "akonadistoragetest.moc"
