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

#include <testlib/qtest_zanshin.h>

#include <memory>

#include <QMimeData>

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

        // Three tasks
        auto task1 = Domain::Task::Ptr::create();
        task1->setTitle(QStringLiteral("task1"));
        auto task2 = Domain::Task::Ptr::create();
        task2->setTitle(QStringLiteral("task2"));
        auto task3 = Domain::Task::Ptr::create();
        task3->setTitle(QStringLiteral("task3"));

        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(task1);
        taskProvider->append(task2);
        taskProvider->append(task3);

        // Two tasks under the task1
        auto childTask11 = Domain::Task::Ptr::create();
        childTask11->setTitle(QStringLiteral("childTask11"));
        auto childTask12 = Domain::Task::Ptr::create();
        childTask12->setTitle(QStringLiteral("childTask12"));
        auto childTaskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto childTaskResult = Domain::QueryResult<Domain::Task::Ptr>::create(childTaskProvider);
        taskProvider->append(childTask12);
        childTaskProvider->append(childTask11);
        childTaskProvider->append(childTask12);

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findTopLevelTasks).when(context).thenReturn(taskResult);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(childTaskResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task3).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(childTask11).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(childTask12).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        // WHEN
        QAbstractItemModel *model = page.centralListModel();

        // THEN
        const QModelIndex task1Index = model->index(0, 0);
        const QModelIndex task2Index = model->index(1, 0);
        const QModelIndex task3Index = model->index(2, 0);
        const QModelIndex taskChildTask12Index = model->index(3, 0);

        const QModelIndex childTask11Index = model->index(0, 0, task1Index);
        const QModelIndex childTask12Index = model->index(1, 0, task1Index);

        QCOMPARE(page.context(), context);

        QCOMPARE(model->rowCount(), 4);
        QCOMPARE(model->rowCount(task1Index), 2);
        QCOMPARE(model->rowCount(task2Index), 0);
        QCOMPARE(model->rowCount(task3Index), 0);
        QCOMPARE(model->rowCount(taskChildTask12Index), 0);

        QVERIFY(childTask11Index.isValid());
        QVERIFY(childTask12Index.isValid());
        QCOMPARE(model->rowCount(childTask11Index), 0);
        QCOMPARE(model->rowCount(childTask12Index), 0);

        const Qt::ItemFlags taskFlags = Qt::ItemIsSelectable
                                      | Qt::ItemIsEnabled
                                      | Qt::ItemIsEditable
                                      | Qt::ItemIsDragEnabled
                                      | Qt::ItemIsUserCheckable
                                      | Qt::ItemIsDropEnabled;
        QCOMPARE(model->flags(task1Index), taskFlags);
        QCOMPARE(model->flags(childTask11Index), taskFlags);
        QCOMPARE(model->flags(childTask12Index), taskFlags);
        QCOMPARE(model->flags(task2Index), taskFlags);
        QCOMPARE(model->flags(task3Index), taskFlags);
        QCOMPARE(model->flags(taskChildTask12Index), taskFlags);

        QCOMPARE(model->data(task1Index).toString(), task1->title());
        QCOMPARE(model->data(childTask11Index).toString(), childTask11->title());
        QCOMPARE(model->data(childTask12Index).toString(), childTask12->title());
        QCOMPARE(model->data(task2Index).toString(), task2->title());
        QCOMPARE(model->data(task3Index).toString(), task3->title());
        QCOMPARE(model->data(taskChildTask12Index).toString(), childTask12->title());

        QCOMPARE(model->data(task1Index, Qt::EditRole).toString(), task1->title());
        QCOMPARE(model->data(childTask11Index, Qt::EditRole).toString(), childTask11->title());
        QCOMPARE(model->data(childTask12Index, Qt::EditRole).toString(), childTask12->title());
        QCOMPARE(model->data(task2Index, Qt::EditRole).toString(), task2->title());
        QCOMPARE(model->data(task3Index, Qt::EditRole).toString(), task3->title());
        QCOMPARE(model->data(taskChildTask12Index, Qt::EditRole).toString(), childTask12->title());

        QVERIFY(model->data(task1Index, Qt::CheckStateRole).isValid());
        QVERIFY(model->data(childTask11Index, Qt::CheckStateRole).isValid());
        QVERIFY(model->data(childTask12Index, Qt::CheckStateRole).isValid());
        QVERIFY(model->data(task2Index, Qt::CheckStateRole).isValid());
        QVERIFY(model->data(task3Index, Qt::CheckStateRole).isValid());
        QVERIFY(model->data(taskChildTask12Index, Qt::CheckStateRole).isValid());

        QCOMPARE(model->data(task1Index, Qt::CheckStateRole).toBool(), task1->isDone());
        QCOMPARE(model->data(childTask11Index, Qt::CheckStateRole).toBool(), childTask11->isDone());
        QCOMPARE(model->data(childTask12Index, Qt::CheckStateRole).toBool(), childTask12->isDone());
        QCOMPARE(model->data(task2Index, Qt::CheckStateRole).toBool(), task2->isDone());
        QCOMPARE(model->data(task3Index, Qt::CheckStateRole).toBool(), task3->isDone());
        QCOMPARE(model->data(taskChildTask12Index, Qt::CheckStateRole).toBool(), childTask12->isDone());

        // WHEN
        taskRepositoryMock(&Domain::TaskRepository::update).when(task1).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::update).when(childTask11).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::update).when(childTask12).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::update).when(task2).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::update).when(task3).thenReturn(new FakeJob(this));

        QVERIFY(model->setData(task1Index, "newTask1"));
        QVERIFY(model->setData(childTask11Index, "newChildTask11"));
        QVERIFY(model->setData(task2Index, "newTask2"));
        QVERIFY(model->setData(task3Index, "newTask3"));
        QVERIFY(model->setData(taskChildTask12Index, "newChildTask12"));

        QVERIFY(model->setData(task1Index, Qt::Unchecked, Qt::CheckStateRole));
        QVERIFY(model->setData(childTask11Index, Qt::Unchecked, Qt::CheckStateRole));
        QVERIFY(model->setData(task2Index, Qt::Checked, Qt::CheckStateRole));
        QVERIFY(model->setData(task3Index, Qt::Unchecked, Qt::CheckStateRole));
        QVERIFY(model->setData(taskChildTask12Index, Qt::Checked, Qt::CheckStateRole));

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(task1).exactly(2));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(childTask11).exactly(2));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(childTask12).exactly(2));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(task2).exactly(2));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(task3).exactly(2));

        QCOMPARE(task1->title(), QStringLiteral("newTask1"));
        QCOMPARE(childTask11->title(), QStringLiteral("newChildTask11"));
        QCOMPARE(childTask12->title(), QStringLiteral("newChildTask12"));
        QCOMPARE(task2->title(), QStringLiteral("newTask2"));
        QCOMPARE(task3->title(), QStringLiteral("newTask3"));

        QCOMPARE(task1->isDone(), false);
        QCOMPARE(childTask11->isDone(), false);
        QCOMPARE(childTask12->isDone(), true);
        QCOMPARE(task2->isDone(), true);
        QCOMPARE(task3->isDone(), false);

        // WHEN
        QVERIFY(!model->setData(task1Index, QVariant(), Qt::WhatsThisRole));
        QVERIFY(!model->setData(task1Index, QVariant(), Qt::ForegroundRole));
        QVERIFY(!model->setData(task1Index, QVariant(), Qt::InitialSortOrderRole));

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(task1).exactly(2));

        QCOMPARE(task1->title(), QStringLiteral("newTask1"));
        QCOMPARE(task2->title(), QStringLiteral("newTask2"));

        // WHEN a task is dragged
        auto data = std::unique_ptr<QMimeData>(model->mimeData(QModelIndexList() << task2Index));

        // THEN
        QVERIFY(data->hasFormat(QStringLiteral("application/x-zanshin-object")));
        QCOMPARE(data->property("objects").value<Domain::Artifact::List>(),
                 Domain::Artifact::List() << task2);

        // WHEN a task is dropped
        auto childTask2 = Domain::Task::Ptr::create();
        taskRepositoryMock(&Domain::TaskRepository::associate).when(task1, childTask2).thenReturn(new FakeJob(this));
        data.reset(new QMimeData);
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << childTask2));
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, task1Index);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(task1, childTask2).exactly(1));

        // WHEN two tasks are dropped
        auto childTask3 = Domain::Task::Ptr::create();
        auto childTask4 = Domain::Task::Ptr::create();
        taskRepositoryMock(&Domain::TaskRepository::associate).when(task1, childTask3).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::associate).when(task1, childTask4).thenReturn(new FakeJob(this));
        data.reset(new QMimeData);
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << childTask3 << childTask4));
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, task1Index);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(task1, childTask3).exactly(1));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(task1, childTask4).exactly(1));

        // WHEN a task and a note are dropped
        Domain::Artifact::Ptr childTask5(new Domain::Task);
        Domain::Artifact::Ptr childNote(new Domain::Note);
        data.reset(new QMimeData);
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << childTask5 << childNote));
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, task1Index);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(task1, childTask5.objectCast<Domain::Task>()).exactly(0));
    }

    void shouldAddTasksInContext()
    {
        // GIVEN

        // One Context
        auto context = Domain::Context::Ptr::create();

        // ... in fact we won't list any model
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;
        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;

        // We'll gladly create a task though
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::createInContext).when(any<Domain::Task::Ptr>(),
                                                                          any<Domain::Context::Ptr>())
                                                                    .thenReturn(new FakeJob(this));

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        // WHEN
         auto title = QStringLiteral("New task");
         auto task = page.addItem(title).objectCast<Domain::Task>();

         // THEN
         QVERIFY(taskRepositoryMock(&Domain::TaskRepository::createInContext).when(any<Domain::Task::Ptr>(),
                                                                                        any<Domain::Context::Ptr>())
                                                                                    .exactly(1));
         QVERIFY(task);
         QCOMPARE(task->title(), title);
    }

    void shouldAddChildTask()
    {
        // GIVEN

        // One Context
        auto context = Domain::Context::Ptr::create();

        // A task
        auto task = Domain::Task::Ptr::create();

        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(task);

        auto childProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto childResult = Domain::QueryResult<Domain::Task::Ptr>::create(childProvider);

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findTopLevelTasks).when(context).thenReturn(taskResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task).thenReturn(childResult);

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::createChild).when(any<Domain::Task::Ptr>(),
                                                                      any<Domain::Task::Ptr>())
                                                                .thenReturn(new FakeJob(this));

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        // WHEN
        const auto title = QStringLiteral("New task");
        const auto parentIndex = page.centralListModel()->index(0, 0);
        const auto createdTask = page.addItem(title, parentIndex).objectCast<Domain::Task>();

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::createChild).when(any<Domain::Task::Ptr>(),
                                                                              any<Domain::Task::Ptr>())
                                                                        .exactly(1));
        QVERIFY(createdTask);
        QCOMPARE(createdTask->title(), title);
    }

    void shouldRemoveTopLevelItem()
    {
        // GIVEN

        // One context
        auto context = Domain::Context::Ptr::create();

        // A task
        auto task = Domain::Task::Ptr::create();

        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(task);

        auto childProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto childResult = Domain::QueryResult<Domain::Task::Ptr>::create(childProvider);

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findTopLevelTasks).when(context).thenReturn(taskResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;
        contextRepositoryMock(&Domain::ContextRepository::dissociate).when(context, task).thenReturn(new FakeJob(this));

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task).thenReturn(childResult);

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        // WHEN
        const QModelIndex indexTask = page.centralListModel()->index(0, 0);
        page.removeItem(indexTask);

        // THEN
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::dissociate).when(context, task).exactly(1));
    }

    void shouldRemoveChildItem()
    {
        // GIVEN

        // One context
        auto context = Domain::Context::Ptr::create();

        // A task...
        auto task = Domain::Task::Ptr::create();

        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(task);

        // ... with a child
        auto childTask = Domain::Task::Ptr::create();
        auto childProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto childResult = Domain::QueryResult<Domain::Task::Ptr>::create(childProvider);
        childProvider->append(childTask);

        auto emptyProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto emptyResult = Domain::QueryResult<Domain::Task::Ptr>::create(emptyProvider);

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findTopLevelTasks).when(context).thenReturn(taskResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task).thenReturn(childResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(childTask).thenReturn(emptyResult);

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::dissociate).when(childTask).thenReturn(new FakeJob(this));

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        // WHEN
        const auto taskIndex = page.centralListModel()->index(0, 0);
        const auto childTaskIndex = page.centralListModel()->index(0, 0, taskIndex);
        page.removeItem(childTaskIndex);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::dissociate).when(childTask).exactly(1));
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

        auto childProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto childResult = Domain::QueryResult<Domain::Task::Ptr>::create(childProvider);

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findTopLevelTasks).when(context).thenReturn(taskResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task).thenReturn(childResult);

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::promoteToProject).when(task).thenReturn(new FakeJob(this));

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskQueriesMock.getInstance(),
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
        context->setName(QStringLiteral("Context1"));

        // ... in fact we won't list any model
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;
        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;

        // We'll gladly create a task though
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        taskRepositoryMock(&Domain::TaskRepository::createInContext).when(any<Domain::Task::Ptr>(),
                                                                          any<Domain::Context::Ptr>())
                                                                    .thenReturn(job);

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN
        page.addItem(QStringLiteral("New task"));

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot add task New task in context Context1: Foo"));
    }

    void shouldGetAnErrorMessageWhenUpdateTaskFailed()
    {
        // GIVEN

        // A context
        auto context = Domain::Context::Ptr::create();
        context->setName(QStringLiteral("Context1"));

        // A task
        auto task = Domain::Task::Ptr::create();
        task->setTitle(QStringLiteral("A task"));

        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(task);

        auto childProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto childResult = Domain::QueryResult<Domain::Task::Ptr>::create(childProvider);

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findTopLevelTasks).when(context).thenReturn(taskResult);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task).thenReturn(childResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        QAbstractItemModel *model = page.centralListModel();
        const QModelIndex taskIndex = model->index(0, 0);
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        taskRepositoryMock(&Domain::TaskRepository::update).when(task).thenReturn(job);

        QVERIFY(model->setData(taskIndex, "newTask"));

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot modify task A task in context Context1: Foo"));
    }

    void shouldGetAnErrorMessageWhenAssociateTaskFailed()
    {
        // GIVEN

        // A context
        auto context = Domain::Context::Ptr::create();
        context->setName(QStringLiteral("Context1"));

        // A parent task and a child task
        auto parentTask = Domain::Task::Ptr::create();
        parentTask->setTitle(QStringLiteral("A parent task"));
        auto childTask = Domain::Task::Ptr::create();
        childTask->setTitle(QStringLiteral("A child task"));

        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(parentTask);
        taskProvider->append(childTask);

        auto childProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto childResult = Domain::QueryResult<Domain::Task::Ptr>::create(childProvider);

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findTopLevelTasks).when(context).thenReturn(taskResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(parentTask).thenReturn(childResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(childTask).thenReturn(childResult);

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        // WHEN
        QAbstractItemModel *model = page.centralListModel();
        const QModelIndex parentTaskIndex = model->index(0, 0);
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN a task is dropped
        auto childTask2 = Domain::Task::Ptr::create();
        childTask2->setTitle(QStringLiteral("childTask2"));
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        taskRepositoryMock(&Domain::TaskRepository::associate).when(parentTask, childTask2).thenReturn(job);
        auto data = std::make_unique<QMimeData>();
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << childTask2));
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, parentTaskIndex);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot move task childTask2 as sub-task of A parent task: Foo"));
    }

    void shouldAssociateToContextWithNoParentWhenDroppingOnEmptyArea()
    {
        // GIVEN

        // One context
        auto context = Domain::Context::Ptr::create();
        context->setName(QStringLiteral("Context"));

        // One top level task
        auto topLevelTask = Domain::Task::Ptr::create();
        topLevelTask->setTitle(QStringLiteral("rootTask"));
        auto topLevelProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto topLevelResult = Domain::QueryResult<Domain::Task::Ptr>::create(topLevelProvider);
        topLevelProvider->append(topLevelTask);

        // Two tasks under the top level task
        auto childTask1 = Domain::Task::Ptr::create();
        childTask1->setTitle(QStringLiteral("childTask1"));
        childTask1->setDone(true);
        auto childTask2 = Domain::Task::Ptr::create();
        childTask2->setTitle(QStringLiteral("childTask2"));
        childTask2->setDone(false);
        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(childTask1);
        taskProvider->append(childTask2);

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findTopLevelTasks).when(context).thenReturn(topLevelResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(topLevelTask).thenReturn(taskResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(childTask1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(childTask2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::ContextPageModel page(context,
                                            contextQueriesMock.getInstance(),
                                            contextRepositoryMock.getInstance(),
                                            taskQueriesMock.getInstance(),
                                            taskRepositoryMock.getInstance());

        auto model = page.centralListModel();

        // WHEN
        taskRepositoryMock(&Domain::TaskRepository::dissociate).when(childTask1).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::dissociate).when(childTask2).thenReturn(new FakeJob(this));
        contextRepositoryMock(&Domain::ContextRepository::associate).when(context, childTask1).thenReturn(new FakeJob(this));
        contextRepositoryMock(&Domain::ContextRepository::associate).when(context, childTask2).thenReturn(new FakeJob(this));

        auto data = std::make_unique<QMimeData>();
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << childTask1 << childTask2)); // both will be DnD on the empty part
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, QModelIndex());

        // THEN
        QTest::qWait(150);
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::dissociate).when(childTask1).exactly(1));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::dissociate).when(childTask2).exactly(1));
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::associate).when(context, childTask1).exactly(1));
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::associate).when(context, childTask2).exactly(1));
    }
};

ZANSHIN_TEST_MAIN(ContextPageModelTest)

#include "contextpagemodeltest.moc"
