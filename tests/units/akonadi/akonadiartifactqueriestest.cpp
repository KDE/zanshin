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

#include "akonadi/akonadiartifactqueries.h"
#include "akonadi/akonadiserializer.h"

#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/gennote.h"
#include "testlib/gentag.h"
#include "testlib/gentodo.h"
#include "testlib/testhelpers.h"

using namespace Testlib;

#include "utils/mockobject.h"

#include "testlib/akonadifakejobs.h"
#include "testlib/akonadifakemonitor.h"

#include "domain/note.h"
#include "domain/task.h"

#include "akonadi/akonadiapplicationselectedattribute.h"
#include "akonadi/akonadiartifactqueries.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;

Q_DECLARE_METATYPE(Testlib::AkonadiFakeItemFetchJob*)
Q_DECLARE_METATYPE(Testlib::AkonadiFakeCollectionFetchJob*)

class AkonadiArtifactQueriesTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldLookInAllSelectedCollectionsForInboxArtifacts()
    {
        // GIVEN
        AkonadiFakeData data;

        // Three top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(44).withRootAsParent().withTaskContent().enabled(false));

        // One note in the first collection
        data.createItem(GenNote().withId(42).withParent(42).withTitle("42"));

        // Two tasks in the second collection
        data.createItem(GenTodo().withId(43).withParent(43).withTitle("43"));
        data.createItem(GenTodo().withId(44).withParent(43).withTitle("44"));

        // One task in the third collection
        data.createItem(GenTodo().withId(45).withParent(44).withTitle("45"));

        // WHEN
        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                     Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                     Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findInboxTopLevel();
        result->data();
        result = queries->findInboxTopLevel(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("43"));
        QCOMPARE(result->data().at(2)->title(), QString("44"));
    }

    void shouldIgnoreItemsWhichAreNotTasksOrNotesInInbox()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withNoteContent());

        // Two items in the collection
        data.createItem(GenNote().withId(42).withParent(42).withTitle("42"));
        // One of them is not a task or a note
        auto item = Akonadi::Item(43);
        item.setPayloadFromData("FooBar");
        item.setParentCollection(Akonadi::Collection(42));
        data.createItem(item);

        // WHEN
        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                     Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                     Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findInboxTopLevel();

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
    }

    void shouldNotHaveArtifactsWithParentsInInbox()
    {
        // TODO: Note that this specification is kind of an over simplification which
        // assumes that all the underlying data is correct. Ideally it should be checked
        // that the uid refered to actually points to a todo which exists in a proper
        // collection. We will need a cache to be able to implement that properly though.

        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent().withNoteContent());

        // Three items in the collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42").withUid("uid-42"));
        data.createItem(GenTodo().withId(43).withParent(42).withTitle("43").withUid("uid-43").withParentUid("uid-42"));
        data.createItem(GenTodo().withId(44).withParent(42).withTitle("44").withUid("uid-44").withParentUid("foo"));

        // WHEN
        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                     Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                     Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findInboxTopLevel();

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
    }

    void shouldNotHaveArtifactsWithContextsOrTagsInInbox_data()
    {
        QTest::addColumn<bool>("isTodo");
        QTest::addColumn<bool>("hasContexts");
        QTest::addColumn<bool>("hasTags");
        QTest::addColumn<bool>("isExpectedInInbox");

        QTest::newRow("task with no tags") << true << false << false << true;
        QTest::newRow("task with tags") << true << false << true << false;
        QTest::newRow("task with contexts") << true << true << false << false;
        QTest::newRow("task with both") << true << true << true << false;

        QTest::newRow("note with no tags") << false << false << false << true;
        QTest::newRow("note with tags") << false << false << true << false;
        QTest::newRow("note with contexts") << false << true << false << true;
        QTest::newRow("note with both") << false << true << true << false;
    }

    void shouldNotHaveArtifactsWithContextsOrTagsInInbox()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent().withNoteContent());

        // One plain tag and one context tag
        data.createTag(GenTag().withId(42).withName("tag-42").asPlain());
        data.createTag(GenTag().withId(43).withName("tag-43").asContext());

        // One item in the collection
        QFETCH(bool, hasTags);
        QFETCH(bool, hasContexts);
        auto tagIds = QList<Akonadi::Tag::Id>();
        if (hasTags) tagIds << 42;
        if (hasContexts) tagIds << 43;

        QFETCH(bool, isTodo);
        if (isTodo) {
            data.createItem(GenTodo().withId(42).withParent(42).withTitle("42").withTags(tagIds));
        } else {
            data.createItem(GenNote().withId(42).withParent(42).withTitle("42").withTags(tagIds));
        }

        // WHEN
        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                     Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                     Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findInboxTopLevel();

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QFETCH(bool, isExpectedInInbox);
        if (isExpectedInInbox) {
            QCOMPARE(result->data().size(), 1);
            QCOMPARE(result->data().at(0)->title(), QString("42"));
        } else {
            QVERIFY(result->data().isEmpty());
        }
    }

    void shouldReactToItemAddsForInbox_data()
    {
        QTest::addColumn<bool>("reactionExpected");
        QTest::addColumn<bool>("isTodo");
        QTest::addColumn<QString>("relatedUid");
        QTest::addColumn<bool>("hasContexts");
        QTest::addColumn<bool>("hasTags");

        QTest::newRow("task which should be in inbox") << true << true << QString() << false << false;
        QTest::newRow("task with related uid") << false << true << "foo" << false << false;
        QTest::newRow("task with context") << false << true << QString() << true << false;
        QTest::newRow("task with tag") << false << true << QString() << false << true;

        QTest::newRow("note which should be in inbox") << true << false << QString() << false << false;
        QTest::newRow("note with related uid") << false << false << "foo" << false << false;
        QTest::newRow("note with tag") << false << false << QString() << false << true;
        QTest::newRow("note with context") << true << false << QString() << true << false;
    }

    void shouldReactToItemAddsForInbox()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent().withNoteContent());

        // One plain tag and one context tag
        data.createTag(GenTag().withId(42).withName("tag-42").asPlain());
        data.createTag(GenTag().withId(43).withName("tag-43").asContext());

        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                     Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                     Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findInboxTopLevel();
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        QFETCH(QString, relatedUid);
        QFETCH(bool, hasTags);
        QFETCH(bool, hasContexts);
        auto tagIds = QList<Akonadi::Tag::Id>();
        if (hasTags) tagIds << 42;
        if (hasContexts) tagIds << 43;

        QFETCH(bool, isTodo);
        if (isTodo) {
            data.createItem(GenTodo().withId(42).withParent(42).withTitle("42").withTags(tagIds).withParentUid(relatedUid));
        } else {
            data.createItem(GenNote().withId(42).withParent(42).withTitle("42").withTags(tagIds).withParentUid(relatedUid));
        }

        // THEN
        QFETCH(bool, reactionExpected);
        if (reactionExpected) {
            QCOMPARE(result->data().size(), 1);
            QCOMPARE(result->data().at(0)->title(), QString("42"));
        } else {
            QVERIFY(result->data().isEmpty());
        }
    }

    void shouldReactToItemRemovesForInbox()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One item in the collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42"));

        // WHEN
        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                     Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                     Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findInboxTopLevel();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first()->title(), QString("42"));

        // WHEN
        data.removeItem(Akonadi::Item(42));

        // THEN
        QVERIFY(result->data().isEmpty());
    }

    void shouldReactToItemChangesForInbox_data()
    {
        QTest::addColumn<bool>("inListAfterChange");
        QTest::addColumn<bool>("isTodo");
        QTest::addColumn<QString>("relatedUidBefore");
        QTest::addColumn<QString>("relatedUidAfter");
        QTest::addColumn<bool>("hasContextsBefore");
        QTest::addColumn<bool>("hasContextsAfter");
        QTest::addColumn<bool>("hasTagsBefore");
        QTest::addColumn<bool>("hasTagsAfter");

        QTest::newRow("task appears in inbox (related uid)") << true << true << "foo" << QString() << false << false << false << false;
        QTest::newRow("task disappears from inbox (related uid)") << false << true << QString() << "foo" << false << false << false << false;
        QTest::newRow("task appears in inbox (context)") << true << true << QString() << QString() << true << false << false << false;
        QTest::newRow("task disappears from inbox (context)") << false << true << QString() << QString() << false << true << false << false;

        QTest::newRow("note appears in inbox (related uid)") << true << false << "foo" << QString() << false << false << false << false;
        QTest::newRow("note disappears from inbox (related uid)") << false << false << QString() << "foo" << false << false << false << false;
        QTest::newRow("note appears in inbox (tag)") << true << false << QString() << QString() << false << false << true << false;
        QTest::newRow("note disappears from inbox (tag)") << false << false << QString() << QString() << false << false << false << true;
    }

    void shouldReactToItemChangesForInbox()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent().withNoteContent());

        // One plain tag and one context tag
        data.createTag(GenTag().withId(42).withName("tag-42").asPlain());
        data.createTag(GenTag().withId(43).withName("tag-43").asContext());

        // Artifact data
        QFETCH(QString, relatedUidBefore);
        QFETCH(bool, hasContextsBefore);
        QFETCH(bool, hasTagsBefore);

        auto tagIds = QList<Akonadi::Tag::Id>();
        if (hasTagsBefore) tagIds << 42;
        if (hasContextsBefore) tagIds << 43;

        QFETCH(bool, isTodo);
        if (isTodo) {
            data.createItem(GenTodo().withId(42).withParent(42).withTitle("42").withTags(tagIds).withParentUid(relatedUidBefore));
        } else {
            data.createItem(GenNote().withId(42).withParent(42).withTitle("42").withTags(tagIds).withParentUid(relatedUidBefore));
        }

        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                     Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                     Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findInboxTopLevel();
        TestHelpers::waitForEmptyJobQueue();

        QFETCH(bool, inListAfterChange);

        if (inListAfterChange) {
            QVERIFY(result->data().isEmpty());
        } else {
            QCOMPARE(result->data().size(), 1);
            QCOMPARE(result->data().at(0)->title(), QString("42"));
        }

        // WHEN
        QFETCH(QString, relatedUidAfter);
        QFETCH(bool, hasContextsAfter);
        QFETCH(bool, hasTagsAfter);

        tagIds.clear();
        if (hasTagsAfter) tagIds << 42;
        if (hasContextsAfter) tagIds << 43;

        if (isTodo) {
            data.modifyItem(GenTodo(data.item(42)).withTags(tagIds).withParentUid(relatedUidAfter));
        } else {
            data.modifyItem(GenNote(data.item(42)).withTags(tagIds).withParentUid(relatedUidAfter));
        }

        // THEN
        if (inListAfterChange) {
            QCOMPARE(result->data().size(), 1);
            QCOMPARE(result->data().at(0)->title(), QString("42"));
        } else {
            QVERIFY(result->data().isEmpty());
        }
    }

    void shouldReactToCollectionSelectionChangesForInbox()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withTaskContent());

        // One task in each collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42"));
        data.createItem(GenTodo().withId(43).withParent(43).withTitle("43"));

        QScopedPointer<Domain::ArtifactQueries> queries(new Akonadi::ArtifactQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                     Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                     Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findInboxTopLevel();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
        QCOMPARE(result->data().at(1)->title(), QString("43"));

        // WHEN
        data.modifyCollection(GenCollection(data.collection(43)).selected(false));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("42"));
    }
};

QTEST_MAIN(AkonadiArtifactQueriesTest)

#include "akonadiartifactqueriestest.moc"
