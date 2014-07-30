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

#include "domain/artifactqueries.h"
#include "domain/datasourcequeries.h"
#include "domain/noterepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/applicationmodel.h"
#include "presentation/datasourcelistmodel.h"
#include "presentation/inboxpagemodel.h"

#include "testlib/fakejob.h"

using namespace mockitopp;
using namespace mockitopp::matcher;

class ApplicationModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldDefaultToInboxPageAsCurrent()
    {
        // GIVEN
        Presentation::ApplicationModel app(0, 0, 0, 0, 0);

        // WHEN
        QObject *page = app.currentPage();

        // THEN
        QVERIFY(qobject_cast<Presentation::InboxPageModel*>(page));
    }

    void shouldProvideDataSourceModels()
    {
        // GIVEN
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        Presentation::ApplicationModel app(0, &sourceQueriesMock.getInstance(), 0, 0, 0);

        // WHEN
        auto tasks = app.taskSourcesModel();
        auto notes = app.noteSourcesModel();

        // THEN
        QVERIFY(qobject_cast<Presentation::DataSourceListModel*>(tasks));
        QVERIFY(qobject_cast<Presentation::DataSourceListModel*>(notes));
    }

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
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);

        // Dummy note repository
        mock_object<Domain::NoteRepository> noteRepositoryMock;

        // Repository mock returning the data source as default
        mock_object<Domain::TaskRepository> taskRepositoryMock;
        foreach (const Domain::DataSource::Ptr &source, provider->data()) {
            taskRepositoryMock(&Domain::TaskRepository::isDefaultSource).when(source).thenReturn(source == expectedSource);
        }

        Presentation::ApplicationModel app(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        auto source = app.defaultTaskDataSource();

        // THEN
        QCOMPARE(source, expectedSource);
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::isDefaultSource).when(expectedSource).exactly(1));
    }

    void shouldGiveFirstTaskCollectionAsDefaultIfNoneMatched()
    {
        // GIVEN

        // A list of irrelevant sources
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);

        // Dummy note repository
        mock_object<Domain::NoteRepository> noteRepositoryMock;

        // Repository mock returning the data source as default
        mock_object<Domain::TaskRepository> taskRepositoryMock;
        foreach (const Domain::DataSource::Ptr &source, provider->data()) {
            taskRepositoryMock(&Domain::TaskRepository::isDefaultSource).when(source).thenReturn(false);
        }

        Presentation::ApplicationModel app(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        auto source = app.defaultTaskDataSource();

        // THEN
        QCOMPARE(source, provider->data().first());
    }

    void shouldProvideNullPointerIfNoTaskSourceIsAvailable()
    {
        // GIVEN

        // An empty source list
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);

        // Dummy note repository
        mock_object<Domain::NoteRepository> noteRepositoryMock;

        // Repository mock returning the data source as default
        mock_object<Domain::TaskRepository> taskRepositoryMock;

        Presentation::ApplicationModel app(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        auto source = app.defaultTaskDataSource();

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
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);

        // Dummy note repository
        mock_object<Domain::NoteRepository> noteRepositoryMock;

        // Repository mock setting the default data source
        mock_object<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::setDefaultSource).when(source).thenReturn();

        Presentation::ApplicationModel app(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        app.setDefaultTaskDataSource(source);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::setDefaultSource).when(source).exactly(1));
    }

    void shouldRetrieveDefaultNoteCollectionFromRepository()
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
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(sourceResult);
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        // Dummy task repository
        mock_object<Domain::TaskRepository> taskRepositoryMock;

        // Repository mock returning the data source as default
        mock_object<Domain::NoteRepository> noteRepositoryMock;
        foreach (const Domain::DataSource::Ptr &source, provider->data()) {
            noteRepositoryMock(&Domain::NoteRepository::isDefaultSource).when(source).thenReturn(source == expectedSource);
        }

        Presentation::ApplicationModel app(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        auto source = app.defaultNoteDataSource();

        // THEN
        QCOMPARE(source, expectedSource);
        QVERIFY(noteRepositoryMock(&Domain::NoteRepository::isDefaultSource).when(expectedSource).exactly(1));
    }

    void shouldGiveFirstNoteCollectionAsDefaultIfNoneMatched()
    {
        // GIVEN

        // A list of irrelevant sources
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(sourceResult);
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        // Dummy task repository
        mock_object<Domain::TaskRepository> taskRepositoryMock;

        // Repository mock returning the data source as default
        mock_object<Domain::NoteRepository> noteRepositoryMock;
        foreach (const Domain::DataSource::Ptr &source, provider->data()) {
            noteRepositoryMock(&Domain::NoteRepository::isDefaultSource).when(source).thenReturn(false);
        }

        Presentation::ApplicationModel app(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        auto source = app.defaultNoteDataSource();

        // THEN
        QCOMPARE(source, provider->data().first());
    }

    void shouldProvideNullPointerIfNoNoteSourceIsAvailable()
    {
        // GIVEN

        // An empty source list
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(sourceResult);
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        // Dummy task repository
        mock_object<Domain::TaskRepository> taskRepositoryMock;

        // Repository mock returning the data source as default
        mock_object<Domain::NoteRepository> noteRepositoryMock;

        Presentation::ApplicationModel app(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        auto source = app.defaultNoteDataSource();

        // THEN
        QVERIFY(source.isNull());
    }

    void shouldForwardDefaultNoteCollectionToRepository()
    {
        // GIVEN

        // A data source
        auto source = Domain::DataSource::Ptr::create();

        // A dummy source list
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(sourceResult);
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        // Dummy task repository
        mock_object<Domain::TaskRepository> taskRepositoryMock;

        // Repository mock setting the default data source
        mock_object<Domain::NoteRepository> noteRepositoryMock;
        noteRepositoryMock(&Domain::NoteRepository::setDefaultSource).when(source).thenReturn();

        Presentation::ApplicationModel app(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        app.setDefaultNoteDataSource(source);

        // THEN
        QVERIFY(noteRepositoryMock(&Domain::NoteRepository::setDefaultSource).when(source).exactly(1));
    }
};

QTEST_MAIN(ApplicationModelTest)

#include "applicationmodeltest.moc"
