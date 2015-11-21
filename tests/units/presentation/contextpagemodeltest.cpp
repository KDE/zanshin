/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Rémi Benoit <r3m1.benoit@gmail.com>

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

#include "domain/context.h"
#include "domain/task.h"
#include "domain/contextqueries.h"
#include "domain/taskqueries.h"

#include "domain/contextqueries.h"
#include "domain/contextrepository.h"
#include "domain/taskrepository.h"
#include "domain/noterepository.h"

#include "presentation/contextpagemodel.h"
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

class ContextPageModelTest : public QObject
{
    Q_OBJECT

private slots:
    void shouldListAssociatedTaskInContextCentralListView() {
        // GIVEN

        // A context
        auto context = Domain::Context::Ptr::create();

        // A parent task and a child task
        auto parentTask = Domain::Task::Ptr::create();
        parentTask->setTitle("A parent task");
        auto childTask = Domain::Task::Ptr::create();
        childTask->setTitle("A child task");

        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(parentTask);
        taskProvider->append(childTask);
        //FIXME : for now findTopLevelTasks(context) returns all tasks associated not just top level ones

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findTopLevelTasks).when(context).thenReturn(taskResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        // WHEN
        QAbstractItemModel *model = page.centralListModel();

        // THEN
        const QModelIndex parentTaskIndex = model->index(0, 0);
        const QModelIndex childTaskIndex = model->index(1, 0);

        QCOMPARE(page.context(), context);

        QCOMPARE(model->rowCount(), 2);
        QCOMPARE(model->rowCount(parentTaskIndex), 0);
        QCOMPARE(model->rowCount(childTaskIndex), 0);

        const Qt::ItemFlags taskFlags = Qt::ItemIsSelectable
                                      | Qt::ItemIsEnabled
                                      | Qt::ItemIsEditable
                                      | Qt::ItemIsDragEnabled
                                      | Qt::ItemIsUserCheckable
                                      | Qt::ItemIsDropEnabled;
        QCOMPARE(model->flags(parentTaskIndex), taskFlags);
        QCOMPARE(model->flags(childTaskIndex), taskFlags);

        QCOMPARE(model->data(parentTaskIndex).toString(), parentTask->title());
        QCOMPARE(model->data(childTaskIndex).toString(), childTask->title());

        QCOMPARE(model->data(parentTaskIndex, Qt::EditRole).toString(), parentTask->title());
        QCOMPARE(model->data(childTaskIndex, Qt::EditRole).toString(), childTask->title());

        QVERIFY(model->data(parentTaskIndex, Qt::CheckStateRole).isValid());
        QVERIFY(model->data(childTaskIndex, Qt::CheckStateRole).isValid());

        QCOMPARE(model->data(parentTaskIndex, Qt::CheckStateRole).toBool(), parentTask->isDone());
        QCOMPARE(model->data(childTaskIndex, Qt::CheckStateRole).toBool(), childTask->isDone());

        QVERIFY(!model->data(parentTaskIndex, Qt::FontRole).isValid());
        QVERIFY(!model->data(parentTaskIndex, Qt::ToolTipRole).isValid());

        // WHEN
        taskRepositoryMock(&Domain::TaskRepository::update).when(parentTask).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::update).when(childTask).thenReturn(new FakeJob(this));

        QVERIFY(model->setData(parentTaskIndex, "newParentTask"));
        QVERIFY(model->setData(childTaskIndex, "newChildTask"));

