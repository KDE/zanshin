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

#include "akonadi/akonadidatasourcequeries.h"
#include "akonadi/akonadiserializer.h"

#include "utils/jobhandler.h"
#include "utils/mem_fn.h"

#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/testhelpers.h"

using namespace Testlib;

typedef std::function<Domain::QueryResult<Domain::DataSource::Ptr>::Ptr(Domain::DataSourceQueries*)> QueryFunction;
Q_DECLARE_METATYPE(QueryFunction)
Q_DECLARE_METATYPE(Akonadi::StorageInterface::FetchContentType)

class AkonadiDataSourceQueriesTest : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiDataSourceQueriesTest(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
        qRegisterMetaType<QueryFunction>();
    }

private:
    void generateDataTable()
    {
        QTest::addColumn<Akonadi::StorageInterface::FetchContentType>("contentType");
        QTest::addColumn<QueryFunction>("queryFunction");

        {
            QueryFunction query = Utils::mem_fn(&Domain::DataSourceQueries::findNotes);
            QTest::newRow("notes") << Akonadi::StorageInterface::Notes << query;
        }

        {
            QueryFunction query = Utils::mem_fn(&Domain::DataSourceQueries::findTasks);
            QTest::newRow("tasks") << Akonadi::StorageInterface::Tasks << query;
        }
    }

