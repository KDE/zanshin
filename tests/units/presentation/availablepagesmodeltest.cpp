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

#include "domain/contextqueries.h"
#include "domain/contextrepository.h"
#include "domain/projectqueries.h"
#include "domain/projectrepository.h"
#include "domain/note.h"
#include "domain/tag.h"
#include "domain/tagqueries.h"
#include "domain/tagrepository.h"
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

class FakeErrorHandler : public Presentation::ErrorHandler
{
public:
    void doDisplayMessage(const QString &message)
    {
        m_message = message;
    }

    QString m_message;
};

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

        // No Tags
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);

        // Two artifacts (used for dropping later on)
        Domain::Artifact::Ptr taskToDrop(new Domain::Task);
        Domain::Artifact::Ptr noteToDrop(new Domain::Note);

        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);

        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                projectQueriesMock.getInstance(),
                                                projectRepositoryMock.getInstance(),
                                                contextQueriesMock.getInstance(),
                                                contextRepositoryMock.getInstance(),
                                                Domain::TaskQueries::Ptr(),
                                                taskRepositoryMock.getInstance(),
                                                Domain::NoteRepository::Ptr(),
                                                tagQueriesMock.getInstance(),
                                                Domain::TagRepository::Ptr());

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
        const QModelIndex tagsIndex = model->index(3, 0);

        QCOMPARE(model->rowCount(), 4);
        QCOMPARE(model->rowCount(inboxIndex), 0);
        QCOMPARE(model->rowCount(projectsIndex), 2);
        QCOMPARE(model->rowCount(project1Index), 0);
        QCOMPARE(model->rowCount(project2Index), 0);
        QCOMPARE(model->rowCount(contextsIndex), 2);
        QCOMPARE(model->rowCount(context1Index), 0);
        QCOMPARE(model->rowCount(context2Index), 0);
        QCOMPARE(model->rowCount(tagsIndex), 0);

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
        QCOMPARE(model->flags(tagsIndex), Qt::NoItemFlags);

        QCOMPARE(model->data(inboxIndex).toString(), tr("Inbox"));
        QCOMPARE(model->data(projectsIndex).toString(), tr("Projects"));
        QCOMPARE(model->data(project1Index).toString(), project1->name());
        QCOMPARE(model->data(project2Index).toString(), project2->name());
        QCOMPARE(model->data(contextsIndex).toString(), tr("Contexts"));
        QCOMPARE(model->data(context1Index).toString(), context1->name());
        QCOMPARE(model->data(context2Index).toString(), context2->name());
        QCOMPARE(model->data(tagsIndex).toString(), tr("Tags"));

        QVERIFY(!model->data(inboxIndex, Qt::EditRole).isValid());
        QVERIFY(!model->data(projectsIndex, Qt::EditRole).isValid());
        QCOMPARE(model->data(project1Index, Qt::EditRole).toString(), project1->name());
        QCOMPARE(model->data(project2Index, Qt::EditRole).toString(), project2->name());
        QVERIFY(!model->data(contextsIndex, Qt::EditRole).isValid());
        QCOMPARE(model->data(context1Index, Qt::EditRole).toString(), context1->name());
        QCOMPARE(model->data(context2Index, Qt::EditRole).toString(), context2->name());
        QVERIFY(!model->data(tagsIndex, Qt::EditRole).isValid());

        QCOMPARE(model->data(inboxIndex, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("mail-folder-inbox"));
        QCOMPARE(model->data(projectsIndex, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("folder"));
        QCOMPARE(model->data(project1Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("view-pim-tasks"));
        QCOMPARE(model->data(project2Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("view-pim-tasks"));
        QCOMPARE(model->data(contextsIndex, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("folder"));
        QCOMPARE(model->data(context1Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("view-pim-tasks"));
        QCOMPARE(model->data(context2Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("view-pim-tasks"));
        QCOMPARE(model->data(tagsIndex, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("folder"));

        QVERIFY(!model->data(inboxIndex, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(projectsIndex, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(project1Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(project2Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(contextsIndex, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(context1Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(context2Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(tagsIndex, Qt::CheckStateRole).isValid());

        // WHEN
        projectRepositoryMock(&Domain::ProjectRepository::update).when(project1).thenReturn(new FakeJob(this));
        projectRepositoryMock(&Domain::ProjectRepository::update).when(project2).thenReturn(new FakeJob(this));
        contextRepositoryMock(&Domain::ContextRepository::update).when(context1).thenReturn(new FakeJob(this));
        contextRepositoryMock(&Domain::ContextRepository::update).when(context2).thenReturn(new FakeJob(this));

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
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::update).when(context1).exactly(1));
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::update).when(context2).exactly(1));

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

        // WHEN a task is dropped on a context
        contextRepositoryMock(&Domain::ContextRepository::associate).when(context1, taskToDrop.objectCast<Domain::Task>()).thenReturn(new FakeJob(this));
        data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << taskToDrop));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, context1Index);

        // THEN
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::associate).when(context1, taskToDrop.objectCast<Domain::Task>()).exactly(1));

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

        // WHEN a task and a note are dropped on a context
        data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << taskToDrop2 << noteToDrop2));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, context1Index);

        // THEN
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::associate).when(context1, taskToDrop2.objectCast<Domain::Task>()).exactly(0));

        // WHEN two tasks are dropped on a context
        Domain::Task::Ptr taskToDrop3(new Domain::Task);
        Domain::Task::Ptr taskToDrop4(new Domain::Task);
        contextRepositoryMock(&Domain::ContextRepository::associate).when(context1, taskToDrop3).thenReturn(new FakeJob(this));
        contextRepositoryMock(&Domain::ContextRepository::associate).when(context1, taskToDrop4).thenReturn(new FakeJob(this));
        data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << taskToDrop3 << taskToDrop4));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, context1Index);

        // THEN
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::associate).when(context1, taskToDrop3).exactly(1));
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::associate).when(context1, taskToDrop4).exactly(1));
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
        // Empty tag provider
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);

        // context mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        // projects mocking
        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);

        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                projectQueriesMock.getInstance(),
                                                projectRepositoryMock.getInstance(),
                                                contextQueriesMock.getInstance(),
                                                contextRepositoryMock.getInstance(),
                                                Domain::TaskQueries::Ptr(),
                                                Domain::TaskRepository::Ptr(),
                                                Domain::NoteRepository::Ptr(),
                                                tagQueriesMock.getInstance(),
                                                Domain::TagRepository::Ptr());

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

        // Empty tag provider
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);

        // projects mocking
        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;

        // contexts mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        // tags mocking
        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);

        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                projectQueriesMock.getInstance(),
                                                projectRepositoryMock.getInstance(),
                                                contextQueriesMock.getInstance(),
                                                Domain::ContextRepository::Ptr(),
                                                Domain::TaskQueries::Ptr(),
                                                Domain::TaskRepository::Ptr(),
                                                Domain::NoteRepository::Ptr(),
                                                tagQueriesMock.getInstance(),
                                                Domain::TagRepository::Ptr());

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

        // Empty tag provider
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);

        // contexts mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        // projects mocking
        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;

        // tags mocking
        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);


        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                projectQueriesMock.getInstance(),
                                                projectRepositoryMock.getInstance(),
                                                contextQueriesMock.getInstance(),
                                                contextRepositoryMock.getInstance(),
                                                Domain::TaskQueries::Ptr(),
                                                Domain::TaskRepository::Ptr(),
                                                Domain::NoteRepository::Ptr(),
                                                tagQueriesMock.getInstance(),
                                                Domain::TagRepository::Ptr());

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

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;
        projectRepositoryMock(&Domain::ProjectRepository::create).when(any<Domain::Project::Ptr>(),
                                                                       any<Domain::DataSource::Ptr>())
                                                                 .thenReturn(new FakeJob(this));

        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                Domain::ProjectQueries::Ptr(),
                                                projectRepositoryMock.getInstance(),
                                                Domain::ContextQueries::Ptr(),
                                                Domain::ContextRepository::Ptr(),
                                                Domain::TaskQueries::Ptr(),
                                                Domain::TaskRepository::Ptr(),
                                                Domain::NoteRepository::Ptr(),
                                                Domain::TagQueries::Ptr(),
                                                Domain::TagRepository::Ptr());

        // WHEN
        pages.addProject("Foo", source);

        // THEN
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::create).when(any<Domain::Project::Ptr>(),
                                                                               any<Domain::DataSource::Ptr>())
                                                                         .exactly(1));
    }

    void shouldGetAnErrorMessageWhenAddProjectFailed()
    {
        // GIVEN

        auto source = Domain::DataSource::Ptr::create();
        source->setName("Source1");

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        projectRepositoryMock(&Domain::ProjectRepository::create).when(any<Domain::Project::Ptr>(),
                                                                       any<Domain::DataSource::Ptr>())
                                                                 .thenReturn(job);

        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                Domain::ProjectQueries::Ptr(),
                                                projectRepositoryMock.getInstance(),
                                                Domain::ContextQueries::Ptr(),
                                                Domain::ContextRepository::Ptr(),
                                                Domain::TaskQueries::Ptr(),
                                                Domain::TaskRepository::Ptr(),
                                                Domain::NoteRepository::Ptr(),
                                                Domain::TagQueries::Ptr(),
                                                Domain::TagRepository::Ptr());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        // WHEN
        pages.addProject("Foo", source);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot add project Foo in dataSource Source1: Foo"));
    }

    void shouldAddContexts()
    {
        // GIVEN

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;
        contextRepositoryMock(&Domain::ContextRepository::create).when(any<Domain::Context::Ptr>())
                                                                 .thenReturn(new FakeJob(this));

        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                Domain::ProjectQueries::Ptr(),
                                                Domain::ProjectRepository::Ptr(),
                                                Domain::ContextQueries::Ptr(),
                                                contextRepositoryMock.getInstance(),
                                                Domain::TaskQueries::Ptr(),
                                                Domain::TaskRepository::Ptr(),
                                                Domain::NoteRepository::Ptr(),
                                                Domain::TagQueries::Ptr(),
                                                Domain::TagRepository::Ptr());

        // WHEN
        pages.addContext("Foo");

        // THEN
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::create).when(any<Domain::Context::Ptr>())
                                                                       .exactly(1));
    }

    void shouldGetAnErrorMessageWhenAddContextFailed()
    {
        // GIVEN

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        contextRepositoryMock(&Domain::ContextRepository::create).when(any<Domain::Context::Ptr>())
                                                                 .thenReturn(job);

        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                Domain::ProjectQueries::Ptr(),
                                                Domain::ProjectRepository::Ptr(),
                                                Domain::ContextQueries::Ptr(),
                                                contextRepositoryMock.getInstance(),
                                                Domain::TaskQueries::Ptr(),
                                                Domain::TaskRepository::Ptr(),
                                                Domain::NoteRepository::Ptr(),
                                                Domain::TagQueries::Ptr(),
                                                Domain::TagRepository::Ptr());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        // WHEN
        pages.addContext("Foo");

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot add context Foo: Foo"));
    }

    void shouldAddTags()
    {
        // GIVEN

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;
        tagRepositoryMock(&Domain::TagRepository::create).when(any<Domain::Tag::Ptr>())
                                                                 .thenReturn(new FakeJob(this));

        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                Domain::ProjectQueries::Ptr(),
                                                Domain::ProjectRepository::Ptr(),
                                                Domain::ContextQueries::Ptr(),
                                                Domain::ContextRepository::Ptr(),
                                                Domain::TaskQueries::Ptr(),
                                                Domain::TaskRepository::Ptr(),
                                                Domain::NoteRepository::Ptr(),
                                                Domain::TagQueries::Ptr(),
                                                tagRepositoryMock.getInstance());

        // WHEN
        pages.addTag("Foo");

        // THEN
        QVERIFY(tagRepositoryMock(&Domain::TagRepository::create).when(any<Domain::Tag::Ptr>())
                                                                       .exactly(1));
    }

    void shouldGetAnErrorMessageWhenAddTagFailed()
    {
        // GIVEN

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        tagRepositoryMock(&Domain::TagRepository::create).when(any<Domain::Tag::Ptr>())
                                                                 .thenReturn(job);

        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                Domain::ProjectQueries::Ptr(),
                                                Domain::ProjectRepository::Ptr(),
                                                Domain::ContextQueries::Ptr(),
                                                Domain::ContextRepository::Ptr(),
                                                Domain::TaskQueries::Ptr(),
                                                Domain::TaskRepository::Ptr(),
                                                Domain::NoteRepository::Ptr(),
                                                Domain::TagQueries::Ptr(),
                                                tagRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        // WHEN
        pages.addTag("Foo");

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot add tag Foo: Foo"));
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

        // No contexts
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);

        // Empty tag provider
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);

        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);

        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                projectQueriesMock.getInstance(),
                                                projectRepositoryMock.getInstance(),
                                                contextQueriesMock.getInstance(),
                                                Domain::ContextRepository::Ptr(),
                                                Domain::TaskQueries::Ptr(),
                                                Domain::TaskRepository::Ptr(),
                                                Domain::NoteRepository::Ptr(),
                                                tagQueriesMock.getInstance(),
                                                Domain::TagRepository::Ptr());

        QAbstractItemModel *model = pages.pageListModel();

        const QModelIndex projectsIndex = model->index(1, 0);
        const QModelIndex project1Index = model->index(0, 0, projectsIndex);

        projectRepositoryMock(&Domain::ProjectRepository::remove).when(project1).thenReturn(new FakeJob(this));

        // WHEN
        pages.removeItem(project1Index);

        // THEN
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::remove).when(project1).exactly(1));
    }

    void shouldGetAnErrorMessageWhenRemoveProjectFailed()
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

        // Empty tag provider
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);

        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);

        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                projectQueriesMock.getInstance(),
                                                projectRepositoryMock.getInstance(),
                                                contextQueriesMock.getInstance(),
                                                Domain::ContextRepository::Ptr(),
                                                Domain::TaskQueries::Ptr(),
                                                Domain::TaskRepository::Ptr(),
                                                Domain::NoteRepository::Ptr(),
                                                tagQueriesMock.getInstance(),
                                                Domain::TagRepository::Ptr());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        QAbstractItemModel *model = pages.pageListModel();

        const QModelIndex projectsIndex = model->index(1, 0);
        const QModelIndex project1Index = model->index(0, 0, projectsIndex);

        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        projectRepositoryMock(&Domain::ProjectRepository::remove).when(project1).thenReturn(job);

        // WHEN
        pages.removeItem(project1Index);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot remove project Project 1: Foo"));
    }

    void shouldRemoveContext()
    {
        // GIVEN

        // Two contexts
        auto context1 = Domain::Context::Ptr::create();
        context1->setName("context 1");
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);
        contextProvider->append(context1);
        // empty projects
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);

        // contexts mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        // Empty tag provider
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        // projects mocking
        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);


        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                projectQueriesMock.getInstance(),
                                                Domain::ProjectRepository::Ptr(),
                                                contextQueriesMock.getInstance(),
                                                contextRepositoryMock.getInstance(),
                                                Domain::TaskQueries::Ptr(),
                                                Domain::TaskRepository::Ptr(),
                                                Domain::NoteRepository::Ptr(),
                                                tagQueriesMock.getInstance(),
                                                Domain::TagRepository::Ptr());

        QAbstractItemModel *model = pages.pageListModel();

        const QModelIndex contextsIndex = model->index(2, 0);
        const QModelIndex context1Index = model->index(0, 0, contextsIndex);

        contextRepositoryMock(&Domain::ContextRepository::remove).when(context1).thenReturn(new FakeJob(this));

        // WHEN
        pages.removeItem(context1Index);

        // THEN
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::remove).when(context1).exactly(1));
    }

    void shouldGetAnErrorMessageWhenRemoveContextFailed()
    {
        // GIVEN

        // Two contexts
        auto context1 = Domain::Context::Ptr::create();
        context1->setName("context 1");
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);
        contextProvider->append(context1);
        // empty projects
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);

        // contexts mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        // Empty tag provider
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        // projects mocking
        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);


        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                projectQueriesMock.getInstance(),
                                                Domain::ProjectRepository::Ptr(),
                                                contextQueriesMock.getInstance(),
                                                contextRepositoryMock.getInstance(),
                                                Domain::TaskQueries::Ptr(),
                                                Domain::TaskRepository::Ptr(),
                                                Domain::NoteRepository::Ptr(),
                                                tagQueriesMock.getInstance(),
                                                Domain::TagRepository::Ptr());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        QAbstractItemModel *model = pages.pageListModel();

        const QModelIndex contextsIndex = model->index(2, 0);
        const QModelIndex context1Index = model->index(0, 0, contextsIndex);

        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        contextRepositoryMock(&Domain::ContextRepository::remove).when(context1).thenReturn(job);

        // WHEN
        pages.removeItem(context1Index);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot remove context context 1: Foo"));
    }

    void shouldRemoveTag()
    {
        // GIVEN

        // context provider
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);
        // empty projects
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);

        // contexts mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        // one tag
        auto tag1 = Domain::Tag::Ptr::create();
        tag1->setName("tag 1");
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);
        tagProvider->append(tag1);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        // projects mocking
        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;

        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                projectQueriesMock.getInstance(),
                                                Domain::ProjectRepository::Ptr(),
                                                contextQueriesMock.getInstance(),
                                                contextRepositoryMock.getInstance(),
                                                Domain::TaskQueries::Ptr(),
                                                Domain::TaskRepository::Ptr(),
                                                Domain::NoteRepository::Ptr(),
                                                tagQueriesMock.getInstance(),
                                                tagRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        QAbstractItemModel *model = pages.pageListModel();

        const QModelIndex tagsIndex = model->index(3, 0);
        const QModelIndex tag1Index = model->index(0, 0, tagsIndex);

        auto job = new FakeJob(this);
        tagRepositoryMock(&Domain::TagRepository::remove).when(tag1).thenReturn(job);

        // WHEN
        pages.removeItem(tag1Index);

        // THEN
        QTest::qWait(150);
        QVERIFY(errorHandler.m_message.isEmpty());
        QVERIFY(tagRepositoryMock(&Domain::TagRepository::remove).when(tag1).exactly(1));
    }

    void shouldGetAnErrorMessageWhenRemoveTagFailed()
    {
        // GIVEN

        // context provider
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);
        // empty projects
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);

        // contexts mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        // one tag
        auto tag1 = Domain::Tag::Ptr::create();
        tag1->setName("tag 1");
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);
        tagProvider->append(tag1);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        // projects mocking
        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;

        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                projectQueriesMock.getInstance(),
                                                Domain::ProjectRepository::Ptr(),
                                                contextQueriesMock.getInstance(),
                                                contextRepositoryMock.getInstance(),
                                                Domain::TaskQueries::Ptr(),
                                                Domain::TaskRepository::Ptr(),
                                                Domain::NoteRepository::Ptr(),
                                                tagQueriesMock.getInstance(),
                                                tagRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        QAbstractItemModel *model = pages.pageListModel();

        const QModelIndex tagsIndex = model->index(3, 0);
        const QModelIndex tag1Index = model->index(0, 0, tagsIndex);

        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        tagRepositoryMock(&Domain::TagRepository::remove).when(tag1).thenReturn(job);

        // WHEN
        pages.removeItem(tag1Index);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot remove tag tag 1: Foo"));
    }

    void shouldGetAnErrorMessageWhenUpdateProjectFailed()
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

        // No Tags
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);

        // Two artifacts (used for dropping later on)
        Domain::Artifact::Ptr taskToDrop(new Domain::Task);
        Domain::Artifact::Ptr noteToDrop(new Domain::Note);

        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findAll).when().thenReturn(projectResult);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);

        Presentation::AvailablePagesModel pages(Domain::ArtifactQueries::Ptr(),
                                                projectQueriesMock.getInstance(),
                                                projectRepositoryMock.getInstance(),
                                                contextQueriesMock.getInstance(),
                                                contextRepositoryMock.getInstance(),
                                                Domain::TaskQueries::Ptr(),
                                                taskRepositoryMock.getInstance(),
                                                Domain::NoteRepository::Ptr(),
                                                tagQueriesMock.getInstance(),
                                                Domain::TagRepository::Ptr());

        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);
        QAbstractItemModel *model = pages.pageListModel();
        const QModelIndex projectsIndex = model->index(1, 0);
        const QModelIndex project1Index = model->index(0, 0, projectsIndex);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        projectRepositoryMock(&Domain::ProjectRepository::update).when(project1).thenReturn(job);

        QVERIFY(model->setData(project1Index, "New Project 1"));

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot modify project Project 1: Foo"));
    }
};

QTEST_MAIN(AvailablePagesModelTest)

#include "availablepagesmodeltest.moc"
