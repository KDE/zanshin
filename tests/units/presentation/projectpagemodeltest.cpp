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

#include "presentation/projectpagemodel.h"
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

class ProjectPageModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldListProjectInCentralListModel()
    {
        // GIVEN

        // One project
        auto project = Domain::Project::Ptr::create();

        // One note and one task
        auto rootTask = Domain::Task::Ptr::create();
        rootTask->setTitle("rootTask");
        auto topLevelProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto topLevelResult = Domain::QueryResult<Domain::Task::Ptr>::create(topLevelProvider);
        topLevelProvider->append(rootTask);

        // One task under the root task
        auto childTask = Domain::Task::Ptr::create();
        childTask->setTitle("childTask");
        childTask->setDone(true);
        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(childTask);

        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findTopLevel).when(project).thenReturn(topLevelResult);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(rootTask).thenReturn(taskResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(childTask).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::ProjectPageModel page(project,
                                            projectQueriesMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        // WHEN
        QAbstractItemModel *model = page.centralListModel();

        // THEN
        const QModelIndex rootTaskIndex = model->index(0, 0);
        const QModelIndex childTaskIndex = model->index(0, 0, rootTaskIndex);

        QCOMPARE(page.project(), project);

        QCOMPARE(model->rowCount(), 1);
        QCOMPARE(model->rowCount(rootTaskIndex), 1);
        QCOMPARE(model->rowCount(childTaskIndex), 0);

        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled
                                         | Qt::ItemIsEditable
                                         | Qt::ItemIsDragEnabled
                                         | Qt::ItemIsUserCheckable
                                         | Qt::ItemIsDropEnabled;
        QCOMPARE(model->flags(rootTaskIndex), defaultFlags);
        QCOMPARE(model->flags(childTaskIndex), defaultFlags);

        QCOMPARE(model->data(rootTaskIndex).toString(), rootTask->title());
        QCOMPARE(model->data(childTaskIndex).toString(), childTask->title());

        QCOMPARE(model->data(rootTaskIndex, Qt::EditRole).toString(), rootTask->title());
        QCOMPARE(model->data(childTaskIndex, Qt::EditRole).toString(), childTask->title());

        QVERIFY(model->data(rootTaskIndex, Qt::CheckStateRole).isValid());
        QVERIFY(model->data(childTaskIndex, Qt::CheckStateRole).isValid());

        QCOMPARE(model->data(rootTaskIndex, Qt::CheckStateRole).toBool(), rootTask->isDone());
        QCOMPARE(model->data(childTaskIndex, Qt::CheckStateRole).toBool(), childTask->isDone());

        // WHEN
        taskRepositoryMock(&Domain::TaskRepository::update).when(rootTask).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::update).when(childTask).thenReturn(new FakeJob(this));

        QVERIFY(model->setData(rootTaskIndex, "newRootTask"));
        QVERIFY(model->setData(childTaskIndex, "newChildTask"));

        QVERIFY(model->setData(rootTaskIndex, Qt::Checked, Qt::CheckStateRole));
        QVERIFY(model->setData(childTaskIndex, Qt::Unchecked, Qt::CheckStateRole));

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(rootTask).exactly(2));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(childTask).exactly(2));

        QCOMPARE(rootTask->title(), QString("newRootTask"));
        QCOMPARE(childTask->title(), QString("newChildTask"));

        QCOMPARE(rootTask->isDone(), true);
        QCOMPARE(childTask->isDone(), false);

        // WHEN
        QMimeData *data = model->mimeData(QModelIndexList() << childTaskIndex);

        // THEN
        QVERIFY(data->hasFormat("application/x-zanshin-object"));
        QCOMPARE(data->property("objects").value<Domain::Artifact::List>(),
                 Domain::Artifact::List() << childTask);

        // WHEN
        auto childTask2 = Domain::Task::Ptr::create();
        taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask2).thenReturn(new FakeJob(this));
        data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << childTask2));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, rootTaskIndex);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask2).exactly(1));


        // WHEN
        auto childTask3 = Domain::Task::Ptr::create();
        auto childTask4 = Domain::Task::Ptr::create();
        taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask3).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask4).thenReturn(new FakeJob(this));
        data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << childTask3 << childTask4));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, rootTaskIndex);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask3).exactly(1));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask4).exactly(1));
    }

    void shouldAddTasks()
    {
        // GIVEN

        // One project
        auto project = Domain::Project::Ptr::create();

        // ... in fact we won't list any model
        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;

        // We'll gladly create a task though
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::createInProject).when(any<Domain::Task::Ptr>(),
                                                                          any<Domain::Project::Ptr>())
                                                                    .thenReturn(new FakeJob(this));

        Presentation::ProjectPageModel page(project,
                                            projectQueriesMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        // WHEN
        auto title = QString("New task");
        auto task = page.addItem(title).objectCast<Domain::Task>();

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::createInProject).when(any<Domain::Task::Ptr>(),
                                                                                  any<Domain::Project::Ptr>())
                                                                            .exactly(1));
        QVERIFY(task);
        QCOMPARE(task->title(), title);
    }

    void shouldGetAnErrorMessageWhenAddTaskFailed()
    {
        // GIVEN

        // One project
        auto project = Domain::Project::Ptr::create();
        project->setName("Project1");

        // ... in fact we won't list any model
        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;

        // We'll gladly create a task though
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        taskRepositoryMock(&Domain::TaskRepository::createInProject).when(any<Domain::Task::Ptr>(),
                                                                          any<Domain::Project::Ptr>())
                                                                    .thenReturn(job);

        Presentation::ProjectPageModel page(project,
                                            projectQueriesMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN
        page.addItem("New task");

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot add task New task in project Project1: Foo"));
    }

    void shouldDeleteItems()
    {
        // GIVEN

        // One project
        auto project = Domain::Project::Ptr::create();

        // Two tasks
        auto task1 = Domain::Task::Ptr::create();
        auto task2 = Domain::Task::Ptr::create();
        auto topLevelProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto topLevelResult = Domain::QueryResult<Domain::Task::Ptr>::create(topLevelProvider);
        topLevelProvider->append(task1);
        topLevelProvider->append(task2);

        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findTopLevel).when(project).thenReturn(topLevelResult);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::remove).when(task2).thenReturn(new FakeJob(this));

        Presentation::ProjectPageModel page(project,
                                            projectQueriesMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        // WHEN
        const QModelIndex index = page.centralListModel()->index(1, 0);
        page.removeItem(index);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::remove).when(task2).exactly(1));
    }

    void shouldGetAnErrorMessageWhenDeleteItemsFailed()
    {
        // GIVEN

        // One project
        auto project = Domain::Project::Ptr::create();
        project->setName("Project1");

        // Two tasks
        auto task1 = Domain::Task::Ptr::create();
        auto task2 = Domain::Task::Ptr::create();
        task2->setTitle("Task2");
        auto topLevelProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto topLevelResult = Domain::QueryResult<Domain::Task::Ptr>::create(topLevelProvider);
        topLevelProvider->append(task1);
        topLevelProvider->append(task2);

        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findTopLevel).when(project).thenReturn(topLevelResult);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        taskRepositoryMock(&Domain::TaskRepository::remove).when(task2).thenReturn(job);

        Presentation::ProjectPageModel page(project,
                                            projectQueriesMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN
        const QModelIndex index = page.centralListModel()->index(1, 0);
        page.removeItem(index);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot remove task Task2 from project Project1: Foo"));
    }

    void shouldPromoteItem()
    {
        // GIVEN

        // One project
        auto project = Domain::Project::Ptr::create();

        // Two tasks
        auto task1 = Domain::Task::Ptr::create();
        auto task2 = Domain::Task::Ptr::create();
        auto topLevelProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto topLevelResult = Domain::QueryResult<Domain::Task::Ptr>::create(topLevelProvider);
        topLevelProvider->append(task1);
        topLevelProvider->append(task2);

        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findTopLevel).when(project).thenReturn(topLevelResult);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::promoteToProject).when(task2).thenReturn(new FakeJob(this));

        Presentation::ProjectPageModel page(project,
                                            projectQueriesMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        // WHEN
        const QModelIndex index = page.centralListModel()->index(1, 0);
        page.promoteItem(index);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::promoteToProject).when(task2).exactly(1));
    }

    void shouldGetAnErrorMessageWhenPromoteItemFailed()
    {
        // GIVEN

        // One project
        auto project = Domain::Project::Ptr::create();
        project->setName("Project1");

        // Two tasks
        auto task1 = Domain::Task::Ptr::create();
        auto task2 = Domain::Task::Ptr::create();
        task2->setTitle("Task2");
        auto topLevelProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto topLevelResult = Domain::QueryResult<Domain::Task::Ptr>::create(topLevelProvider);
        topLevelProvider->append(task1);
        topLevelProvider->append(task2);

        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findTopLevel).when(project).thenReturn(topLevelResult);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        taskRepositoryMock(&Domain::TaskRepository::promoteToProject).when(task2).thenReturn(job);

        Presentation::ProjectPageModel page(project,
                                            projectQueriesMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN
        const QModelIndex index = page.centralListModel()->index(1, 0);
        page.promoteItem(index);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot promote task Task2 to be a project: Foo"));
    }

    void shouldGetAnErrorMessageWhenUpdateTaskFailed()
    {
        // GIVEN

        // One project
        auto project = Domain::Project::Ptr::create();
        project->setName("Project1");

        // One note and one task
        auto rootTask = Domain::Task::Ptr::create();
        rootTask->setTitle("rootTask");
        auto topLevelProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto topLevelResult = Domain::QueryResult<Domain::Task::Ptr>::create(topLevelProvider);
        topLevelProvider->append(rootTask);

        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findTopLevel).when(project).thenReturn(topLevelResult);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(rootTask).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::ProjectPageModel page(project,
                                            projectQueriesMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        QAbstractItemModel *model = page.centralListModel();
        const QModelIndex rootTaskIndex = model->index(0, 0);
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        taskRepositoryMock(&Domain::TaskRepository::update).when(rootTask).thenReturn(job);

        QVERIFY(model->setData(rootTaskIndex, "newRootTask"));

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot modify task rootTask in project Project1: Foo"));
    }

    void shouldGetAnErrorMessageWhenAssociateTaskFailed()
    {
        // GIVEN

        // One project
        auto project = Domain::Project::Ptr::create();
        project->setName("Project1");

        // One note and one task
        auto rootTask = Domain::Task::Ptr::create();
        rootTask->setTitle("rootTask");
        auto topLevelProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto topLevelResult = Domain::QueryResult<Domain::Task::Ptr>::create(topLevelProvider);
        topLevelProvider->append(rootTask);

        Utils::MockObject<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findTopLevel).when(project).thenReturn(topLevelResult);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(rootTask).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::ProjectPageModel page(project,
                                            projectQueriesMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        QAbstractItemModel *model = page.centralListModel();
        const QModelIndex rootTaskIndex = model->index(0, 0);
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN
        auto childTask3 = Domain::Task::Ptr::create();
        childTask3->setTitle("childTask3");
        auto childTask4 = Domain::Task::Ptr::create();
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask3).thenReturn(job);
        taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask4).thenReturn(new FakeJob(this));
        auto data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << childTask3 << childTask4));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, rootTaskIndex);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot move task childTask3 as a sub-task of rootTask: Foo"));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask4).exactly(1));
    }
};

QTEST_MAIN(ProjectPageModelTest)

#include "projectpagemodeltest.moc"
