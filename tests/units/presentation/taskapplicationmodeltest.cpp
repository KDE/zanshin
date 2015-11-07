/* This file is part of Zanshin

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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

#include "utils/mockobject.h"

#define ZANSHIN_I_SWEAR_I_AM_IN_A_PRESENTATION_TEST

#include "presentation/availabletaskpagesmodel.h"
#include "presentation/datasourcelistmodel.h"
#include "presentation/taskapplicationmodel.h"

class TaskApplicationModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldProvideAvailableTaskPagesModel()
    {
        // GIVEN
        auto projectQueries = Domain::ProjectQueries::Ptr();
        auto projectRepository = Domain::ProjectRepository::Ptr();
        auto contextQueries = Domain::ContextQueries::Ptr();
        auto contextRepository = Domain::ContextRepository::Ptr();
        auto sourceQueries = Domain::DataSourceQueries::Ptr();
        auto sourceRepository = Domain::DataSourceRepository::Ptr();
        auto taskQueries = Domain::TaskQueries::Ptr();
        auto taskRepository = Domain::TaskRepository::Ptr();
        auto noteRepository = Domain::NoteRepository::Ptr();
        auto tagQueries = Domain::TagQueries::Ptr();
        auto tagRepository = Domain::TagRepository::Ptr();
        Presentation::TaskApplicationModel app(projectQueries,
                                               projectRepository,
                                               contextQueries,
                                               contextRepository,
                                               sourceQueries,
                                               sourceRepository,
                                               taskQueries,
                                               taskRepository,
                                               noteRepository,
                                               tagQueries,
                                               tagRepository);

        // WHEN
        QObject *available = app.availablePages();

        // THEN
        QVERIFY(qobject_cast<Presentation::AvailableTaskPagesModel*>(available));
    }

    void shouldProvideDataSourceModels()
    {
        // GIVEN
        Utils::MockObject<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        auto projectQueries = Domain::ProjectQueries::Ptr();
        auto projectRepository = Domain::ProjectRepository::Ptr();
        auto contextQueries = Domain::ContextQueries::Ptr();
        auto contextRepository = Domain::ContextRepository::Ptr();
        auto sourceRepository = Domain::DataSourceRepository::Ptr();
        auto taskQueries = Domain::TaskQueries::Ptr();
        auto taskRepository = Domain::TaskRepository::Ptr();
        auto noteRepository = Domain::NoteRepository::Ptr();
        auto tagQueries = Domain::TagQueries::Ptr();
        auto tagRepository = Domain::TagRepository::Ptr();
        Presentation::TaskApplicationModel app(projectQueries,
                                               projectRepository,
                                               contextQueries,
                                               contextRepository,
                                               sourceQueriesMock.getInstance(),
                                               sourceRepository,
                                               taskQueries,
                                               taskRepository,
                                               noteRepository,
                                               tagQueries,
                                               tagRepository);

        // WHEN
        auto tasks = app.dataSourcesModel();

        // THEN
        QVERIFY(qobject_cast<Presentation::DataSourceListModel*>(tasks));
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
        Utils::MockObject<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);
        foreach (const Domain::DataSource::Ptr &source, provider->data()) {
            sourceQueriesMock(&Domain::DataSourceQueries::isDefaultSource).when(source).thenReturn(source == expectedSource);
        }

        auto projectQueries = Domain::ProjectQueries::Ptr();
        auto projectRepository = Domain::ProjectRepository::Ptr();
        auto contextQueries = Domain::ContextQueries::Ptr();
        auto contextRepository = Domain::ContextRepository::Ptr();
        auto sourceRepository = Domain::DataSourceRepository::Ptr();
        auto taskQueries = Domain::TaskQueries::Ptr();
        auto taskRepository = Domain::TaskRepository::Ptr();
        auto noteRepository = Domain::NoteRepository::Ptr();
        auto tagQueries = Domain::TagQueries::Ptr();
        auto tagRepository = Domain::TagRepository::Ptr();
        Presentation::TaskApplicationModel app(projectQueries,
                                               projectRepository,
                                               contextQueries,
                                               contextRepository,
                                               sourceQueriesMock.getInstance(),
                                               sourceRepository,
                                               taskQueries,
                                               taskRepository,
                                               noteRepository,
                                               tagQueries,
                                               tagRepository);

        // WHEN
        auto source = app.defaultDataSource();

        // THEN
        QCOMPARE(source, expectedSource);
        QVERIFY(sourceQueriesMock(&Domain::DataSourceQueries::isDefaultSource).when(expectedSource).exactly(1));
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
        Utils::MockObject<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);
        foreach (const Domain::DataSource::Ptr &source, provider->data()) {
            sourceQueriesMock(&Domain::DataSourceQueries::isDefaultSource).when(source).thenReturn(false);
        }

        auto projectQueries = Domain::ProjectQueries::Ptr();
        auto projectRepository = Domain::ProjectRepository::Ptr();
        auto contextQueries = Domain::ContextQueries::Ptr();
        auto contextRepository = Domain::ContextRepository::Ptr();
        auto sourceRepository = Domain::DataSourceRepository::Ptr();
        auto taskQueries = Domain::TaskQueries::Ptr();
        auto taskRepository = Domain::TaskRepository ::Ptr();
        auto noteRepository = Domain::NoteRepository::Ptr();
        auto tagQueries = Domain::TagQueries::Ptr();
        auto tagRepository = Domain::TagRepository::Ptr();
        Presentation::TaskApplicationModel app(projectQueries,
                                               projectRepository,
                                               contextQueries,
                                               contextRepository,
                                               sourceQueriesMock.getInstance(),
                                               sourceRepository,
                                               taskQueries,
                                               taskRepository,
                                               noteRepository,
                                               tagQueries,
                                               tagRepository);

        // WHEN
        auto source = app.defaultDataSource();

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
        Utils::MockObject<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);

        auto projectQueries = Domain::ProjectQueries::Ptr();
        auto projectRepository = Domain::ProjectRepository::Ptr();
        auto contextQueries = Domain::ContextQueries::Ptr();
        auto contextRepository = Domain::ContextRepository::Ptr();
        auto sourceRepository = Domain::DataSourceRepository::Ptr();
        auto taskQueries = Domain::TaskQueries::Ptr();
        auto taskRepository = Domain::TaskRepository::Ptr();
        auto noteRepository = Domain::NoteRepository::Ptr();
        auto tagQueries = Domain::TagQueries::Ptr();
        auto tagRepository = Domain::TagRepository::Ptr();
        Presentation::TaskApplicationModel app(projectQueries,
                                               projectRepository,
                                               contextQueries,
                                               contextRepository,
                                               sourceQueriesMock.getInstance(),
                                               sourceRepository,
                                               taskQueries,
                                               taskRepository,
                                               noteRepository,
                                               tagQueries,
                                               tagRepository);

        // WHEN
        auto source = app.defaultDataSource();

        // THEN
        QVERIFY(source.isNull());
    }

    void shouldForwardDefaultTaskCollectionToQueries()
    {
        // GIVEN

        // A data source
        auto source = Domain::DataSource::Ptr::create();

        // A dummy source list
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        Utils::MockObject<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);
        sourceQueriesMock(&Domain::DataSourceQueries::isDefaultSource).when(source).thenReturn(false);
        sourceQueriesMock(&Domain::DataSourceQueries::changeDefaultSource).when(source).thenReturn();

        auto projectQueries = Domain::ProjectQueries::Ptr();
        auto projectRepository = Domain::ProjectRepository::Ptr();
        auto contextQueries = Domain::ContextQueries::Ptr();
        auto contextRepository = Domain::ContextRepository::Ptr();
        auto sourceRepository = Domain::DataSourceRepository::Ptr();
        auto taskQueries = Domain::TaskQueries::Ptr();
        auto taskRepository = Domain::TaskRepository::Ptr();
        auto noteRepository = Domain::NoteRepository::Ptr();
        auto tagQueries = Domain::TagQueries::Ptr();
        auto tagRepository = Domain::TagRepository::Ptr();
        Presentation::TaskApplicationModel app(projectQueries,
                                               projectRepository,
                                               contextQueries,
                                               contextRepository,
                                               sourceQueriesMock.getInstance(),
                                               sourceRepository,
                                               taskQueries,
                                               taskRepository,
                                               noteRepository,
                                               tagQueries,
                                               tagRepository);


        // WHEN
        app.setDefaultDataSource(source);

        // THEN
        QVERIFY(sourceQueriesMock(&Domain::DataSourceQueries::changeDefaultSource).when(source).exactly(1));
    }
};

QTEST_MAIN(TaskApplicationModelTest)

#include "taskapplicationmodeltest.moc"
