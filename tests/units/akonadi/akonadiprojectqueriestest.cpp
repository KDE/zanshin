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

#include "akonadi/akonadiprojectqueries.h"
#include "akonadi/akonadiserializer.h"

#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/gentodo.h"
#include "testlib/testhelpers.h"

using namespace Testlib;

class AkonadiProjectQueriesTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldLookInAllReportedForAllProjects()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withTaskContent());

        // One project in the first collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42").asProject());

        // Two projects in the second collection
        data.createItem(GenTodo().withId(43).withParent(43).withTitle("43").asProject());
        data.createItem(GenTodo().withId(44).withParent(43).withTitle("44").asProject());

        // WHEN
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        result->data();
        result = queries->findAll(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->name(), QString("42"));
        QCOMPARE(result->data().at(1)->name(), QString("43"));
        QCOMPARE(result->data().at(2)->name(), QString("44"));
    }

    void shouldIgnoreItemsWhichAreNotProjects()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project and one regular task in the collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42").asProject());
        data.createItem(GenTodo().withId(43).withParent(42).withTitle("43"));

        // WHEN
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QString("42"));
    }

    void shouldReactToItemAddsForProjectsOnly()
    {
        // GIVEN
        AkonadiFakeData data;

        // One empty collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42").asProject());
        data.createItem(GenTodo().withId(43).withParent(42).withTitle("43"));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QString("42"));
    }

    void shouldReactToItemRemovesForAllProjects()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three projects in the collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42").asProject());
        data.createItem(GenTodo().withId(43).withParent(42).withTitle("43").asProject());
        data.createItem(GenTodo().withId(44).withParent(42).withTitle("44").asProject());

        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 3);

        // WHEN
        data.removeItem(Akonadi::Item(43));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QString("42"));
        QCOMPARE(result->data().at(1)->name(), QString("44"));
    }

    void shouldReactToItemChangesForAllProjects()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three projects in the collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42").asProject());
        data.createItem(GenTodo().withId(43).withParent(42).withTitle("43").asProject());
        data.createItem(GenTodo().withId(44).withParent(42).withTitle("44").asProject());

        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        // Even though the pointer didn't change it's convenient to user if we call
        // the replace handlers
        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::Project::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 3);

        // WHEN
        data.modifyItem(GenTodo(data.item(43)).withTitle("43bis"));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->name(), QString("42"));
        QCOMPARE(result->data().at(1)->name(), QString("43bis"));
        QCOMPARE(result->data().at(2)->name(), QString("44"));
        QVERIFY(replaceHandlerCalled);
    }

    void shouldReactToCollectionSelectionChangesForAllProjects()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withTaskContent());

        // Two projects, one in each collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle("42").asProject());
        data.createItem(GenTodo().withId(43).withParent(43).withTitle("43").asProject());

        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QString("42"));
        QCOMPARE(result->data().at(1)->name(), QString("43"));

        // WHEN
        data.modifyCollection(GenCollection(data.collection(43)).selected(false));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QString("42"));
    }

    void shouldLookOnlyInParentCollectionForProjectTopLevel()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withTaskContent());

        // One project and two tasks in the first collection (one task being child of the project)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42").asProject());
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43")
                                 .withParentUid("uid-42"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44"));

        // Two tasks in the second collection (one having the project uid as parent)
        data.createItem(GenTodo().withId(45).withParent(43)
                                 .withTitle("45").withParentUid("uid-42"));
        data.createItem(GenTodo().withId(46).withParent(43).withTitle("46"));

        // WHEN
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   serializer,
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto project = serializer->createProjectFromItem(data.item(42));
        auto result = queries->findTopLevel(project);
        result->data();
        result = queries->findTopLevel(project); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("43"));

        // Should not change nothing
        result = queries->findTopLevel(project);

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("43"));
    }

    void shouldNotCrashWhenWeAskAgainTheSameTopLevelArtifacts()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project in the collection
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42").asProject());

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   serializer,
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto project = serializer->createProjectFromItem(data.item(42));

        // The bug we're trying to hit here is the following:
        //  - when findChildren is called the first time a provider is created internally
        //  - result is deleted at the end of the loop, no one holds the provider with
        //    a strong reference anymore so it is deleted as well
        //  - when findChildren is called the second time, there's a risk of a dangling
        //    pointer if the recycling of providers is wrongly implemented which can lead
        //    to a crash, if it is properly done no crash will occur
        for (int i = 0; i < 2; i++) {
            // WHEN * 2
            auto result = queries->findTopLevel(project);

            // THEN * 2
            QVERIFY(result->data().isEmpty());
            TestHelpers::waitForEmptyJobQueue();
            QVERIFY(result->data().isEmpty());
        }
    }

    void shouldReactToItemAddsForTopLevelArtifact()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project in the collection
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42").asProject());

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   serializer,
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto project = serializer->createProjectFromItem(data.item(42));
        auto result = queries->findTopLevel(project);
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43")
                                 .withParentUid("uid-42"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("43"));
        QCOMPARE(result->data().at(1)->title(), QString("44"));
    }

    void shouldReactToItemChangesForTopLevelArtifacts()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project and two tasks in the collection (tasks being children of the project)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42").asProject());
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43")
                                 .withParentUid("uid-42"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   serializer,
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto project = serializer->createProjectFromItem(data.item(42));
        auto result = queries->findTopLevel(project);

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::Artifact::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyItem(GenTodo(data.item(43)).withTitle("43bis"));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("43bis"));
        QCOMPARE(result->data().at(1)->title(), QString("44"));

        QVERIFY(replaceHandlerCalled);
    }

    void shouldRemoveItemFromCorrespondingResultWhenRelatedItemChangesForTopLevelArtifact()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project and two tasks in the collection (tasks being children of the project)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42").asProject());
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43")
                                 .withParentUid("uid-42"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   serializer,
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto project = serializer->createProjectFromItem(data.item(42));
        auto result = queries->findTopLevel(project);
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyItem(GenTodo(data.item(43)).withParentUid(""));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("44"));
    }

    void shouldAddItemToCorrespondingResultWhenRelatedItemChangeForChildrenTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project and two tasks in the collection (one being child of the project)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42").asProject());
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   serializer,
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto project = serializer->createProjectFromItem(data.item(42));
        auto result = queries->findTopLevel(project);

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::Artifact::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);

        // WHEN
        data.modifyItem(GenTodo(data.item(43)).withParentUid("uid-42"));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QString("44"));
        QCOMPARE(result->data().at(1)->title(), QString("43"));

        QVERIFY(!replaceHandlerCalled);
    }

    void shouldMoveItemToCorrespondingResultWhenRelatedItemChangeForTopLevelArtifacts()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Two project and one task in the collection (task being child of the first project)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42").asProject());
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43").asProject());
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   serializer,
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto project1 = serializer->createProjectFromItem(data.item(42));
        auto project2 = serializer->createProjectFromItem(data.item(43));
        auto result1 = queries->findTopLevel(project1);
        auto result2 = queries->findTopLevel(project2);

        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result1->data().size(), 1);
        QCOMPARE(result1->data().at(0)->title(), QString("44"));
        QCOMPARE(result2->data().size(), 0);

        // WHEN
        data.modifyItem(GenTodo(data.item(44)).withParentUid("uid-43"));

        // THEN
        QCOMPARE(result1->data().size(), 0);
        QCOMPARE(result2->data().size(), 1);
        QCOMPARE(result2->data().at(0)->title(), QString("44"));
    }

    void shouldReactToItemRemovesForTopLevelArtifacts()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project and two tasks in the collection (tasks being children of the project)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle("42").withUid("uid-42").asProject());
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle("43").withUid("uid-43")
                                 .withParentUid("uid-42"));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle("44").withUid("uid-44")
                                 .withParentUid("uid-42"));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   serializer,
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto project = serializer->createProjectFromItem(data.item(42));
        auto result = queries->findTopLevel(project);
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.removeItem(Akonadi::Item(43));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QString("44"));
    }
};

QTEST_MAIN(AkonadiProjectQueriesTest)

#include "akonadiprojectqueriestest.moc"
