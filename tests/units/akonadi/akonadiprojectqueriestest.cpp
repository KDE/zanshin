/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

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
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")).asProject());

        // Two projects in the second collection
        data.createItem(GenTodo().withId(43).withParent(43).withTitle(QStringLiteral("43")).asProject());
        data.createItem(GenTodo().withId(44).withParent(43).withTitle(QStringLiteral("44")).asProject());

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
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43"));
        QCOMPARE(result->data().at(2)->name(), QStringLiteral("44"));
    }

    void shouldIgnoreItemsWhichAreNotProjects()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project and one regular task in the collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")).asProject());
        data.createItem(GenTodo().withId(43).withParent(42).withTitle(QStringLiteral("43")));

        // WHEN
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
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
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")).asProject());
        data.createItem(GenTodo().withId(43).withParent(42).withTitle(QStringLiteral("43")));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
    }

    void shouldReactToItemRemovesForAllProjects()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three projects in the collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")).asProject());
        data.createItem(GenTodo().withId(43).withParent(42).withTitle(QStringLiteral("43")).asProject());
        data.createItem(GenTodo().withId(44).withParent(42).withTitle(QStringLiteral("44")).asProject());

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
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("44"));
    }

    void shouldReactToItemChangesForAllProjects()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three projects in the collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")).asProject());
        data.createItem(GenTodo().withId(43).withParent(42).withTitle(QStringLiteral("43")).asProject());
        data.createItem(GenTodo().withId(44).withParent(42).withTitle(QStringLiteral("44")).asProject());

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
        data.modifyItem(GenTodo(data.item(43)).withTitle(QStringLiteral("43bis")));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43bis"));
        QCOMPARE(result->data().at(2)->name(), QStringLiteral("44"));
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
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")).asProject());
        data.createItem(GenTodo().withId(43).withParent(43).withTitle(QStringLiteral("43")).asProject());

        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   Akonadi::Serializer::Ptr(new Akonadi::Serializer),
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto result = queries->findAll();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43"));

        // WHEN
        data.modifyCollection(GenCollection(data.collection(43)).selected(false));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
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
                                 .withTitle(QStringLiteral("42")).withUid(QStringLiteral("uid-42")).asProject());
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle(QStringLiteral("43")).withUid(QStringLiteral("uid-43"))
                                 .withParentUid(QStringLiteral("uid-42")));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle(QStringLiteral("44")).withUid(QStringLiteral("uid-44")));

        // Two tasks in the second collection (one having the project uid as parent)
        data.createItem(GenTodo().withId(45).withParent(43)
                                 .withTitle(QStringLiteral("45")).withParentUid(QStringLiteral("uid-42")));
        data.createItem(GenTodo().withId(46).withParent(43).withTitle(QStringLiteral("46")));

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
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("43"));

        // Should not change nothing
        result = queries->findTopLevel(project);

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("43"));
    }

    void shouldNotCrashWhenWeAskAgainTheSameTopLevelTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project in the collection
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle(QStringLiteral("42")).withUid(QStringLiteral("uid-42")).asProject());

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

    void shouldReactToItemAddsForTopLevelTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project in the collection
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle(QStringLiteral("42")).withUid(QStringLiteral("uid-42")).asProject());

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
                                 .withTitle(QStringLiteral("43")).withUid(QStringLiteral("uid-43"))
                                 .withParentUid(QStringLiteral("uid-42")));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle(QStringLiteral("44")).withUid(QStringLiteral("uid-44"))
                                 .withParentUid(QStringLiteral("uid-42")));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("43"));
        QCOMPARE(result->data().at(1)->title(), QStringLiteral("44"));
    }

    void shouldReactToItemChangesForTopLevelTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project and two tasks in the collection (tasks being children of the project)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle(QStringLiteral("42")).withUid(QStringLiteral("uid-42")).asProject());
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle(QStringLiteral("43")).withUid(QStringLiteral("uid-43"))
                                 .withParentUid(QStringLiteral("uid-42")));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle(QStringLiteral("44")).withUid(QStringLiteral("uid-44"))
                                 .withParentUid(QStringLiteral("uid-42")));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   serializer,
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto project = serializer->createProjectFromItem(data.item(42));
        auto result = queries->findTopLevel(project);

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::Task::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyItem(GenTodo(data.item(43)).withTitle(QStringLiteral("43bis")));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("43bis"));
        QCOMPARE(result->data().at(1)->title(), QStringLiteral("44"));

        QVERIFY(replaceHandlerCalled);
    }

    void shouldRemoveItemFromCorrespondingResultWhenRelatedItemChangesForTopLevelTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project and two tasks in the collection (tasks being children of the project)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle(QStringLiteral("42")).withUid(QStringLiteral("uid-42")).asProject());
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle(QStringLiteral("43")).withUid(QStringLiteral("uid-43"))
                                 .withParentUid(QStringLiteral("uid-42")));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle(QStringLiteral("44")).withUid(QStringLiteral("uid-44"))
                                 .withParentUid(QStringLiteral("uid-42")));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   serializer,
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto project = serializer->createProjectFromItem(data.item(42));
        auto result = queries->findTopLevel(project);
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyItem(GenTodo(data.item(43)).withParentUid(QLatin1String("")));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("44"));
    }

    void shouldAddItemToCorrespondingResultWhenRelatedItemChangeForChildrenTask()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project and two tasks in the collection (one being child of the project)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle(QStringLiteral("42")).withUid(QStringLiteral("uid-42")).asProject());
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle(QStringLiteral("43")).withUid(QStringLiteral("uid-43")));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle(QStringLiteral("44")).withUid(QStringLiteral("uid-44"))
                                 .withParentUid(QStringLiteral("uid-42")));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::ProjectQueries> queries(new Akonadi::ProjectQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                   serializer,
                                                                                   Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        auto project = serializer->createProjectFromItem(data.item(42));
        auto result = queries->findTopLevel(project);

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::Task::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);

        // WHEN
        data.modifyItem(GenTodo(data.item(43)).withParentUid(QStringLiteral("uid-42")));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("44"));
        QCOMPARE(result->data().at(1)->title(), QStringLiteral("43"));

        QVERIFY(!replaceHandlerCalled);
    }

    void shouldMoveItemToCorrespondingResultWhenRelatedItemChangeForTopLevelTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Two project and one task in the collection (task being child of the first project)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle(QStringLiteral("42")).withUid(QStringLiteral("uid-42")).asProject());
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle(QStringLiteral("43")).withUid(QStringLiteral("uid-43")).asProject());
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle(QStringLiteral("44")).withUid(QStringLiteral("uid-44"))
                                 .withParentUid(QStringLiteral("uid-42")));

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
        QCOMPARE(result1->data().at(0)->title(), QStringLiteral("44"));
        QCOMPARE(result2->data().size(), 0);

        // WHEN
        data.modifyItem(GenTodo(data.item(44)).withParentUid(QStringLiteral("uid-43")));

        // THEN
        QCOMPARE(result1->data().size(), 0);
        QCOMPARE(result2->data().size(), 1);
        QCOMPARE(result2->data().at(0)->title(), QStringLiteral("44"));
    }

    void shouldReactToItemRemovesForTopLevelTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project and two tasks in the collection (tasks being children of the project)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle(QStringLiteral("42")).withUid(QStringLiteral("uid-42")).asProject());
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle(QStringLiteral("43")).withUid(QStringLiteral("uid-43"))
                                 .withParentUid(QStringLiteral("uid-42")));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle(QStringLiteral("44")).withUid(QStringLiteral("uid-44"))
                                 .withParentUid(QStringLiteral("uid-42")));

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
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("44"));
    }
};

ZANSHIN_TEST_MAIN(AkonadiProjectQueriesTest)

#include "akonadiprojectqueriestest.moc"