private slots:
    void shouldListDataSources_data()
    {
        generateDataTable();
    }

    void shouldListDataSources()
    {
        // GIVEN
        AkonadiFakeData data;
        QFETCH(Akonadi::StorageInterface::FetchContentType, contentType);
        QFETCH(QueryFunction, queryFunction);

        // Six top level collections, three with tasks, 3 with notes
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("43Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(44).withName("44Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(45).withName("45Note").withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(46).withName("46Note").withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(47).withName("47Note").withRootAsParent().withNoteContent());

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries( Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                          Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                          Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queryFunction(queries.data());
        result->data();
        result = queryFunction(queries.data()); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 3);

        if (contentType == Akonadi::StorageInterface::Tasks) {
            QCOMPARE(result->data().at(0)->name(), QString("42Task"));
            QCOMPARE(result->data().at(1)->name(), QString("43Task"));
            QCOMPARE(result->data().at(2)->name(), QString("44Task"));
        } else {
            QCOMPARE(result->data().at(0)->name(), QString("45Note"));
            QCOMPARE(result->data().at(1)->name(), QString("46Note"));
            QCOMPARE(result->data().at(2)->name(), QString("47Note"));
        }
    }

    void shouldListDataSourcesOnlyOnce_data()
    {
        generateDataTable();
    }

    void shouldListDataSourcesOnlyOnce()
    {
        // GIVEN
        AkonadiFakeData data;
        QFETCH(Akonadi::StorageInterface::FetchContentType, contentType);
        QFETCH(QueryFunction, queryFunction);

        // Two top level collections, one with tasks, one with notes
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("43Note").withRootAsParent().withNoteContent());

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result1 = queryFunction(queries.data());
        QVERIFY(result1->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result1->data().size(), 1);

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result2 = queryFunction(queries.data());

        // THEN
        QCOMPARE(result1->data().size(), 1);
        QCOMPARE(result2->data().size(), 1);
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result1->data().size(), 1);
        QCOMPARE(result2->data().size(), 1);
        if (contentType == Akonadi::StorageInterface::Tasks) {
            QCOMPARE(result1->data().at(0)->name(), QString("42Task"));
        } else {
            QCOMPARE(result2->data().at(0)->name(), QString("43Note"));
        }
    }

    void shouldIgnoreCollectionsWhichAreNotDataSources_data()
    {
        generateDataTable();
    }

    void shouldIgnoreCollectionsWhichAreNotDataSources()
    {
        // GIVEN
        AkonadiFakeData data;
        QFETCH(Akonadi::StorageInterface::FetchContentType, contentType);
        QFETCH(QueryFunction, queryFunction);

            // Two top level collections with tasks, one not being a data source
            data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
            data.createCollection(GenCollection().withId(-1).withName("43Task").withTaskContent()); // invalid data source
            // One child collection
            data.createCollection(GenCollection().withId(44).withName("44Task").withParent(42).withTaskContent());

            // Two top level collections with notes, one not being a data source
            data.createCollection(GenCollection().withId(45).withName("45Note").withRootAsParent().withNoteContent());
            data.createCollection(GenCollection().withId(-2).withName("46Note").withNoteContent()); // invalid data source
            // One child collection
            data.createCollection(GenCollection().withId(47).withName("47Note").withParent(45).withNoteContent());

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queryFunction(queries.data());

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        if (contentType == Akonadi::StorageInterface::Tasks) {
            QCOMPARE(result->data().size(), 2);
            QCOMPARE(result->data().at(0)->name(), QString("42Task"));
            QCOMPARE(result->data().at(1)->name(), QString("42Task/44Task")); // name is: parentName/childName
        } else {
            QCOMPARE(result->data().size(), 2);
            QCOMPARE(result->data().at(0)->name(), QString("45Note"));
            QCOMPARE(result->data().at(1)->name(), QString("45Note/47Note")); // name is: parentName/childName
        }
    }

    void shouldReactToCollectionAddsForCompatibleDataSourcesOnly_data()
    {
        generateDataTable();
    }

    void shouldReactToCollectionAddsForCompatibleDataSourcesOnly()
    {
        // GIVEN
        AkonadiFakeData data;
        QFETCH(Akonadi::StorageInterface::FetchContentType, contentType);
        QFETCH(QueryFunction, queryFunction);

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor()))) ;

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queryFunction(queries.data());
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("43Note").withRootAsParent().withNoteContent());

        // THEN
        QCOMPARE(result->data().size(), 1);
        const QString expected = contentType == Akonadi::StorageInterface::Tasks ? "42Task" : "43Note";
        QCOMPARE(result->data().first()->name(), expected);
    }

    void shouldReactToCollectionRemovesForDataSources_data()
    {
        generateDataTable();
    }

    void shouldReactToCollectionRemovesForDataSources()
    {
        // GIVEN
        AkonadiFakeData data;
        QFETCH(Akonadi::StorageInterface::FetchContentType, contentType);
        QFETCH(QueryFunction, queryFunction);

        // Four top level collections, two with notes, two with tasks
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("43Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(45).withName("45Note").withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(46).withName("46Note").withRootAsParent().withNoteContent());

        // One child collection with tasks
        data.createCollection(GenCollection().withId(44).withName("44Task").withParent(43).withTaskContent());

        // One child collection with notes
        data.createCollection(GenCollection().withId(47).withName("47Note").withParent(46).withNoteContent());

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries( Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                          Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                          Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queryFunction(queries.data());
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 3);

        // WHEN
        data.removeCollection(Akonadi::Collection(44));
        data.removeCollection(Akonadi::Collection(47));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(result->data().size(), 2);
        if (contentType == Akonadi::StorageInterface::Tasks) {
            QCOMPARE(result->data().at(0)->name(), QString("42Task"));
            QCOMPARE(result->data().at(1)->name(), QString("43Task"));
        } else {
            QCOMPARE(result->data().at(0)->name(), QString("45Note"));
            QCOMPARE(result->data().at(1)->name(), QString("46Note"));
        }
    }

    void shouldReactToCollectionChangesForDataSources_data()
    {
        generateDataTable();
    }

    void shouldReactToCollectionChangesForDataSources()
    {
        // GIVEN
        AkonadiFakeData data;
        QFETCH(Akonadi::StorageInterface::FetchContentType, contentType);
        QFETCH(QueryFunction, queryFunction);

        // Four top level collections, two tasks, two notes),
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("43Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(45).withName("45Note").withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(46).withName("46Note").withRootAsParent().withNoteContent());

        // One child collection with tasks
        data.createCollection(GenCollection().withId(44).withName("44Task").withParent(43).withTaskContent());

        // One child collcetion with notes
        data.createCollection(GenCollection().withId(47).withName("47Note").withParent(46).withNoteContent());

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries( Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                          Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                          Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queryFunction(queries.data());
        // Even though the pointer didn't change it's convenient to user if we call
        // the replace handlers
        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::DataSource::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 3);

        // WHEN
        data.modifyCollection(GenCollection(data.collection(42)).withName("42TaskBis"));
        data.modifyCollection(GenCollection(data.collection(45)).withName("45NoteBis"));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(result->data().size(), 3);

        if (contentType == Akonadi::StorageInterface::Tasks) {
            QCOMPARE(result->data().at(0)->name(), QString("42TaskBis"));
            QCOMPARE(result->data().at(1)->name(), QString("43Task"));
            QCOMPARE(result->data().at(2)->name(), QString("43Task/44Task")); // name is: parentName/childName
            QVERIFY(replaceHandlerCalled);
        } else {
            QCOMPARE(result->data().at(0)->name(), QString("45NoteBis"));
            QCOMPARE(result->data().at(1)->name(), QString("46Note"));
            QCOMPARE(result->data().at(2)->name(), QString("46Note/47Note")); // name is: parentName/childName
            QVERIFY(replaceHandlerCalled);
        }
    }

    void shouldLookInAllReportedForTopLevelSources_data()
    {
        generateDataTable();
    }

    void shouldLookInAllReportedForTopLevelSources()
    {
        // GIVEN
        AkonadiFakeData data;
        QFETCH(QueryFunction, queryFunction);

        // Two top level collections, one with tasks, one with notes
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(44).withName("44Note").withRootAsParent().withNoteContent());

        // One with a note child collection
        data.createCollection(GenCollection().withId(43).withName("43TaskChild").withParent(42).withTaskContent());

        // One with a task child collection
        data.createCollection(GenCollection().withId(45).withName("45NoteChild").withParent(44).withNoteContent());

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries( Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                          Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                          Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findTopLevel();
        result->data();
        result = queries->findTopLevel(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().at(0)->name(), QString("42Task"));
        QCOMPARE(result->data().at(1)->name(), QString("44Note"));
    }

    void shouldReactToCollectionAddsForTopLevelSources()
    {
        // GIVEN
        AkonadiFakeData data;

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findTopLevel();
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("43Note").withRootAsParent().withNoteContent());

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QString("42Task"));
        QCOMPARE(result->data().at(1)->name(), QString("43Note"));
    }

    void shouldReactToCollectionRemovesForTopLevelSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections and two child collections
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("43Note").withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(44).withName("43TaskChild").withParent(42).withTaskContent());
        data.createCollection(GenCollection().withId(45).withName("43NoteChild").withParent(43).withNoteContent());

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries( Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                          Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                          Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findTopLevel();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.removeCollection(Akonadi::Collection(42));
        data.removeCollection(Akonadi::Collection(43));

        // THEN
        QCOMPARE(result->data().size(), 0);
    }

    void shouldReactToItemChangesForTopLevelTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections and one child collection
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("43Note").withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(44).withName("44NoteChild").withParent(43).withNoteContent());

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries( Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                          Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                          Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findTopLevel();

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::DataSource::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyCollection(GenCollection(data.collection(42)).withName("42TaskBis"));
        data.modifyCollection(GenCollection(data.collection(43)).withName("43NoteBis"));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QString("42TaskBis"));
        QCOMPARE(result->data().at(1)->name(), QString("43NoteBis"));
        QVERIFY(replaceHandlerCalled);
    }

    void shouldRemoveUnlistedCollectionFromTopLevelTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections and one child collection
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("43Note").withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(44).withName("44NoteChild").withParent(43).withNoteContent());

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findTopLevel();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyCollection(GenCollection(data.collection(43)).referenced(false).enabled(false)); // datasource unlisted now

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first()->name(), QString("42Task"));
    }

    void shouldNotCrashDuringFindTopLevelWhenFetchJobFailedOrEmpty_data()
    {
        QTest::addColumn<int>("colErrorCode");
        QTest::addColumn<int>("colFetchBehavior");
        QTest::addColumn<bool>("deleteQuery");

        QTest::newRow("No error with empty collection list") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                             << false;

        QTest::newRow("No error with empty collection list (+ query delete)") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                             << true;

        QTest::newRow("Error with empty collection list") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                          << false;

        QTest::newRow("Error with empty collection list (+ query delete)") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                          << true;

        QTest::newRow("Error with collection list") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                    << false;

        QTest::newRow("Error with collection list (+ query delete)") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                    << true;
    }

    void shouldNotCrashDuringFindTopLevelWhenFetchJobFailedOrEmpty()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withNoteContent());

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        QFETCH(int, colErrorCode);
        QFETCH(int, colFetchBehavior);
        data.storageBehavior().setFetchCollectionsErrorCode(Akonadi::Collection::root().id(), colErrorCode);
        data.storageBehavior().setFetchCollectionsBehavior(Akonadi::Collection::root().id(),
                                                           AkonadiFakeStorageBehavior::FetchBehavior(colFetchBehavior));

        QFETCH(bool, deleteQuery);

        // WHEN
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findTopLevel();

        if (deleteQuery)
            delete queries.take();

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 0);
    }

    void shouldLookInAllReportedForChildSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection with two children (one of them also having a child)
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("43TaskFirstChild").withParent(42).withTaskContent());
        data.createCollection(GenCollection().withId(44).withName("44TaskFirstChildChild").withParent(43).withTaskContent());
        data.createCollection(GenCollection().withId(45).withName("45NoteSecondChild").withParent(42).withNoteContent());

        // Serializer
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        Domain::DataSource::Ptr topLevelDataSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findChildren(topLevelDataSource);
        result->data();
        result = queries->findChildren(topLevelDataSource); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QString("43TaskFirstChild"));
        QCOMPARE(result->data().at(1)->name(), QString("45NoteSecondChild"));
    }

    void shouldReactToCollectionAddsForChildSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection with no child yet
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());

        // Serializer
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        Domain::DataSource::Ptr topLevelDataSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findChildren(topLevelDataSource);
        result->data();
        result = queries->findChildren(topLevelDataSource); // Should not cause any problem or wrong data
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 0);

        // WHEN
        data.createCollection(GenCollection().withId(43).withName("43TaskChild").withParent(42).withTaskContent());

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first()->name(), QString("43TaskChild"));
    }

    void shouldReactToCollectionRemovesForChildSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection with two children
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("43TaskFirstChild").withParent(42).withTaskContent());
        data.createCollection(GenCollection().withId(44).withName("45NoteSecondChild").withParent(42).withNoteContent());

        // Serializer
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        Domain::DataSource::Ptr topLevelDataSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findChildren(topLevelDataSource);
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.removeCollection(Akonadi::Collection(44));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first()->name(), QString("43TaskFirstChild"));
    }

    void shouldReactToCollectionChangesForChildSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection with two children
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("43TaskFirstChild").withParent(42).withTaskContent());
        data.createCollection(GenCollection().withId(44).withName("44NoteSecondChild").withParent(42).withNoteContent());

        // Serializer
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        Domain::DataSource::Ptr topLevelDataSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findChildren(topLevelDataSource);
        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::DataSource::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyCollection(GenCollection(data.collection(43)).withName("43TaskFirstChildBis"));

        // THEN
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().first()->name(), QString("43TaskFirstChildBis"));
        QCOMPARE(result->data().at(1)->name(), QString("44NoteSecondChild"));
        QVERIFY(replaceHandlerCalled);
    }

    void shouldRemoveUnlistedCollectionsFromChildSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection with two children
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("43TaskFirstChild").withParent(42).withTaskContent());
        data.createCollection(GenCollection().withId(44).withName("44NoteSecondChild").withParent(42).withNoteContent());

        // Serializer
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        Domain::DataSource::Ptr topLevelDataSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findChildren(topLevelDataSource);
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyCollection(GenCollection(data.collection(43)).referenced(false).enabled(false)); // unlist the child collection

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first()->name(), QString("44NoteSecondChild"));
    }

    void shouldNotCrashDuringFindChildrenWhenFetchJobFailedOrEmpty_data()
    {
        QTest::addColumn<int>("colErrorCode");
        QTest::addColumn<int>("colFetchBehavior");
        QTest::addColumn<bool>("deleteQuery");

        QTest::newRow("No error with empty collection list") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                             << false;

        QTest::newRow("No error with empty collection list (+ query delete)") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                             << true;

        QTest::newRow("Error with empty collection list") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                          << false;

        QTest::newRow("Error with empty collection list (+ query delete)") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                          << true;

        QTest::newRow("Error with collection list") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                    << false;

        QTest::newRow("Error with collection list (+ query delete)") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                    << true;
    }

    void shouldNotCrashDuringFindChildrenWhenFetchJobFailedOrEmpty()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection with two children
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("43TaskFirstChild").withParent(42).withTaskContent());
        data.createCollection(GenCollection().withId(44).withName("44NoteSecondChild").withParent(42).withNoteContent());

        // Serializer
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        Domain::DataSource::Ptr topLevelDataSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);

        QFETCH(bool, deleteQuery);
        QFETCH(int, colErrorCode);
        QFETCH(int, colFetchBehavior);
        data.storageBehavior().setFetchCollectionsErrorCode(data.collection(42).id(), colErrorCode);
        data.storageBehavior().setFetchCollectionsBehavior(data.collection(42).id(),
                                                           AkonadiFakeStorageBehavior::FetchBehavior(colFetchBehavior));

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findChildren(topLevelDataSource);

        if (deleteQuery)
            delete queries.take();

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 0);
    }

    void shouldLookInAllReportedForSearchTopLevelSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // Four top level collections, two with tasks, two with notes
        data.createCollection(GenCollection().withId(42).withName("TaskToto").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("NoteTiti").withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(44).withName("TaskTiti").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(46).withName("NoteCol").withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(47).withName("NoCol").withRootAsParent());

        // One child collection with tasks
        data.createCollection(GenCollection().withId(45).withName("TaskTotoCol").withParent(42).withTaskContent());

        QString searchTerm("Col");

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries( Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                          Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                          Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        queries->setSearchTerm(searchTerm);
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findSearchTopLevel();
        result->data();
        result = queries->findSearchTopLevel(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QString("TaskToto"));
        QCOMPARE(result->data().at(1)->name(), QString("NoteCol"));

        // WHEN
        QString searchTerm2("Titi");
        queries->setSearchTerm(searchTerm2);

        // THEN
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QString("NoteTiti"));
        QCOMPARE(result->data().at(1)->name(), QString("TaskTiti"));
    }

    void shouldReactToCollectionAddsForSearchTopLevelSources()
    {
        // GIVEN
        AkonadiFakeData data;

        QString searchTerm("col");

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        queries->setSearchTerm(searchTerm);
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findSearchTopLevel();
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createCollection(GenCollection().withId(42).withName("42Task").withRootAsParent().withTaskContent());

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first()->name(), QString("42Task"));
    }

    void shouldReactToCollectionRemovesForSearchTopLevelSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withName("TaskToto").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("NoteToto").withRootAsParent().withNoteContent());

        QString searchTerm("Toto");

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        queries->setSearchTerm(searchTerm);
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findSearchTopLevel();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.removeCollection(Akonadi::Collection(42));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first()->name(), QString("NoteToto"));
    }

    void shouldReactToItemChangesForSearchTopLevelTasks()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withName("TaskCol1").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("NoteCol2").withRootAsParent().withNoteContent());

        // One child collection
        data.createCollection(GenCollection().withId(44).withName("NoteCol2Child").withParent(43).withNoteContent());

        QString searchTerm("Col");

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries( Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                          Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                          Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        queries->setSearchTerm(searchTerm);
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findSearchTopLevel();
        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::DataSource::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyCollection(GenCollection(data.collection(43)).withName("NoteCol2Bis"));
        data.modifyCollection(GenCollection(data.collection(44)).withName("NoteCol2ChildBis"));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QString("TaskCol1"));
        QCOMPARE(result->data().at(1)->name(), QString("NoteCol2Bis"));
        QVERIFY(replaceHandlerCalled);
    }

    void shouldNotCrashDuringFindSearchTopLevelWhenFetchJobFailedOrEmpty_data()
    {
        QTest::addColumn<int>("colErrorCode");
        QTest::addColumn<int>("colFetchBehavior");
        QTest::addColumn<bool>("deleteQuery");

        QTest::newRow("No error with empty collection list") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                             << false;

        QTest::newRow("No error with empty collection list (+ query delete)") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                             << true;

        QTest::newRow("Error with empty collection list") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                          << false;

        QTest::newRow("Error with empty collection list (+ query delete)") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                          << true;

        QTest::newRow("Error with collection list") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                    << false;

        QTest::newRow("Error with collection list (+ query delete)") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                    << true;
    }

    void shouldNotCrashDuringFindSearchTopLevelWhenFetchJobFailedOrEmpty()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withNoteContent());


        auto storage = Akonadi::StorageInterface::Ptr(data.createStorage());
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        auto monitor = Akonadi::MonitorInterface::Ptr(data.createMonitor());

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(storage,
                                                                                         serializer,
                                                                                         monitor));

        QFETCH(bool, deleteQuery);
        QFETCH(int, colErrorCode);
        QFETCH(int, colFetchBehavior);
        data.storageBehavior().setFetchCollectionsErrorCode(Akonadi::Collection::root().id(), colErrorCode);
        data.storageBehavior().setFetchCollectionsBehavior(Akonadi::Collection::root().id(),
                                                           AkonadiFakeStorageBehavior::FetchBehavior(colFetchBehavior));

        // WHEN
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findSearchTopLevel();

        if (deleteQuery)
            delete queries.take();

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 0);
    }

    void shouldNotStartJobDuringFindSearchTopLevelWhenSearchTermIsEmpty()
    {
        // GIVEN
        AkonadiFakeData data;

        // one top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName("parent"));

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findSearchTopLevel();
        result->data();
        result = queries->findSearchTopLevel(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        QCOMPARE(Utils::JobHandler::jobCount(), 0);
    }

    void shouldLookInAllReportedForSearchChildSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withName("parent").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(46).withName("Col46").withRootAsParent().withNoteContent());

        // two child of parent
        data.createCollection(GenCollection().withId(43).withName("NoteCol1").withParent(42).withNoteContent());
        data.createCollection(GenCollection().withId(44).withName("TaskToto").withParent(42).withTaskContent());
        data.createCollection(GenCollection().withId(47).withName("NoCol").withParent(42));

        // One child of the first child
        data.createCollection(GenCollection().withId(45).withName("TaskCol43Child").withParent(43).withTaskContent());

        QString searchTerm("Col");

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        Domain::DataSource::Ptr parentSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        queries->setSearchTerm(searchTerm);
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findSearchChildren(parentSource);
        result->data();
        result = queries->findSearchChildren(parentSource); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QString("NoteCol1"));

        // WHEN
        QString searchTerm2("toto");
        queries->setSearchTerm(searchTerm2);

        // THEN
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QString("TaskToto"));
    }

    void shouldReactToCollectionAddsForSearchChildSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collections
        data.createCollection(GenCollection().withId(42).withName("Col1").withRootAsParent().withTaskContent());

        QString searchTerm("Col");

        // Serializer
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        Domain::DataSource::Ptr parentSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        queries->setSearchTerm(searchTerm);
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findSearchChildren(parentSource);
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createCollection(GenCollection().withId(43).withName("Col2").withParent(42).withNoteContent());

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first()->name(), QString("Col2"));
    }

    void shouldReactToCollectionRemovesForSearchChildSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withName("parent").withRootAsParent().withTaskContent());

        // two children of parent
        data.createCollection(GenCollection().withId(43).withName("NoteCol1").withParent(42).withNoteContent());
        data.createCollection(GenCollection().withId(44).withName("TaskCol2").withParent(42).withTaskContent());

        QString searchTerm("Col");

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        Domain::DataSource::Ptr parentSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        queries->setSearchTerm(searchTerm);
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findSearchChildren(parentSource);
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.removeCollection(Akonadi::Collection(43));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first()->name(), QString("TaskCol2"));
    }

    void shouldReactToCollectionChangesForSearchChildSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withName("parent").withRootAsParent().withTaskContent());

        // two children
        data.createCollection(GenCollection().withId(43).withName("NoteCol1").withParent(42).withNoteContent());
        data.createCollection(GenCollection().withId(44).withName("TaskCol2").withParent(42).withTaskContent());

        QString searchTerm("Col");

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        Domain::DataSource::Ptr parentSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        queries->setSearchTerm(searchTerm);
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findSearchChildren(parentSource);
        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::DataSource::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });

        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyCollection(GenCollection(data.collection(43)).withName("NoteCol1Bis"));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QString("NoteCol1Bis"));
        QCOMPARE(result->data().at(1)->name(), QString("TaskCol2"));
        QVERIFY(replaceHandlerCalled);
    }

    void shouldNotCrashDuringFindSearchChildrenWhenFetchJobFailedOrEmpty_data()
    {
        QTest::addColumn<int>("colErrorCode");
        QTest::addColumn<int>("colFetchBehavior");
        QTest::addColumn<bool>("deleteQuery");

        QTest::newRow("No error with empty collection list") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                             << false;

        QTest::newRow("No error with empty collection list (+ query delete)") << int(KJob::NoError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                             << true;

        QTest::newRow("Error with empty collection list") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                          << false;

        QTest::newRow("Error with empty collection list (+ query delete)") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::EmptyFetch)
                                                          << true;

        QTest::newRow("Error with collection list") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                    << false;

        QTest::newRow("Error with collection list (+ query delete)") << int(KJob::KilledJobError) << int(AkonadiFakeStorageBehavior::NormalFetch)
                                                    << true;
    }

    void shouldNotCrashDuringFindSearchChildrenWhenFetchJobFailedOrEmpty()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection with two children
        data.createCollection(GenCollection().withId(42).withName("parent").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("TaskChild").withParent(42).withTaskContent());
        data.createCollection(GenCollection().withId(44).withName("TaskChild").withParent(42).withNoteContent());

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        Domain::DataSource::Ptr parentSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);

        QFETCH(bool, deleteQuery);
        QFETCH(int, colErrorCode);
        QFETCH(int, colFetchBehavior);
        data.storageBehavior().setFetchCollectionsErrorCode(data.collection(42).id(), colErrorCode);
        data.storageBehavior().setFetchCollectionsBehavior(data.collection(42).id(),
                                                           AkonadiFakeStorageBehavior::FetchBehavior(colFetchBehavior));

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findSearchChildren(parentSource);

        if (deleteQuery)
            delete queries.take();

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 0);
    }

    void shouldNotStartJobDuringFindSearchChildrenWhenSearchTermIsEmpty()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection and its child
        data.createCollection(GenCollection().withId(42).withName("parent").withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName("child").withParent(42).withTaskContent());

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        Domain::DataSource::Ptr parentSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findSearchChildren(parentSource);

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QVERIFY(result->data().isEmpty());
    }
};

QTEST_MAIN(AkonadiDataSourceQueriesTest)

#include "akonadidatasourcequeriestest.moc"
