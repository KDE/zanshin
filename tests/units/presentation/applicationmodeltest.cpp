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

#include "utils/dependencymanager.h"
#include "utils/mockobject.h"

#include "presentation/applicationmodel.h"
#include "presentation/artifacteditormodel.h"
#include "presentation/availablepagesmodelinterface.h"
#include "presentation/availablesourcesmodel.h"
#include "presentation/pagemodel.h"
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

    bool hasProjectPages() const Q_DECL_OVERRIDE { return false; }
    bool hasContextPages() const Q_DECL_OVERRIDE { return false; }
    bool hasTagPages() const Q_DECL_OVERRIDE { return false; }

    QObject *createPageForIndex(const QModelIndex &) Q_DECL_OVERRIDE { return Q_NULLPTR; }

    void addProject(const QString &, const Domain::DataSource::Ptr &) Q_DECL_OVERRIDE {}
    void addContext(const QString &) Q_DECL_OVERRIDE {}
    void addTag(const QString &) Q_DECL_OVERRIDE {}
    void removeItem(const QModelIndex &) Q_DECL_OVERRIDE {}
};

class FakePageModel : public Presentation::PageModel
{
    Q_OBJECT
public:
    explicit FakePageModel(QObject *parent = Q_NULLPTR)
        : Presentation::PageModel(parent) {}

    Domain::Artifact::Ptr addItem(const QString &) Q_DECL_OVERRIDE { return {}; }
    void removeItem(const QModelIndex &) Q_DECL_OVERRIDE {}
    void promoteItem(const QModelIndex &) Q_DECL_OVERRIDE {}

private:
    QAbstractItemModel *createCentralListModel() Q_DECL_OVERRIDE { return {}; }
};

class ApplicationModelTest : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationModelTest(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
        Utils::DependencyManager::globalInstance().add<Presentation::AvailablePagesModelInterface,
                                                       FakeAvailablePagesModel>();
        Utils::DependencyManager::globalInstance().add<Presentation::ArtifactEditorModel>(
            [] (Utils::DependencyManager *) {
                return new Presentation::ArtifactEditorModel;
        });
        Utils::DependencyManager::globalInstance().add<Presentation::AvailableSourcesModel>(
            [] (Utils::DependencyManager *) {
                return new Presentation::AvailableSourcesModel(Domain::DataSourceQueries::Ptr(),
                                                               Domain::DataSourceRepository::Ptr());
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
        QVERIFY(qobject_cast<FakeAvailablePagesModel*>(available));
    }

    void shouldProvideCurrentPage()
    {
        // GIVEN
        Presentation::ApplicationModel app;
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

    void shouldTakeOwnershipOfCurrentPage()
    {
        // GIVEN
        auto page = QPointer<QObject>(new QObject(this));

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

    void shouldProvideArtifactEditorModel()
    {
        // GIVEN
        Presentation::ApplicationModel app;

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
        Presentation::ApplicationModel app;
        app.setCurrentPage(new FakePageModel);

        // WHEN
        app.setErrorHandler(&errorHandler);

        // THEN
        auto availableSource = static_cast<Presentation::AvailableSourcesModel*>(app.availableSources());
        auto availablePages = static_cast<FakeAvailablePagesModel*>(app.availablePages());
        auto editor = static_cast<Presentation::ArtifactEditorModel*>(app.editor());
        auto page = static_cast<Presentation::PageModel*>(app.currentPage());
        QCOMPARE(availableSource->errorHandler(), &errorHandler);
        QCOMPARE(availablePages->errorHandler(), &errorHandler);
        QCOMPARE(editor->errorHandler(), &errorHandler);
        QCOMPARE(page->errorHandler(), &errorHandler);

        // WHEN
        FakeErrorHandler errorHandler2;

        app.setErrorHandler(&errorHandler2);

        // THEN
        QCOMPARE(availableSource->errorHandler(), &errorHandler2);
        QCOMPARE(availablePages->errorHandler(), &errorHandler2);
        QCOMPARE(editor->errorHandler(), &errorHandler2);
        QCOMPARE(page->errorHandler(), &errorHandler2);
    }
};

QTEST_MAIN(ApplicationModelTest)

#include "applicationmodeltest.moc"
