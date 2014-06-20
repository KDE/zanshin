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

#include <mockitopp/mockitopp.hpp>

#include "domain/datasourcequeries.h"
#include "domain/taskrepository.h"
#include "presentation/inboxmodel.h"

using namespace mockitopp;

class InboxModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldRetrieveDefaultTaskCollectionFromRepository()
    {
        // GIVEN

        // A data source
        auto expectedSource = Domain::DataSource::Ptr::create();

        // A source list containing the source we expect as default
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(expectedSource);
        provider->append(Domain::DataSource::Ptr::create());
        auto sourceResult = Domain::QueryResultProvider<Domain::DataSource::Ptr>::createResult(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);

        // Repository mock returning the data source as default
        mock_object<Domain::TaskRepository> taskRepositoryMock;
        foreach (const Domain::DataSource::Ptr &source, provider->data()) {
            taskRepositoryMock(&Domain::TaskRepository::isDefaultSource).when(source).thenReturn(source == expectedSource);
        }

        Presentation::InboxModel inbox(&sourceQueriesMock.getInstance(),
                                       &taskRepositoryMock.getInstance());

        // WHEN
        auto source = inbox.defaultTaskDataSource();

        // THEN
        QCOMPARE(source, expectedSource);
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::isDefaultSource).when(expectedSource).exactly(1));
    }

    void shouldGiveFirstCollectionAsDefaultIfNoneMatched()
    {
        // GIVEN

        // A list of irrelevant sources
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        auto sourceResult = Domain::QueryResultProvider<Domain::DataSource::Ptr>::createResult(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);

        // Repository mock returning the data source as default
        mock_object<Domain::TaskRepository> taskRepositoryMock;
        foreach (const Domain::DataSource::Ptr &source, provider->data()) {
            taskRepositoryMock(&Domain::TaskRepository::isDefaultSource).when(source).thenReturn(false);
        }

        Presentation::InboxModel inbox(&sourceQueriesMock.getInstance(),
                                       &taskRepositoryMock.getInstance());

        // WHEN
        auto source = inbox.defaultTaskDataSource();

        // THEN
        QCOMPARE(source, provider->data().first());
    }

    void shouldProvideNullPointerIfNoSourceIsAvailable()
    {
        // GIVEN

        // An empty source list
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResultProvider<Domain::DataSource::Ptr>::createResult(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);

        // Repository mock returning the data source as default
        mock_object<Domain::TaskRepository> taskRepositoryMock;

        Presentation::InboxModel inbox(&sourceQueriesMock.getInstance(),
                                       &taskRepositoryMock.getInstance());

        // WHEN
        auto source = inbox.defaultTaskDataSource();

        // THEN
        QVERIFY(source.isNull());
    }

    void shouldForwardDefaultTaskCollectionToRepository()
    {
        // GIVEN

        // A data source
        auto source = Domain::DataSource::Ptr::create();

        // A dummy source list
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResultProvider<Domain::DataSource::Ptr>::createResult(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);

        // Repository mock setting the default data source
        mock_object<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::setDefaultSource).when(source).thenReturn();

        Presentation::InboxModel inbox(&sourceQueriesMock.getInstance(),
                                       &taskRepositoryMock.getInstance());

        // WHEN
        inbox.setDefaultTaskDataSource(source);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::setDefaultSource).when(source).exactly(1));
    }
};

QTEST_MAIN(InboxModelTest)

#include "inboxmodeltest.moc"