        QVERIFY(model->setData(parentTaskIndex, Qt::Checked, Qt::CheckStateRole));
        QVERIFY(model->setData(childTaskIndex, Qt::Unchecked, Qt::CheckStateRole));

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(parentTask).exactly(2));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(childTask).exactly(2));

        QCOMPARE(parentTask->title(), QString("newParentTask"));
        QCOMPARE(childTask->title(), QString("newChildTask"));

        QCOMPARE(parentTask->isDone(), true);
        QCOMPARE(childTask->isDone(), false);

        // WHEN
        QVERIFY(!model->setData(parentTaskIndex, QVariant(), Qt::WhatsThisRole));
        QVERIFY(!model->setData(parentTaskIndex, QVariant(), Qt::ForegroundRole));
        QVERIFY(!model->setData(parentTaskIndex, QVariant(), Qt::InitialSortOrderRole));

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(parentTask).exactly(2));

        QCOMPARE(parentTask->title(), QString("newParentTask"));
        QCOMPARE(childTask->title(), QString("newChildTask"));

        // WHEN a task is dragged
        QMimeData *data = model->mimeData(QModelIndexList() << childTaskIndex);

        // THEN
        QVERIFY(data->hasFormat("application/x-zanshin-object"));
        QCOMPARE(data->property("objects").value<Domain::Artifact::List>(),
                 Domain::Artifact::List() << childTask);

        // WHEN a task is dropped
        auto childTask2 = Domain::Task::Ptr::create();
        taskRepositoryMock(&Domain::TaskRepository::associate).when(parentTask, childTask2).thenReturn(new FakeJob(this));
        data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << childTask2));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, parentTaskIndex);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(parentTask, childTask2).exactly(1));

        // WHEN two tasks are dropped
        auto childTask3 = Domain::Task::Ptr::create();
        auto childTask4 = Domain::Task::Ptr::create();
        taskRepositoryMock(&Domain::TaskRepository::associate).when(parentTask, childTask3).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::associate).when(parentTask, childTask4).thenReturn(new FakeJob(this));
        data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << childTask3 << childTask4));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, parentTaskIndex);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(parentTask, childTask3).exactly(1));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(parentTask, childTask4).exactly(1));

        // WHEN a task and a note are dropped
        Domain::Artifact::Ptr childTask5(new Domain::Task);
        Domain::Artifact::Ptr childNote(new Domain::Note);
        data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << childTask5 << childNote));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, parentTaskIndex);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(parentTask, childTask5.objectCast<Domain::Task>()).exactly(0));
    }

    void shouldAddTasks()
    {
        // GIVEN

        // One Context
        auto context = Domain::Context::Ptr::create();

        // ... in fact we won't list any model
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        // We'll gladly create a task though
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::createInContext).when(any<Domain::Task::Ptr>(),
                                                                          any<Domain::Context::Ptr>())
                                                                    .thenReturn(new FakeJob(this));

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        // WHEN
         auto title = QString("New task");
         auto task = page.addItem(title).objectCast<Domain::Task>();

         // THEN
         QVERIFY(taskRepositoryMock(&Domain::TaskRepository::createInContext).when(any<Domain::Task::Ptr>(),
                                                                                        any<Domain::Context::Ptr>())
                                                                                    .exactly(1));
         QVERIFY(task);
         QCOMPARE(task->title(), title);
    }

    void shouldRemoveItem()
    {
        // GIVEN

        // One context
        auto context = Domain::Context::Ptr::create();

        // A task
        auto task = Domain::Task::Ptr::create();

        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(task);

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findTopLevelTasks).when(context).thenReturn(taskResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;
        contextRepositoryMock(&Domain::ContextRepository::dissociate).when(context, task).thenReturn(new FakeJob(this));

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        // WHEN
        const QModelIndex indexTask = page.centralListModel()->index(0, 0);
        page.removeItem(indexTask);

        // THEN
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::dissociate).when(context, task).exactly(1));
    }

    void shouldPromoteItem()
    {
        // GIVEN

        // One context
        auto context = Domain::Context::Ptr::create();

        // A task
        auto task = Domain::Task::Ptr::create();

        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(task);

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findTopLevelTasks).when(context).thenReturn(taskResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::promoteToProject).when(task).thenReturn(new FakeJob(this));

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        // WHEN
        const QModelIndex indexTask = page.centralListModel()->index(0, 0);
        page.promoteItem(indexTask);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::promoteToProject).when(task).exactly(1));
    }

    void shouldGetAnErrorMessageWhenAddTaskFailed()
    {
        // GIVEN

        // One Context
        auto context = Domain::Context::Ptr::create();
        context->setName("Context1");

        // ... in fact we won't list any model
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        // We'll gladly create a task though
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        taskRepositoryMock(&Domain::TaskRepository::createInContext).when(any<Domain::Task::Ptr>(),
                                                                          any<Domain::Context::Ptr>())
                                                                    .thenReturn(job);

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN
        page.addItem("New task");

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot add task New task in context Context1: Foo"));
    }

    void shouldGetAnErrorMessageWhenUpdateTaskFailed()
    {
        // GIVEN

        // A context
        auto context = Domain::Context::Ptr::create();
        context->setName("Context1");

        // A task
        auto task = Domain::Task::Ptr::create();
        task->setTitle("A task");

        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(task);

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findTopLevelTasks).when(context).thenReturn(taskResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        QAbstractItemModel *model = page.centralListModel();
        const QModelIndex taskIndex = model->index(0, 0);
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        taskRepositoryMock(&Domain::TaskRepository::update).when(task).thenReturn(job);

        QVERIFY(model->setData(taskIndex, "newTask"));

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot modify task A task in context Context1: Foo"));
    }

    void shouldGetAnErrorMessageWhenAssociateTaskFailed()
    {
        // GIVEN

        // A context
        auto context = Domain::Context::Ptr::create();
        context->setName("Context1");

        // A parent task and a child task
        auto parentTask = Domain::Task::Ptr::create();
        parentTask->setTitle("A parent task");
        auto childTask = Domain::Task::Ptr::create();
        childTask->setTitle("A child task");

        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(parentTask);
        taskProvider->append(childTask);

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findTopLevelTasks).when(context).thenReturn(taskResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        // WHEN
        QAbstractItemModel *model = page.centralListModel();
        const QModelIndex parentTaskIndex = model->index(0, 0);
        const QModelIndex childTaskIndex = model->index(1, 0);
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN a task is dropped
        auto childTask2 = Domain::Task::Ptr::create();
        childTask2->setTitle("childTask2");
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        taskRepositoryMock(&Domain::TaskRepository::associate).when(parentTask, childTask2).thenReturn(job);
        auto data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << childTask2));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, parentTaskIndex);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot move task childTask2 as sub-task of A parent task: Foo"));
    }
};

QTEST_MAIN(ContextPageModelTest)

#include "contextpagemodeltest.moc"
