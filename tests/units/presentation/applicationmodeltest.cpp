/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "utils/dependencymanager.h"
#include "utils/jobhandler.h"
#include "utils/mockobject.h"

#include "presentation/applicationmodel.h"
#include "presentation/availablepagesmodel.h"
#include "presentation/availablesourcesmodel.h"
#include "presentation/editormodel.h"
#include "presentation/pagemodel.h"
#include "presentation/errorhandler.h"
#include "presentation/runningtaskmodel.h"

#include "testlib/fakejob.h"

#include <QSignalSpy>

using namespace mockitopp;
using namespace mockitopp::matcher;

class FakeErrorHandler : public Presentation::ErrorHandler
{
public:
    void doDisplayMessage(const QString &message) override
    {
        m_message = message;
    }

    QString m_message;
};

class FakePageModel : public Presentation::PageModel
{
    Q_OBJECT
public:
    explicit FakePageModel(QObject *parent = nullptr)
        : Presentation::PageModel(parent) {}

    Domain::Task::Ptr addItem(const QString &, const QModelIndex &) override { return {}; }
    void removeItem(const QModelIndex &) override {}
    void promoteItem(const QModelIndex &) override {}

private:
    QAbstractItemModel *createCentralListModel() override { return {}; }
};

class ApplicationModelTest : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationModelTest(QObject *parent = nullptr)
        : QObject(parent)
    {
        Utils::DependencyManager::globalInstance().add<Presentation::AvailablePagesModel>(
            [] (Utils::DependencyManager *) {
                return new Presentation::AvailablePagesModel(Domain::DataSourceQueries::Ptr(),
                                                             Domain::ProjectQueries::Ptr(),
                                                             Domain::ProjectRepository::Ptr(),
                                                             Domain::ContextQueries::Ptr(),
                                                             Domain::ContextRepository::Ptr(),
                                                             Domain::TaskQueries::Ptr(),
                                                             Domain::TaskRepository::Ptr());
        });
        Utils::DependencyManager::globalInstance().add<Presentation::EditorModel>(
            [] (Utils::DependencyManager *) {
                return new Presentation::EditorModel;
        });
        Utils::DependencyManager::globalInstance().add<Presentation::AvailableSourcesModel>(
            [] (Utils::DependencyManager *) {
                return new Presentation::AvailableSourcesModel(Domain::DataSourceQueries::Ptr(),
                                                               Domain::DataSourceRepository::Ptr());
        });
        Utils::DependencyManager::globalInstance().add<Presentation::RunningTaskModel>(
            [] (Utils::DependencyManager *) {
                return new Presentation::RunningTaskModel(Domain::TaskQueries::Ptr(),
                                                         Domain::TaskRepository::Ptr());
        });
    }

private slots:
    void shouldProvideAvailableSourcesModel()
    {
        // GIVEN
        Presentation::ApplicationModel app;

        // WHEN
        QObject *available = app.availableSources();

        // THEN
        QVERIFY(qobject_cast<Presentation::AvailableSourcesModel*>(available));
    }

    void shouldProvideAvailablePagesModel()
    {
        // GIVEN
        Presentation::ApplicationModel app;

        // WHEN
        QObject *available = app.availablePages();

        // THEN
        QVERIFY(qobject_cast<Presentation::AvailablePagesModel*>(available));
    }

    void shouldProvideCurrentPage()
    {
        // GIVEN
        Presentation::ApplicationModel app;
        QVERIFY(!app.currentPage());
        QSignalSpy spy(&app, &Presentation::ApplicationModel::currentPageChanged);

        // WHEN
        auto page = new FakePageModel(this);
        app.setCurrentPage(page);

        // THEN
        QCOMPARE(app.currentPage(), page);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).value<QObject*>(), page);
    }

    void shouldSupportNullPage()
    {
        // GIVEN
        Presentation::ApplicationModel app;
        auto page = new FakePageModel(this);
        app.setCurrentPage(page);
        QCOMPARE(app.currentPage(), page);
        QSignalSpy spy(&app, &Presentation::ApplicationModel::currentPageChanged);

        // WHEN
        app.setCurrentPage(nullptr);

        // THEN
        QVERIFY(!app.currentPage());
        QCOMPARE(spy.count(), 1);
        QVERIFY(!spy.takeFirst().at(0).value<QObject*>());
    }

    void shouldTakeOwnershipOfCurrentPage()
    {
        // GIVEN
        auto page = QPointer<QObject>(new FakePageModel(this));

        {
            Presentation::ApplicationModel app;

            // WHEN
            app.setCurrentPage(page.data());

            // THEN
            QVERIFY(!page->parent());
            QCOMPARE(app.currentPage(), page.data());
        }
        // We don't crash and page is deleted
        QVERIFY(!page);
    }

    void shouldProvideEditorModel()
    {
        // GIVEN
        Presentation::ApplicationModel app;

        // WHEN
        QObject *page = app.editor();

        // THEN
        QVERIFY(qobject_cast<Presentation::EditorModel*>(page));
    }

    void shouldProvideRunningTaskModel()
    {
        // GIVEN
        Presentation::ApplicationModel app;

        // WHEN
        QObject *model = app.runningTaskModel();

        // THEN
        QVERIFY(qobject_cast<Presentation::RunningTaskModel*>(model));
    }

    void shouldSetErrorHandlerToAllModels()
    {
        // GIVEN

        // An ErrorHandler
        FakeErrorHandler errorHandler;
        Presentation::ApplicationModel app;
        app.setCurrentPage(new FakePageModel);

        // WHEN
        app.setErrorHandler(&errorHandler);

        // THEN
        auto availableSource = static_cast<Presentation::AvailableSourcesModel*>(app.availableSources());
        auto availablePages = static_cast<Presentation::AvailablePagesModel*>(app.availablePages());
        auto editor = static_cast<Presentation::EditorModel*>(app.editor());
        auto page = static_cast<Presentation::PageModel*>(app.currentPage());
        auto runningTask = static_cast<Presentation::RunningTaskModel*>(app.runningTaskModel());
        QCOMPARE(availableSource->errorHandler(), &errorHandler);
        QCOMPARE(availablePages->errorHandler(), &errorHandler);
        QCOMPARE(editor->errorHandler(), &errorHandler);
        QCOMPARE(page->errorHandler(), &errorHandler);
        QCOMPARE(runningTask->errorHandler(), &errorHandler);

        // WHEN
        FakeErrorHandler errorHandler2;

        app.setErrorHandler(&errorHandler2);

        // THEN
        QCOMPARE(availableSource->errorHandler(), &errorHandler2);
        QCOMPARE(availablePages->errorHandler(), &errorHandler2);
        QCOMPARE(editor->errorHandler(), &errorHandler2);
        QCOMPARE(page->errorHandler(), &errorHandler2);
        QCOMPARE(runningTask->errorHandler(), &errorHandler2);
    }

    void shouldClearJobHandlersOnExit()
    {
        // GIVEN
        auto app = new Presentation::ApplicationModel;
        Utils::JobHandler::install(new FakeJob, [] { qFatal("Shouldn't happen"); });
        QCOMPARE(Utils::JobHandler::jobCount(), 1);

        // WHEN
        delete app;

        // THEN
        QCOMPARE(Utils::JobHandler::jobCount(), 0);
        QTest::qWait(FakeJob::DURATION * 2);
    }
};

ZANSHIN_TEST_MAIN(ApplicationModelTest)

#include "applicationmodeltest.moc"
