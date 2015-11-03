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

#include "utils/mockobject.h"

#include "domain/datasourcequeries.h"
#include "domain/noterepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/applicationmodel.h"
#include "presentation/artifacteditormodel.h"
#include "presentation/availablepagesmodelinterface.h"
#include "presentation/availablesourcesmodel.h"
#include "presentation/datasourcelistmodel.h"
#include "presentation/errorhandler.h"

#include "testlib/fakejob.h"

using namespace mockitopp;
using namespace mockitopp::matcher;

class FakeErrorHandler : public Presentation::ErrorHandler
{
public:
    void doDisplayMessage(const QString &message)
    {
        m_message = message;
    }

    QString m_message;
};

class FakeAvailablePagesModel : public Presentation::AvailablePagesModelInterface
{
    Q_OBJECT
public:
    explicit FakeAvailablePagesModel(QObject *parent = Q_NULLPTR)
        : Presentation::AvailablePagesModelInterface(parent) {}

    QAbstractItemModel *pageListModel() Q_DECL_OVERRIDE { return Q_NULLPTR; }

    QObject *createPageForIndex(const QModelIndex &) Q_DECL_OVERRIDE { return Q_NULLPTR; }

    void addProject(const QString &, const Domain::DataSource::Ptr &) Q_DECL_OVERRIDE {}
    void addContext(const QString &) Q_DECL_OVERRIDE {}
    void addTag(const QString &) Q_DECL_OVERRIDE {}
    void removeItem(const QModelIndex &) Q_DECL_OVERRIDE {}
};

class ApplicationModel : public Presentation::ApplicationModel
{
    Q_OBJECT
public:
    explicit ApplicationModel(const Domain::ProjectQueries::Ptr &projectQueries,
                              const Domain::ProjectRepository::Ptr &projectRepository,
                              const Domain::ContextQueries::Ptr &contextQueries,
                              const Domain::ContextRepository::Ptr &contextRepository,
                              const Domain::DataSourceQueries::Ptr &sourceQueries,
                              const Domain::DataSourceRepository::Ptr &sourceRepository,
                              const Domain::TaskQueries::Ptr &taskQueries,
                              const Domain::TaskRepository::Ptr &taskRepository,
                              const Domain::NoteRepository::Ptr &noteRepository,
                              const Domain::TagQueries::Ptr &tagQueries,
                              const Domain::TagRepository::Ptr &tagRepository,
                              QObject *parent = Q_NULLPTR)
        : Presentation::ApplicationModel(projectQueries,
                                         projectRepository,
                                         contextQueries,
                                         contextRepository,
                                         sourceQueries,
                                         sourceRepository,
                                         taskQueries,
                                         taskRepository,
                                         noteRepository,
                                         tagQueries,
                                         tagRepository,
                                         parent)
    {
    }

public slots:
    void setDefaultDataSource(const Domain::DataSource::Ptr &)
    {
    }

private:
    Domain::QueryResult<Domain::DataSource::Ptr>::Ptr createDataSourceQueryResult() Q_DECL_OVERRIDE
    {
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        return Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);
    }

    bool isDefaultSource(const Domain::DataSource::Ptr &) Q_DECL_OVERRIDE
    {
        return false;
    }

    Presentation::AvailablePagesModelInterface *createAvailablePagesModel() Q_DECL_OVERRIDE
    {
        return new FakeAvailablePagesModel(this);
    }
};

class ApplicationModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldProvideAvailableSourcesModel()
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
        ApplicationModel app(projectQueries,
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
        QObject *available = app.availableSources();

        // THEN
        QVERIFY(qobject_cast<Presentation::AvailableSourcesModel*>(available));
    }

    void shouldProvideAvailablePagesModel()
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
        ApplicationModel app(projectQueries,
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
        QVERIFY(qobject_cast<FakeAvailablePagesModel*>(available));
    }

    void shouldProvideCurrentPage()
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
        ApplicationModel app(projectQueries,
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
        QVERIFY(!app.currentPage());
        QSignalSpy spy(&app, SIGNAL(currentPageChanged(QObject*)));

        // WHEN
        auto page = new QObject(this);
        app.setCurrentPage(page);

        // THEN
        QCOMPARE(app.currentPage(), page);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().takeFirst().value<QObject*>(), page);
    }

    void shouldAllowChangingPage()
    {
        // GIVEN
    }

    void shouldProvideArtifactEditorModel()
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
        ApplicationModel app(projectQueries,
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
        QObject *page = app.editor();

        // THEN
        QVERIFY(qobject_cast<Presentation::ArtifactEditorModel*>(page));
    }

    void shouldSetErrorHandlerToAllModels()
    {
        // GIVEN

        // An ErrorHandler
        FakeErrorHandler errorHandler;

        auto projectQueries = Domain::ProjectQueries::Ptr();
        auto projectRepository = Domain::ProjectRepository::Ptr();
        auto contextQueries = Domain::ContextQueries::Ptr();
        auto contextRepository = Domain::ContextRepository::Ptr();
        auto noteRepository = Domain::NoteRepository::Ptr();
        auto sourceQueries = Domain::DataSourceQueries::Ptr();
        auto sourceRepository = Domain::DataSourceRepository::Ptr();
        auto taskQueries = Domain::TaskQueries::Ptr();
        auto taskRepository = Domain::TaskRepository::Ptr();
        auto tagQueries = Domain::TagQueries::Ptr();
        auto tagRepository = Domain::TagRepository::Ptr();
        ApplicationModel app(projectQueries,
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
        app.setErrorHandler(&errorHandler);

        // THEN
        auto availableSource = static_cast<Presentation::AvailableSourcesModel*>(app.availableSources());
        auto availablePages = static_cast<FakeAvailablePagesModel*>(app.availablePages());
        auto editor = static_cast<Presentation::ArtifactEditorModel*>(app.editor());
        QCOMPARE(availableSource->errorHandler(), &errorHandler);
        QCOMPARE(availablePages->errorHandler(), &errorHandler);
        QCOMPARE(editor->errorHandler(), &errorHandler);

        // WHEN
        FakeErrorHandler errorHandler2;

        app.setErrorHandler(&errorHandler2);

        // THEN
        QCOMPARE(availableSource->errorHandler(), &errorHandler2);
        QCOMPARE(availablePages->errorHandler(), &errorHandler2);
        QCOMPARE(editor->errorHandler(), &errorHandler2);
    }
};

QTEST_MAIN(ApplicationModelTest)

#include "applicationmodeltest.moc"
