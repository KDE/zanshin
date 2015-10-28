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

#include "akonadi/akonaditagqueries.h"
#include "akonadi/akonadiserializer.h"

#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/gennote.h"
#include "testlib/gentag.h"
#include "testlib/testhelpers.h"

using namespace Testlib;

class AkonadiTagQueriesTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldLookInAllReportedForAllTag()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two plain tags
        data.createTag(GenTag().withId(42).withName("42").asPlain());
        data.createTag(GenTag().withId(43).withName("43").asPlain());

        // WHEN
        QScopedPointer<Domain::TagQueries> queries(new Akonadi::TagQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                           Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                           Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        result->data();
        result = queries->findAll(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QString("42"));
        QCOMPARE(result->data().at(1)->name(), QString("43"));
    }

    void shouldReactToTagAdded()
    {
        // GIVEN
        AkonadiFakeData data;

        QScopedPointer<Domain::TagQueries> queries(new Akonadi::TagQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                           Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                           Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createTag(GenTag().withId(42).withName("42").asPlain());
        data.createTag(GenTag().withId(43).withName("43").asPlain());

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QString("42"));
        QCOMPARE(result->data().at(1)->name(), QString("43"));
    }

    void shouldReactToTagRemoved()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two plain tags
        data.createTag(GenTag().withId(42).withName("42").asPlain());
        data.createTag(GenTag().withId(43).withName("43").asPlain());

        QScopedPointer<Domain::TagQueries> queries(new Akonadi::TagQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                           Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                           Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.removeTag(Akonadi::Tag(43));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QString("42"));
    }

    void shouldReactToTagChanges()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two plain tags
        data.createTag(GenTag().withId(42).withName("42").asPlain());
        data.createTag(GenTag().withId(43).withName("43").asPlain());

        QScopedPointer<Domain::TagQueries> queries(new Akonadi::TagQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                           Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                           Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyTag(GenTag(data.tag(43)).withName("43bis"));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QString("42"));
        QCOMPARE(result->data().at(1)->name(), QString("43bis"));
    }

    void shouldLookInAllCollectionsForTagTopLevelArtifacts()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withNoteContent());

        // One plain tag
        data.createTag(GenTag().withId(42).withName("42").asPlain());

        // Two notes in the first collection
        data.createItem(GenNote().withParent(42).withId(42).withTitle("42").withTags({42}));
        data.createItem(GenNote().withParent(42).withId(43).withTitle("43"));

        // Two notes in the second collection
        data.createItem(GenNote().withParent(43).withId(44).withTitle("44").withTags({42}));
        data.createItem(GenNote().withParent(43).withId(45).withTitle("45"));

        // WHEN
        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::TagQueries> queries(new Akonadi::TagQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                           serializer,
                                                                           Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        auto tag = serializer->createTagFromAkonadiTag(data.tag(42));
        auto result = queries->findNotes(tag);
        result->data();
        result = queries->findNotes(tag); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("44"));

        // Should not change nothing
        result = queries->findNotes(tag);
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("44"));
    }

    void shouldReactToItemAddedForTag()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withNoteContent());

        // One plain tag
        data.createTag(GenTag().withId(42).withName("tag-42").asPlain());

        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::TagQueries> queries(new Akonadi::TagQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                           serializer,
                                                                           Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        auto tag = serializer->createTagFromAkonadiTag(data.tag(42));
        auto result = queries->findNotes(tag);
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createItem(GenNote().withParent(42).withId(42).withTitle("42").withTags({42}));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
    }

    void shouldReactToItemNewlyAssociatedToTag()
    {
        // GIVEN
        AkonadiFakeData data;

        // One plain tag
        data.createTag(GenTag().withId(42).withName("tag-42").asPlain());

        // One top level collection with a note
        data.createCollection(GenCollection().withId(42).withRootAsParent().withNoteContent());
        data.createItem(GenNote().withId(42).withParent(42).withTitle("42"));

        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::TagQueries> queries(new Akonadi::TagQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                           serializer,
                                                                           Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        auto tag = serializer->createTagFromAkonadiTag(data.tag(42));
        auto result = queries->findNotes(tag);

        bool insertHandlerCalled = false;
        result->addPostInsertHandler([&insertHandlerCalled](const Domain::Artifact::Ptr &, int) {
                                          insertHandlerCalled = true;
                                      });

        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.modifyItem(GenNote(data.item(42)).withTags({42}));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("42"));

        QVERIFY(insertHandlerCalled);
    }

    void shouldReactWhenAnItemTaggedIsRemoved()
    {
        // GIVEN

        AkonadiFakeData data;

        // One plain tag
        data.createTag(GenTag().withId(42).withName("tag-42").asPlain());

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withNoteContent());

        // Two notes related to the tag
        data.createItem(GenNote().withId(42).withParent(42).withTitle("42").withTags({42}));
        data.createItem(GenNote().withId(43).withParent(42).withTitle("43").withTags({42}));


        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::TagQueries> queries(new Akonadi::TagQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                           serializer,
                                                                           Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        auto tag = serializer->createTagFromAkonadiTag(data.tag(42));
        auto result = queries->findNotes(tag);

        bool removeHandlerCalled = false;
        result->addPostRemoveHandler([&removeHandlerCalled](const Domain::Artifact::Ptr &, int) {
                                          removeHandlerCalled = true;
                                      });

        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("43"));

        // WHEN
        data.removeItem(Akonadi::Item(43));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QVERIFY(removeHandlerCalled);
    }
};

QTEST_MAIN(AkonadiTagQueriesTest)

#include "akonaditagqueriestest.moc"
