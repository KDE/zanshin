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

#include "domain/contextqueries.h"
#include "domain/contextrepository.h"
#include "domain/projectqueries.h"
#include "domain/projectrepository.h"
#include "domain/note.h"
#include "domain/task.h"
#include "domain/taskrepository.h"

#include "presentation/availablepagesmodel.h"
#include "presentation/contextpagemodel.h"
#include "presentation/inboxpagemodel.h"
#include "presentation/projectpagemodel.h"
#include "presentation/querytreemodelbase.h"

#include "testlib/fakejob.h"

using namespace mockitopp;
using namespace mockitopp::matcher;

class AvailablePagesModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldListAvailablePages()
    {
        // GIVEN

        // Two projects
        auto project1 = Domain::Project::Ptr::create();
        project1->setName("Project 1");
        auto project2 = Domain::Project::Ptr::create();
        project2->setName("Project 2");
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);
        projectProvider->append(project1);
        projectProvider->append(project2);

        // Two contexts
        auto context1 = Domain::Context::Ptr::create();
        context1->setName("context 1");
        auto context2 = Domain::Context::Ptr::create();
        context2->setName("context 2");
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);
        contextProvider->append(context1);
        contextProvider->append(context2);

        // Two artifacts (used for dropping later on)
        Domain::Artifact::Ptr taskToDrop(new Domain::Task);
        Domain::Artifact::Ptr noteToDrop(new Domain::Note);

        mock_object<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        mock_object<Domain::ProjectRepository> projectRepositoryMock;
        mock_object<Domain::TaskRepository> taskRepositoryMock;

        mock_object<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        mock_object<Domain::ContextRepository> contextRepositoryMock;

        Presentation::AvailablePagesModel pages(0,
                                                &projectQueriesMock.getInstance(),
                                                &projectRepositoryMock.getInstance(),
                                                &contextQueriesMock.getInstance(),
                                                &contextRepositoryMock.getInstance(),
                                                0,
                                                &taskRepositoryMock.getInstance(),
                                                0);

        // WHEN
        QAbstractItemModel *model = pages.pageListModel();

        // THEN
        const QModelIndex inboxIndex = model->index(0, 0);
        const QModelIndex projectsIndex = model->index(1, 0);
        const QModelIndex project1Index = model->index(0, 0, projectsIndex);
        const QModelIndex project2Index = model->index(1, 0, projectsIndex);
        const QModelIndex contextsIndex = model->index(2, 0);
        const QModelIndex context1Index = model->index(0, 0, contextsIndex);
        const QModelIndex context2Index = model->index(1, 0, contextsIndex);

        QCOMPARE(model->rowCount(), 3);
        QCOMPARE(model->rowCount(inboxIndex), 0);
        QCOMPARE(model->rowCount(projectsIndex), 2);
        QCOMPARE(model->rowCount(project1Index), 0);
        QCOMPARE(model->rowCount(project2Index), 0);
        QCOMPARE(model->rowCount(contextsIndex), 2);
        QCOMPARE(model->rowCount(context1Index), 0);
        QCOMPARE(model->rowCount(context2Index), 0);

        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled
                                         | Qt::ItemIsEditable;
        QCOMPARE(model->flags(inboxIndex), (defaultFlags & ~(Qt::ItemIsEditable)) | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(projectsIndex), Qt::NoItemFlags);
        QCOMPARE(model->flags(project1Index), defaultFlags | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(project2Index), defaultFlags | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(contextsIndex), Qt::NoItemFlags);
        QCOMPARE(model->flags(context1Index), defaultFlags | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(context2Index), defaultFlags | Qt::ItemIsDropEnabled);

        QCOMPARE(model->data(inboxIndex).toString(), tr("Inbox"));
        QCOMPARE(model->data(projectsIndex).toString(), tr("Projects"));
        QCOMPARE(model->data(project1Index).toString(), project1->name());
        QCOMPARE(model->data(project2Index).toString(), project2->name());
        QCOMPARE(model->data(contextsIndex).toString(), tr("Contexts"));
        QCOMPARE(model->data(context1Index).toString(), context1->name());
        QCOMPARE(model->data(context2Index).toString(), context2->name());

        QVERIFY(!model->data(inboxIndex, Qt::EditRole).isValid());
        QVERIFY(!model->data(projectsIndex, Qt::EditRole).isValid());
        QCOMPARE(model->data(project1Index, Qt::EditRole).toString(), project1->name());
        QCOMPARE(model->data(project2Index, Qt::EditRole).toString(), project2->name());
        QVERIFY(!model->data(contextsIndex, Qt::EditRole).isValid());
        QCOMPARE(model->data(context1Index, Qt::EditRole).toString(), context1->name());
        QCOMPARE(model->data(context2Index, Qt::EditRole).toString(), context2->name());

        QCOMPARE(model->data(inboxIndex, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("mail-folder-inbox"));
        QCOMPARE(model->data(projectsIndex, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("folder"));
        QCOMPARE(model->data(project1Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("view-pim-tasks"));
        QCOMPARE(model->data(project2Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("view-pim-tasks"));
        QCOMPARE(model->data(contextsIndex, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("folder"));
        QCOMPARE(model->data(context1Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("view-pim-tasks"));
        QCOMPARE(model->data(context2Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("view-pim-tasks"));

        QVERIFY(!model->data(inboxIndex, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(projectsIndex, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(project1Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(project2Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(contextsIndex, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(context1Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(context2Index, Qt::CheckStateRole).isValid());

        // WHEN
        projectRepositoryMock(&Domain::ProjectRepository::update).when(project1).thenReturn(new FakeJob(this));
        projectRepositoryMock(&Domain::ProjectRepository::update).when(project2).thenReturn(new FakeJob(this));

        QVERIFY(!model->setData(inboxIndex, "Foo"));
        QVERIFY(!model->setData(projectsIndex, "Foo"));
        QVERIFY(model->setData(project1Index, "New Project 1"));
        QVERIFY(model->setData(project2Index, "New Project 2"));
        QVERIFY(!model->setData(contextsIndex, "Foo"));
        QVERIFY(model->setData(context1Index, "New Context 1"));
        QVERIFY(model->setData(context2Index, "New Context 2"));

        // THEN
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::update).when(project1).exactly(1));
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::update).when(project2).exactly(1));

        QCOMPARE(project1->name(), QString("New Project 1"));
        QCOMPARE(project2->name(), QString("New Project 2"));
        QCOMPARE(context1->name(), QString("New Context 1"));
        QCOMPARE(context2->name(), QString("New Context 2"));

        // WHEN
        projectRepositoryMock(&Domain::ProjectRepository::associate).when(project1, taskToDrop).thenReturn(new FakeJob(this));
        QMimeData *data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << taskToDrop));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, project1Index);

        // THEN
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::associate).when(project1, taskToDrop).exactly(1));

        // WHEN
        projectRepositoryMock(&Domain::ProjectRepository::dissociate).when(taskToDrop).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::dissociate).when(taskToDrop.objectCast<Domain::Task>()).thenReturn(new FakeJob(this));
        data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << taskToDrop));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, inboxIndex);
        QTest::qWait(150);

        // THEN
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::dissociate).when(taskToDrop).exactly(1));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::dissociate).when(taskToDrop.objectCast<Domain::Task>()).exactly(1));

        // WHEN
        projectRepositoryMock(&Domain::ProjectRepository::associate).when(project2, noteToDrop).thenReturn(new FakeJob(this));
        data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << noteToDrop));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, project2Index);

        // THEN
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::associate).when(project2, noteToDrop).exactly(1));

        // WHEN
        Domain::Artifact::Ptr taskToDrop2(new Domain::Task);
        Domain::Artifact::Ptr noteToDrop2(new Domain::Note);
        projectRepositoryMock(&Domain::ProjectRepository::associate).when(project1, taskToDrop2).thenReturn(new FakeJob(this));
        projectRepositoryMock(&Domain::ProjectRepository::associate).when(project1, noteToDrop2).thenReturn(new FakeJob(this));
        data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << taskToDrop2 << noteToDrop2));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, project1Index);

        // THEN
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::associate).when(project1, taskToDrop2).exactly(1));
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::associate).when(project1, noteToDrop2).exactly(1));
    }



    void shouldCreateInboxPage()
    {
        // GIVEN

        // Empty project provider
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);
        // Empty context provider
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);

        // context mocking
        mock_object<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        mock_object<Domain::ContextRepository> contextRepositoryMock;

        // projects mocking
        mock_object<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        mock_object<Domain::ProjectRepository> projectRepositoryMock;

        Presentation::AvailablePagesModel pages(0,
                                                &projectQueriesMock.getInstance(),
                                                &projectRepositoryMock.getInstance(),
                                                &contextQueriesMock.getInstance(),
                                                &contextRepositoryMock.getInstance(),
                                                0,
                                                0,
                                                0);

        // WHEN
        QAbstractItemModel *model = pages.pageListModel();

        // THEN
        const QModelIndex inboxIndex = model->index(0, 0);

        QObject *inboxPage = pages.createPageForIndex(inboxIndex);
        QVERIFY(qobject_cast<Presentation::InboxPageModel*>(inboxPage));
    }

    void shouldCreateProjectsPage()
    {
        // GIVEN

        // Two projects
        auto project1 = Domain::Project::Ptr::create();
        project1->setName("Project 1");
        auto project2 = Domain::Project::Ptr::create();
        project2->setName("Project 2");
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);
        projectProvider->append(project1);
        projectProvider->append(project2);

        // No contexts
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);

        // projects mocking
        mock_object<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        mock_object<Domain::ProjectRepository> projectRepositoryMock;

        // contexts mocking
        mock_object<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Presentation::AvailablePagesModel pages(0,
                                                &projectQueriesMock.getInstance(),
                                                &projectRepositoryMock.getInstance(),
                                                &contextQueriesMock.getInstance(),
                                                0,
                                                0,
                                                0,
                                                0);

        // WHEN
        QAbstractItemModel *model = pages.pageListModel();

        // THEN
        const QModelIndex projectsIndex = model->index(1, 0);
        const QModelIndex project1Index = model->index(0, 0, projectsIndex);
        const QModelIndex project2Index = model->index(1, 0, projectsIndex);

        QObject *projectsPage = pages.createPageForIndex(projectsIndex);
        QObject *project1Page = pages.createPageForIndex(project1Index);
        QObject *project2Page = pages.createPageForIndex(project2Index);

        QVERIFY(!projectsPage);
        QVERIFY(qobject_cast<Presentation::ProjectPageModel*>(project1Page));
        QCOMPARE(qobject_cast<Presentation::ProjectPageModel*>(project1Page)->project(), project1);
        QVERIFY(qobject_cast<Presentation::ProjectPageModel*>(project2Page));
        QCOMPARE(qobject_cast<Presentation::ProjectPageModel*>(project2Page)->project(), project2);
    }

    void shouldCreateContextsPage()
    {
        // GIVEN

        // Two contexts
        auto context1 = Domain::Context::Ptr::create();
        context1->setName("context 1");
        auto context2 = Domain::Context::Ptr::create();
        context2->setName("context 2");
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);
        contextProvider->append(context1);
        contextProvider->append(context2);

        // Empty Project provider
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);

        // contexts mocking
        mock_object<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        mock_object<Domain::ContextRepository> contextRepositoryMock;

        // projects mocking
        mock_object<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        mock_object<Domain::ProjectRepository> projectRepositoryMock;

        Presentation::AvailablePagesModel pages(0,
                                                &projectQueriesMock.getInstance(),
                                                &projectRepositoryMock.getInstance(),
                                                &contextQueriesMock.getInstance(),
                                                &contextRepositoryMock.getInstance(),
                                                0,
                                                0,
                                                0);

        // WHEN
        QAbstractItemModel *model = pages.pageListModel();

        // THEN
        const QModelIndex contextsIndex = model->index(2, 0);
        const QModelIndex context1Index = model->index(0, 0, contextsIndex);
        const QModelIndex context2Index = model->index(1, 0, contextsIndex);

        QObject *contextsPage = pages.createPageForIndex(contextsIndex);
        QObject *context1Page = pages.createPageForIndex(context1Index);
        QObject *context2Page = pages.createPageForIndex(context2Index);

        QVERIFY(!contextsPage);
        QVERIFY(qobject_cast<Presentation::ContextPageModel*>(context1Page));
        QCOMPARE(qobject_cast<Presentation::ContextPageModel*>(context1Page)->context(), context1);
        QVERIFY(qobject_cast<Presentation::ContextPageModel*>(context2Page));
        QCOMPARE(qobject_cast<Presentation::ContextPageModel*>(context2Page)->context(), context2);
    }

    void shouldAddProjects()
    {
        // GIVEN

        auto source = Domain::DataSource::Ptr::create();

        mock_object<Domain::ProjectRepository> projectRepositoryMock;
        projectRepositoryMock(&Domain::ProjectRepository::create).when(any<Domain::Project::Ptr>(),
                                                                       any<Domain::DataSource::Ptr>())
                                                                 .thenReturn(new FakeJob(this));

        Presentation::AvailablePagesModel pages(0, 0,
                                                &projectRepositoryMock.getInstance(),
                                                0, 0, 0, 0, 0);

        // WHEN
        pages.addProject("Foo", source);

        // THEN
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::create).when(any<Domain::Project::Ptr>(),
                                                                               any<Domain::DataSource::Ptr>())
                                                                         .exactly(1));
    }

    void shouldAddContexts()
    {
        // GIVEN

        mock_object<Domain::ContextRepository> contextRepositoryMock;
        contextRepositoryMock(&Domain::ContextRepository::create).when(any<Domain::Context::Ptr>())
                                                                 .thenReturn(new FakeJob(this));

        Presentation::AvailablePagesModel pages(0, 0, 0, 0,
                                                &contextRepositoryMock.getInstance(),
                                                0, 0, 0);

        // WHEN
        pages.addContext("Foo");

        // THEN
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::create).when(any<Domain::Context::Ptr>())
                                                                       .exactly(1));
    }

    void shouldRemoveProject()
    {
        // GIVEN

        // Two projects
        auto project1 = Domain::Project::Ptr::create();
        project1->setName("Project 1");
        auto project2 = Domain::Project::Ptr::create();
        project2->setName("Project 2");
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);
        projectProvider->append(project1);
        projectProvider->append(project2);

        mock_object<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        mock_object<Domain::ProjectRepository> projectRepositoryMock;

        Presentation::AvailablePagesModel pages(0,
                                                &projectQueriesMock.getInstance(),
                                                &projectRepositoryMock.getInstance(),
                                                0,
                                                0,
                                                0);

        QAbstractItemModel *model = pages.pageListModel();

        const QModelIndex projectsIndex = model->index(1, 0);
        const QModelIndex project1Index = model->index(0, 0, projectsIndex);

        projectRepositoryMock(&Domain::ProjectRepository::remove).when(project1).thenReturn(new FakeJob(this));

        // WHEN
        pages.removeItem(project1Index);

        // THEN
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::remove).when(project1).exactly(1));
    }
};

QTEST_MAIN(AvailablePagesModelTest)

#include "availablepagesmodeltest.moc"
