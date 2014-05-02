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

#include <functional>

#include <mockitopp/mockitopp.hpp>

#include "testlib/akonadimocks.h"

#include "akonadi/akonadidatasourcequeries.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;

typedef std::function<Domain::QueryResult<Domain::DataSource::Ptr>::Ptr(Domain::DataSourceQueries*)> QueryFunction;
Q_DECLARE_METATYPE(QueryFunction)
Q_DECLARE_METATYPE(Akonadi::StorageInterface::FetchContentType)

class AkonadiDataSourceQueriesTest : public QObject
{
    Q_OBJECT
public:
    AkonadiDataSourceQueriesTest(QObject *parent = 0)
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
            QueryFunction query = std::mem_fn(&Domain::DataSourceQueries::findNotes);
            QTest::newRow("notes") << Akonadi::StorageInterface::Notes << query;
        }

        {
            QueryFunction query = std::mem_fn(&Domain::DataSourceQueries::findTasks);
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
        QFETCH(Akonadi::StorageInterface::FetchContentType, contentType);
        QFETCH(QueryFunction, queryFunction);

        // Two top level collections and one child collection
        Akonadi::Collection col1(42);
        col1.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource1(new Domain::DataSource);
        Akonadi::Collection col2(43);
        col2.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource2(new Domain::DataSource);
        Akonadi::Collection col3(44);
        col3.setParentCollection(col2);
        Domain::DataSource::Ptr dataSource3(new Domain::DataSource);
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2 << col3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       contentType)
                                                                 .thenReturn(collectionFetchJob);

        // Serializer mock returning the data sources from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).thenReturn(dataSource1);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).thenReturn(dataSource2);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).thenReturn(dataSource3);

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(&storageMock.getInstance(),
                                                                                         &serializerMock.getInstance(),
                                                                                         new MockMonitor(this)));
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queryFunction(queries.data());

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               contentType)
                                                                         .exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).exactly(1));

        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0), dataSource1);
        QCOMPARE(result->data().at(1), dataSource2);
        QCOMPARE(result->data().at(2), dataSource3);
    }

    void shouldIgnoreCollectionsWhichAreNotDataSources_data()
    {
        generateDataTable();
    }

    void shouldIgnoreCollectionsWhichAreNotDataSources()
    {
        // GIVEN
        QFETCH(Akonadi::StorageInterface::FetchContentType, contentType);
        QFETCH(QueryFunction, queryFunction);

        // Two top level collections and one child collection
        Akonadi::Collection col1(42);
        col1.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource1(new Domain::DataSource);
        // One of the collections not being a data source
        Akonadi::Collection col2(43);
        col2.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource2;
        Akonadi::Collection col3(44);
        col3.setParentCollection(col2);
        Domain::DataSource::Ptr dataSource3(new Domain::DataSource);
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2 << col3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       contentType)
                                                                 .thenReturn(collectionFetchJob);

        // Serializer mock returning the data sources from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).thenReturn(dataSource1);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).thenReturn(dataSource2);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).thenReturn(dataSource3);

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(&storageMock.getInstance(),
                                                                                         &serializerMock.getInstance(),
                                                                                         new MockMonitor(this)));
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queryFunction(queries.data());

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               contentType)
                                                                         .exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), dataSource1);
        QCOMPARE(result->data().at(1), dataSource3);
    }

    void shouldReactToCollectionAddsForCompatibleDataSourcesOnly_data()
    {
        generateDataTable();
    }

    void shouldReactToCollectionAddsForCompatibleDataSourcesOnly()
    {
        // GIVEN
        QFETCH(Akonadi::StorageInterface::FetchContentType, contentType);
        QFETCH(QueryFunction, queryFunction);

        // Empty collection fetch
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       contentType)
                                                                 .thenReturn(collectionFetchJob);

        // Serializer mock returning the data sources from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(&storageMock.getInstance(),
                                                                                         &serializerMock.getInstance(),
                                                                                         monitor));
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queryFunction(queries.data());
        QTest::qWait(150);
        QVERIFY(result->data().isEmpty());

        // WHEN
        Akonadi::Collection col1(42);
        Domain::DataSource::Ptr dataSource1(new Domain::DataSource);
        Akonadi::Collection col2(43);
        Domain::DataSource::Ptr dataSource2;
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).thenReturn(dataSource1);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).thenReturn(dataSource2);
        serializerMock(&Akonadi::SerializerInterface::isTaskCollection).when(col1).thenReturn(contentType == Akonadi::StorageInterface::Tasks);
        serializerMock(&Akonadi::SerializerInterface::isTaskCollection).when(col2).thenReturn(false);
        serializerMock(&Akonadi::SerializerInterface::isNoteCollection).when(col1).thenReturn(contentType == Akonadi::StorageInterface::Notes);
        serializerMock(&Akonadi::SerializerInterface::isNoteCollection).when(col2).thenReturn(false);
        monitor->addCollection(col1);
        monitor->addCollection(col2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               contentType)
                                                                         .exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::isTaskCollection).when(col1).atMost(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::isTaskCollection).when(col2).atMost(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::isNoteCollection).when(col1).atMost(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::isNoteCollection).when(col2).atMost(1));

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first(), dataSource1);
    }

    void shouldReactToCollectionRemovesForDataSources_data()
    {
        generateDataTable();
    }

    void shouldReactToCollectionRemovesForDataSources()
    {
        // GIVEN
        QFETCH(Akonadi::StorageInterface::FetchContentType, contentType);
        QFETCH(QueryFunction, queryFunction);

        // Two top level collections and one child collection
        Akonadi::Collection col1(42);
        col1.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource1(new Domain::DataSource);
        Akonadi::Collection col2(43);
        col2.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource2(new Domain::DataSource);
        Akonadi::Collection col3(44);
        col3.setParentCollection(col2);
        Domain::DataSource::Ptr dataSource3(new Domain::DataSource);
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2 << col3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       contentType)
                                                                 .thenReturn(collectionFetchJob);

        // Serializer mock returning the data sources from the collections
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).thenReturn(dataSource1);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).thenReturn(dataSource2);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).thenReturn(dataSource3);

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(&storageMock.getInstance(),
                                                                                         &serializerMock.getInstance(),
                                                                                         monitor));
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queryFunction(queries.data());
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 3);

        // WHEN
        monitor->removeCollection(col3);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               contentType)
                                                                         .exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), dataSource1);
        QCOMPARE(result->data().at(1), dataSource2);
    }

    void shouldReactToCollectionChangesForDataSources_data()
    {
        generateDataTable();
    }

    void shouldReactToCollectionChangesForDataSources()
    {
        // GIVEN
        QFETCH(Akonadi::StorageInterface::FetchContentType, contentType);
        QFETCH(QueryFunction, queryFunction);

        // Two top level collections and one child collection
        Akonadi::Collection col1(42);
        col1.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource1(new Domain::DataSource);
        Akonadi::Collection col2(43);
        col2.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource2(new Domain::DataSource);
        Akonadi::Collection col3(44);
        col3.setParentCollection(col2);
        Domain::DataSource::Ptr dataSource3(new Domain::DataSource);
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2 << col3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       contentType)
                                                                 .thenReturn(collectionFetchJob);

        // Serializer mock returning the data sources from the collections
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).thenReturn(dataSource1);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).thenReturn(dataSource2);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).thenReturn(dataSource3);
        serializerMock(&Akonadi::SerializerInterface::updateDataSourceFromCollection).when(dataSource2, col2).thenReturn();

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(&storageMock.getInstance(),
                                                                                         &serializerMock.getInstance(),
                                                                                         monitor));
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queryFunction(queries.data());
        // Even though the pointer didn't change it's convenient to user if we call
        // the replace handlers
        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::DataSource::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 3);

        // WHEN
        monitor->changeCollection(col2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               contentType)
                                                                         .exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::updateDataSourceFromCollection).when(dataSource2, col2).exactly(1));

        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0), dataSource1);
        QCOMPARE(result->data().at(1), dataSource2);
        QCOMPARE(result->data().at(2), dataSource3);
        QVERIFY(replaceHandlerCalled);
    }
};

QTEST_MAIN(AkonadiDataSourceQueriesTest)

#include "akonadidatasourcequeriestest.moc"
