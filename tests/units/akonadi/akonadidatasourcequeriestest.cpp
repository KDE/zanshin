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

#include <testlib/qtest_zanshin.h>

#include "akonadi/akonadidatasourcequeries.h"
#include "akonadi/akonadiserializer.h"
#include "akonadi/akonadistoragesettings.h"

#include "utils/jobhandler.h"
#include "utils/mem_fn.h"

#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/gentodo.h"
#include "testlib/testhelpers.h"

using namespace Testlib;

typedef std::function<Domain::QueryResult<Domain::DataSource::Ptr>::Ptr(Domain::DataSourceQueries*)> QueryFunction;
Q_DECLARE_METATYPE(QueryFunction)
typedef std::function<void(Akonadi::StorageSettings *, const Akonadi::Collection &)> SetDefaultCollectionFunction;
Q_DECLARE_METATYPE(SetDefaultCollectionFunction)
typedef std::function<Akonadi::Collection(Akonadi::StorageSettings *)> GetDefaultCollectionFunction;
Q_DECLARE_METATYPE(GetDefaultCollectionFunction)

class AkonadiDataSourceQueriesTest : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiDataSourceQueriesTest(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
        qRegisterMetaType<QueryFunction>();
    }

private slots:
    void shouldCheckIfASourceIsDefaultFromSettings()
    {
        // GIVEN
        const auto minId = qint64(42);
        const auto maxId = qint64(44);
        const auto defaultId = qint64(43);

        // A default collection for saving
        Akonadi::StorageSettings::instance().setDefaultCollection(Akonadi::Collection(defaultId));

        // A few data sources
        AkonadiFakeData data;
        for (auto id = minId; id <= maxId; ++id) {
            auto col = GenCollection().withId(id).withName(QString::number(id)).withRootAsParent();
            data.createCollection(col.withTaskContent());
        }

        // WHEN
        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        // THEN
        for (auto id = minId; id <= maxId; ++id) {
            auto source = serializer->createDataSourceFromCollection(data.collection(id), Akonadi::SerializerInterface::BaseName);
            QCOMPARE(queries->isDefaultSource(source), id == defaultId);
        }
    }

    void shouldStoreDefaultSourceInTheSettingsAndNotify()
    {
        // GIVEN
        const auto minId = qint64(42);
        const auto maxId = qint64(44);
        const auto defaultId = qint64(43);

        // A default collection for saving
        Akonadi::StorageSettings::instance().setDefaultCollection(Akonadi::Collection(minId));

        // A few data sources
        AkonadiFakeData data;
        for (auto id = minId; id <= maxId; ++id) {
            auto col = GenCollection().withId(id).withName(QString::number(id)).withRootAsParent();
            data.createCollection(col.withTaskContent());
        }

        // WHEN
        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        QSignalSpy spy(queries->notifier(), &Domain::DataSourceQueriesNotifier::defaultSourceChanged);
        auto defaultSource = serializer->createDataSourceFromCollection(data.collection(defaultId), Akonadi::SerializerInterface::BaseName);
        queries->setDefaultSource(defaultSource);

        // THEN
        QCOMPARE(Akonadi::StorageSettings::instance().defaultCollection().id(), defaultId);
        QCOMPARE(spy.count(), 1);
    }

    void shouldLookInAllReportedForTopLevelSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections, one with tasks, one with notes
        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Task")).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(44).withName(QStringLiteral("44Note")).withRootAsParent().withNoteContent());

        // One with a note child collection
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43TaskChild")).withParent(42).withTaskContent());

        // One with a task child collection
        data.createCollection(GenCollection().withId(45).withName(QStringLiteral("45NoteChild")).withParent(44).withNoteContent());

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findTopLevel();
        result->data();
        result = queries->findTopLevel(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        const auto sources = result->data();
        auto actualNames = QStringList();
        std::transform(sources.constBegin(), sources.constEnd(),
                       std::back_inserter(actualNames),
                       [] (const Domain::DataSource::Ptr &source) { return source->name(); });
        actualNames.sort();

        QCOMPARE(actualNames, QStringList() << "42Task");
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
        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Task")).withRootAsParent().withTaskContent());

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42Task"));
    }

    void shouldReactToCollectionRemovesForTopLevelSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections and two child collections
        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Task")).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43Note")).withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(44).withName(QStringLiteral("43TaskChild")).withParent(42).withTaskContent());
        data.createCollection(GenCollection().withId(45).withName(QStringLiteral("43NoteChild")).withParent(43).withNoteContent());

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findTopLevel();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);

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
        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Task")).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43Note")).withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(44).withName(QStringLiteral("44NoteChild")).withParent(43).withNoteContent());

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findTopLevel();

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::DataSource::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);

        // WHEN
        data.modifyCollection(GenCollection(data.collection(42)).withName(QStringLiteral("42TaskBis")));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42TaskBis"));
        QVERIFY(replaceHandlerCalled);
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
        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Task")).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43TaskFirstChild")).withParent(42).withTaskContent());
        data.createCollection(GenCollection().withId(44).withName(QStringLiteral("44TaskFirstChildChild")).withParent(43).withTaskContent());
        data.createCollection(GenCollection().withId(45).withName(QStringLiteral("45NoteSecondChild")).withParent(42).withNoteContent());

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

        const auto sources = result->data();
        auto actualNames = QStringList();
        std::transform(sources.constBegin(), sources.constEnd(),
                       std::back_inserter(actualNames),
                       [] (const Domain::DataSource::Ptr &source) { return source->name(); });
        actualNames.sort();

        QCOMPARE(actualNames, QStringList() << QStringLiteral("43TaskFirstChild"));
    }

    void shouldReactToCollectionAddsForChildSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection with no child yet
        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Task")).withRootAsParent().withTaskContent());

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
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43TaskChild")).withParent(42).withTaskContent());

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first()->name(), QStringLiteral("43TaskChild"));
    }

    void shouldReactToCollectionRemovesForChildSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection with two children
        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Task")).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43TaskFirstChild")).withParent(42).withTaskContent());
        data.createCollection(GenCollection().withId(44).withName(QStringLiteral("45NoteSecondChild")).withParent(42).withNoteContent());

        // Serializer
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        Domain::DataSource::Ptr topLevelDataSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findChildren(topLevelDataSource);
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);

        // WHEN
        data.removeCollection(Akonadi::Collection(44));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first()->name(), QStringLiteral("43TaskFirstChild"));
    }

    void shouldReactToCollectionChangesForChildSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection with two children
        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Task")).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43TaskFirstChild")).withParent(42).withTaskContent());
        data.createCollection(GenCollection().withId(44).withName(QStringLiteral("44NoteSecondChild")).withParent(42).withNoteContent());

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
        QCOMPARE(result->data().size(), 1);

        // WHEN
        data.modifyCollection(GenCollection(data.collection(43)).withName(QStringLiteral("43TaskFirstChildBis")));

        // THEN
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first()->name(), QStringLiteral("43TaskFirstChildBis"));
        QVERIFY(replaceHandlerCalled);
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
        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Task")).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43TaskFirstChild")).withParent(42).withTaskContent());
        data.createCollection(GenCollection().withId(44).withName(QStringLiteral("44NoteSecondChild")).withParent(42).withNoteContent());

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

    void shouldLookInAllReportedForSelectedSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections, one with tasks, one with notes and two child collections
        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Task")).withRootAsParent().withTaskContent().selected(false));
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43Task")).withParent(42).withTaskContent().selected(true));
        data.createCollection(GenCollection().withId(44).withName(QStringLiteral("44Note")).withRootAsParent().withNoteContent().selected(false));
        data.createCollection(GenCollection().withId(45).withName(QStringLiteral("45Note")).withParent(44).withNoteContent().selected(true));

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findAllSelected();
        result->data();
        result = queries->findAllSelected(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        const auto sources = result->data();
        auto actualNames = QStringList();
        std::transform(sources.constBegin(), sources.constEnd(),
                       std::back_inserter(actualNames),
                       [] (const Domain::DataSource::Ptr &source) { return source->name(); });
        actualNames.sort();

        QCOMPARE(actualNames, QStringList() << QStringLiteral("42Task » 43Task"));
    }

    void shouldReactToCollectionAddsForSelectedSources()
    {
        // GIVEN
        AkonadiFakeData data;

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findAllSelected();
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Task")).withRootAsParent().withTaskContent().selected(false));
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43Task")).withParent(42).withTaskContent().selected(true));
        data.createCollection(GenCollection().withId(44).withName(QStringLiteral("44Note")).withRootAsParent().withNoteContent().selected(false));
        data.createCollection(GenCollection().withId(45).withName(QStringLiteral("45Note")).withParent(44).withNoteContent().selected(true));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42Task » 43Task"));
    }

    void shouldReactToCollectionRemovesForSelectedSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections and two child collections
        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Task")).withRootAsParent().withTaskContent().selected(false));
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43Task")).withParent(42).withTaskContent().selected(true));
        data.createCollection(GenCollection().withId(44).withName(QStringLiteral("44Note")).withRootAsParent().withNoteContent().selected(false));
        data.createCollection(GenCollection().withId(45).withName(QStringLiteral("45Note")).withParent(44).withNoteContent().selected(true));

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findAllSelected();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);

        // WHEN
        data.removeCollection(Akonadi::Collection(43));
        data.removeCollection(Akonadi::Collection(45));

        // THEN
        QCOMPARE(result->data().size(), 0);
    }

    void shouldReactToCollectionChangesForSelectedSources()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections and one child collection
        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Task")).withRootAsParent().withTaskContent().selected(false));
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43Task")).withParent(42).withTaskContent().selected(true));
        data.createCollection(GenCollection().withId(44).withName(QStringLiteral("44Note")).withRootAsParent().withNoteContent().selected(false));
        data.createCollection(GenCollection().withId(45).withName(QStringLiteral("45Note")).withParent(44).withNoteContent().selected(true));

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));

        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findAllSelected();

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::DataSource::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 1);

        // WHEN
        data.modifyCollection(GenCollection(data.collection(43)).withName(QStringLiteral("43TaskBis")));
        data.modifyCollection(GenCollection(data.collection(45)).withName(QStringLiteral("45NoteBis")));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42Task » 43TaskBis"));
        QVERIFY(replaceHandlerCalled);
    }

    void shouldNotCrashDuringFindAllSelectedWhenFetchJobFailedOrEmpty_data()
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

    void shouldNotCrashDuringFindAllSelectedWhenFetchJobFailedOrEmpty()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections and two child collections
        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Task")).withRootAsParent().withTaskContent().selected(false));
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43Task")).withParent(42).withTaskContent().selected(true));
        data.createCollection(GenCollection().withId(44).withName(QStringLiteral("44Note")).withRootAsParent().withNoteContent().selected(false));
        data.createCollection(GenCollection().withId(45).withName(QStringLiteral("45Note")).withParent(44).withNoteContent().selected(true));

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
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findAllSelected();

        if (deleteQuery)
            delete queries.take();

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 0);
    }

    void shouldLookInCollectionForProjects()
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
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        Domain::DataSource::Ptr dataSource1 = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);
        auto result1 = queries->findProjects(dataSource1);
        result1->data();
        result1 = queries->findProjects(dataSource1); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result1->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result1->data().size(), 1);
        QCOMPARE(result1->data().at(0)->name(), QStringLiteral("42"));

        // WHEN
        Domain::DataSource::Ptr dataSource2 = serializer->createDataSourceFromCollection(data.collection(43), Akonadi::SerializerInterface::BaseName);
        auto result2 = queries->findProjects(dataSource2);
        result2->data();
        result2 = queries->findProjects(dataSource2); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result2->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result1->data().size(), 1);
        QCOMPARE(result1->data().at(0)->name(), QStringLiteral("42"));

        QCOMPARE(result2->data().size(), 2);
        QCOMPARE(result2->data().at(0)->name(), QStringLiteral("43"));
        QCOMPARE(result2->data().at(1)->name(), QStringLiteral("44"));
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
        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        Domain::DataSource::Ptr dataSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);
        auto result = queries->findProjects(dataSource);
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

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        Domain::DataSource::Ptr dataSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);
        auto result = queries->findProjects(dataSource);
        TestHelpers::waitForEmptyJobQueue();
        QVERIFY(result->data().isEmpty());

        // WHEN
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")).asProject());
        data.createItem(GenTodo().withId(43).withParent(42).withTitle(QStringLiteral("43")));

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
    }

    void shouldReactToItemRemovesForProjects()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three projects in the collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")).asProject());
        data.createItem(GenTodo().withId(43).withParent(42).withTitle(QStringLiteral("43")).asProject());
        data.createItem(GenTodo().withId(44).withParent(42).withTitle(QStringLiteral("44")).asProject());

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        Domain::DataSource::Ptr dataSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);
        auto result = queries->findProjects(dataSource);
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 3);

        // WHEN
        data.removeItem(Akonadi::Item(43));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("44"));
    }

    void shouldReactToItemChangesForProjects()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three projects in the collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")).asProject());
        data.createItem(GenTodo().withId(43).withParent(42).withTitle(QStringLiteral("43")).asProject());
        data.createItem(GenTodo().withId(44).withParent(42).withTitle(QStringLiteral("44")).asProject());

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                         serializer,
                                                                                         Akonadi::MonitorInterface::Ptr(data.createMonitor())));
        Domain::DataSource::Ptr dataSource = serializer->createDataSourceFromCollection(data.collection(42), Akonadi::SerializerInterface::BaseName);
        auto result = queries->findProjects(dataSource);
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
};

ZANSHIN_TEST_MAIN(AkonadiDataSourceQueriesTest)

#include "akonadidatasourcequeriestest.moc"
