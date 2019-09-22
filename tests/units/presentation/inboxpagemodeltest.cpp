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

#include <testlib/qtest_zanshin.h>

#include <memory>

#include <QMimeData>

#include "utils/mockobject.h"

#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/inboxpagemodel.h"
#include "presentation/errorhandler.h"
#include "presentation/querytreemodelbase.h"

#include "akonadi/akonadiserializerinterface.h"

#include "utils/dependencymanager.h"
#include "integration/dependencies.h"

#include "testlib/fakejob.h"
#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/gentodo.h"
#include "testlib/testhelpers.h"

using namespace Testlib;
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

class InboxPageModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldListInboxInCentralListModel()
    {
        // GIVEN

        AkonadiFakeData data;
        auto deps = data.createDependencies();
        Integration::initializeDefaultDomainDependencies(*deps.get());

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One root task
        data.createItem(GenTodo().withId(1).withParent(42).withUid("1").withTitle(QStringLiteral("rootTask")));

        // One task under the root task
        data.createItem(GenTodo().withId(2).withParent(42).withUid("2").withParentUid("1").done(true).withTitle(QStringLiteral("childTask")));

        auto serializer = deps->create<Akonadi::SerializerInterface>();
        Presentation::InboxPageModel inbox(deps->create<Domain::TaskQueries>(),
                                           deps->create<Domain::TaskRepository>());

        // WHEN
        QAbstractItemModel *model = inbox.centralListModel();
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        const QModelIndex rootTaskIndex = model->index(0, 0);
        const QModelIndex childTaskIndex = model->index(0, 0, rootTaskIndex);

        QCOMPARE(model->rowCount(), 1);
        QCOMPARE(model->rowCount(rootTaskIndex), 1);
        QCOMPARE(model->rowCount(childTaskIndex), 0);

        auto rootTask = model->data(rootTaskIndex, Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        auto childTask = model->data(childTaskIndex, Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();

        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled
                                         | Qt::ItemIsEditable
                                         | Qt::ItemIsDragEnabled;
        QCOMPARE(model->flags(rootTaskIndex), defaultFlags | Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(childTaskIndex), defaultFlags | Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled);

        QCOMPARE(model->data(rootTaskIndex).toString(), rootTask->title());
        QCOMPARE(model->data(childTaskIndex).toString(), childTask->title());

        QCOMPARE(model->data(rootTaskIndex, Qt::EditRole).toString(), rootTask->title());
        QCOMPARE(model->data(childTaskIndex, Qt::EditRole).toString(), childTask->title());

        QVERIFY(model->data(rootTaskIndex, Qt::CheckStateRole).isValid());
        QVERIFY(model->data(childTaskIndex, Qt::CheckStateRole).isValid());

        QCOMPARE(model->data(rootTaskIndex, Qt::CheckStateRole).toBool(), rootTask->isDone());
        QCOMPARE(model->data(childTaskIndex, Qt::CheckStateRole).toBool(), childTask->isDone());

        // WHEN
        QVERIFY(model->setData(rootTaskIndex, "newRootTask"));
        QVERIFY(model->setData(childTaskIndex, "newChildTask"));

        QVERIFY(model->setData(rootTaskIndex, Qt::Checked, Qt::CheckStateRole));
        QVERIFY(model->setData(childTaskIndex, Qt::Unchecked, Qt::CheckStateRole));

        // THEN
        QCOMPARE(rootTask->title(), QStringLiteral("newRootTask"));
        QCOMPARE(childTask->title(), QStringLiteral("newChildTask"));

        QCOMPARE(rootTask->isDone(), true);
        QCOMPARE(childTask->isDone(), false);

        // WHEN
        auto mimeData = std::unique_ptr<QMimeData>(model->mimeData(QModelIndexList() << childTaskIndex));

        // THEN
        QVERIFY(mimeData->hasFormat(QStringLiteral("application/x-zanshin-object")));
        QCOMPARE(mimeData->property("objects").value<Domain::Task::List>(),
                 Domain::Task::List() << childTask);

        // WHEN
        // - root (1)
        //   - child (2)
        //     - grandchild (48), will be dropped onto root
        data.createItem(GenTodo().withId(48).withParent(42).withUid("48").withParentUid("2").withTitle(QStringLiteral("childTask2")));
        QCOMPARE(model->rowCount(childTaskIndex), 1);
        auto grandChildTask = model->data(model->index(0, 0, childTaskIndex), Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        QVERIFY(grandChildTask);
        mimeData.reset(new QMimeData);
        mimeData->setData(QStringLiteral("application/x-zanshin-object"), "object");
        mimeData->setProperty("objects", QVariant::fromValue(Domain::Task::List() << grandChildTask));
        QVERIFY(model->dropMimeData(mimeData.get(), Qt::MoveAction, -1, -1, rootTaskIndex));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        // root (1)
        // - child (2)
        // - second child (48)
        QCOMPARE(serializer->relatedUidFromItem(data.item(48)), QStringLiteral("1"));
        QCOMPARE(model->rowCount(), 1);
        QCOMPARE(model->rowCount(rootTaskIndex), 2);
        QCOMPARE(model->rowCount(childTaskIndex), 0);

        // WHEN
        // two more toplevel tasks
        data.createItem(GenTodo().withId(49).withParent(42).withUid("49").withTitle(QStringLiteral("childTask3")));
        auto task3 = model->data(model->index(1, 0), Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        QVERIFY(task3);
        data.createItem(GenTodo().withId(50).withParent(42).withUid("50").withTitle(QStringLiteral("childTask4")));
        auto task4 = model->data(model->index(2, 0), Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        QVERIFY(task4);
        mimeData.reset(new QMimeData);
        mimeData->setData(QStringLiteral("application/x-zanshin-object"), "object");
        mimeData->setProperty("objects", QVariant::fromValue(Domain::Task::List() << task3 << task4));
        QVERIFY(model->dropMimeData(mimeData.get(), Qt::MoveAction, -1, -1, rootTaskIndex));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        // root (1)
        // - child (2)
        // - second child (48)
        // - task3 (49)
        // - task4 (50)

        QCOMPARE(serializer->relatedUidFromItem(data.item(49)), QStringLiteral("1"));
        QCOMPARE(serializer->relatedUidFromItem(data.item(50)), QStringLiteral("1"));
        QCOMPARE(model->rowCount(), 1);
        QCOMPARE(model->rowCount(rootTaskIndex), 4);
    }

    void shouldAddTasksInInbox()
    {
        // GIVEN

        // ... in fact we won't list any model
        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;

        // We'll gladly create a task though
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::create).when(any<Domain::Task::Ptr>()).thenReturn(new FakeJob(this));

        Presentation::InboxPageModel inbox(taskQueriesMock.getInstance(),
                                               taskRepositoryMock.getInstance());

        // WHEN
        auto title = QStringLiteral("New task");
        auto task = inbox.addItem(title);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::create).when(any<Domain::Task::Ptr>()).exactly(1));
        QVERIFY(task);
        QCOMPARE(task->title(), title);
    }

    void shouldAddChildTask()
    {
        // GIVEN

        // Two tasks
        auto task1 = Domain::Task::Ptr::create();
        auto task2 = Domain::Task::Ptr::create();
        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(task1);
        taskProvider->append(task2);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findInboxTopLevel).when().thenReturn(taskResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findDataSource).when(any<Domain::Task::Ptr>()).thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::createChild).when(any<Domain::Task::Ptr>(),
                                                                      any<Domain::Task::Ptr>())
                                                                .thenReturn(new FakeJob(this));

        Presentation::InboxPageModel inbox(taskQueriesMock.getInstance(),
                                               taskRepositoryMock.getInstance());

        // WHEN
        const auto title = QStringLiteral("New task");
        const auto parentIndex = inbox.centralListModel()->index(0, 0);
        const auto createdTask = inbox.addItem(title, parentIndex);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::createChild).when(any<Domain::Task::Ptr>(),
                                                                              any<Domain::Task::Ptr>())
                                                                        .exactly(1));
        QVERIFY(createdTask);
        QCOMPARE(createdTask->title(), title);
    }

    void shouldGetAnErrorMessageWhenAddTaskFailed()
    {
        // GIVEN

        // ... in fact we won't list any model
        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;

        // We'll gladly create a task though
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        taskRepositoryMock(&Domain::TaskRepository::create).when(any<Domain::Task::Ptr>()).thenReturn(job);

        Presentation::InboxPageModel inbox(taskQueriesMock.getInstance(),
                                               taskRepositoryMock.getInstance());

        FakeErrorHandler errorHandler;
        inbox.setErrorHandler(&errorHandler);

        // WHEN
        inbox.addItem(QStringLiteral("New task"));

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot add task New task in Inbox: Foo"));
    }

    void shouldDeleteItems()
    {
        // GIVEN

        // Two tasks
        auto task1 = Domain::Task::Ptr::create();
        auto task2 = Domain::Task::Ptr::create();
        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(task1);
        taskProvider->append(task2);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findInboxTopLevel).when().thenReturn(taskResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findDataSource).when(any<Domain::Task::Ptr>()).thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::remove).when(task2).thenReturn(new FakeJob(this));

        Presentation::InboxPageModel inbox(taskQueriesMock.getInstance(),
                                               taskRepositoryMock.getInstance());

        // WHEN
        const QModelIndex index = inbox.centralListModel()->index(1, 0);
        inbox.removeItem(index);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::remove).when(task2).exactly(1));
    }

    void shouldGetAnErrorMessageWhenDeleteItemsFailed()
    {
        // GIVEN

        // Two tasks
        auto task1 = Domain::Task::Ptr::create();
        auto task2 = Domain::Task::Ptr::create();
        task2->setTitle(QStringLiteral("task2"));
        auto inboxProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto inboxResult = Domain::QueryResult<Domain::Task::Ptr>::create(inboxProvider);
        inboxProvider->append(task1);
        inboxProvider->append(task2);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findInboxTopLevel).when().thenReturn(inboxResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findDataSource).when(any<Domain::Task::Ptr>()).thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        taskRepositoryMock(&Domain::TaskRepository::remove).when(task2).thenReturn(job);

        Presentation::InboxPageModel inbox(taskQueriesMock.getInstance(),
                                               taskRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        inbox.setErrorHandler(&errorHandler);

        // WHEN
        const QModelIndex index = inbox.centralListModel()->index(1, 0);
        inbox.removeItem(index);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot remove task task2 from Inbox: Foo"));
    }

    void shouldPromoteItem()
    {
        // GIVEN

        // Two tasks
        auto task1 = Domain::Task::Ptr::create();
        auto task2 = Domain::Task::Ptr::create();
        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(task1);
        taskProvider->append(task2);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findInboxTopLevel).when().thenReturn(taskResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findDataSource).when(any<Domain::Task::Ptr>()).thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::promoteToProject).when(task2).thenReturn(new FakeJob(this));

        Presentation::InboxPageModel inbox(taskQueriesMock.getInstance(),
                                               taskRepositoryMock.getInstance());

        // WHEN
        const QModelIndex index = inbox.centralListModel()->index(1, 0);
        inbox.promoteItem(index);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::promoteToProject).when(task2).exactly(1));
    }

    void shouldGetAnErrorMessageWhenPromoteItemFailed()
    {
        // GIVEN

        // Two tasks
        auto task1 = Domain::Task::Ptr::create();
        auto task2 = Domain::Task::Ptr::create();
        task2->setTitle(QStringLiteral("task2"));
        auto inboxProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto inboxResult = Domain::QueryResult<Domain::Task::Ptr>::create(inboxProvider);
        inboxProvider->append(task1);
        inboxProvider->append(task2);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findInboxTopLevel).when().thenReturn(inboxResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findDataSource).when(any<Domain::Task::Ptr>()).thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        taskRepositoryMock(&Domain::TaskRepository::promoteToProject).when(task2).thenReturn(job);

        Presentation::InboxPageModel inbox(taskQueriesMock.getInstance(),
                                               taskRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        inbox.setErrorHandler(&errorHandler);

        // WHEN
        const QModelIndex index = inbox.centralListModel()->index(1, 0);
        inbox.promoteItem(index);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot promote task task2 to be a project: Foo"));
    }

    void shouldGetAnErrorMessageWhenUpdateTaskFailed()
    {
        // GIVEN

        // One task
        auto rootTask = Domain::Task::Ptr::create();
        rootTask->setTitle(QStringLiteral("rootTask"));
        auto inboxProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto inboxResult = Domain::QueryResult<Domain::Task::Ptr>::create(inboxProvider);
        inboxProvider->append(rootTask);
        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findInboxTopLevel).when().thenReturn(inboxResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(rootTask).thenReturn(taskResult);
        taskQueriesMock(&Domain::TaskQueries::findDataSource).when(any<Domain::Task::Ptr>()).thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::InboxPageModel inbox(taskQueriesMock.getInstance(),
                                               taskRepositoryMock.getInstance());

        QAbstractItemModel *model = inbox.centralListModel();
        const QModelIndex rootTaskIndex = model->index(0, 0);
        FakeErrorHandler errorHandler;
        inbox.setErrorHandler(&errorHandler);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        taskRepositoryMock(&Domain::TaskRepository::update).when(rootTask).thenReturn(job);

        QVERIFY(model->setData(rootTaskIndex, "newRootTask"));

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot modify task rootTask in Inbox: Foo"));
    }

    void shouldGetAnErrorMessageWhenAssociateTaskFailed()
    {
        // GIVEN

        // One task
        auto rootTask = Domain::Task::Ptr::create();
        rootTask->setTitle(QStringLiteral("rootTask"));
        auto inboxProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto inboxResult = Domain::QueryResult<Domain::Task::Ptr>::create(inboxProvider);
        inboxProvider->append(rootTask);
        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findInboxTopLevel).when().thenReturn(inboxResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(rootTask).thenReturn(taskResult);
        taskQueriesMock(&Domain::TaskQueries::findDataSource).when(any<Domain::Task::Ptr>()).thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::InboxPageModel inbox(taskQueriesMock.getInstance(),
                                               taskRepositoryMock.getInstance());

        QAbstractItemModel *model = inbox.centralListModel();
        const QModelIndex rootTaskIndex = model->index(0, 0);
        FakeErrorHandler errorHandler;
        inbox.setErrorHandler(&errorHandler);

        // WHEN
        auto childTask3 = Domain::Task::Ptr::create();
        childTask3->setTitle(QStringLiteral("childTask3"));
        auto childTask4 = Domain::Task::Ptr::create();
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask3).thenReturn(job);
        taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask4).thenReturn(new FakeJob(this));
        auto data = std::make_unique<QMimeData>();
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Task::List() << childTask3 << childTask4));
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, rootTaskIndex);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot move task childTask3 as sub-task of rootTask: Foo"));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask4).exactly(1));
    }

    void shouldDeparentWhenNoErrorHappens()
    {
        // GIVEN

        // One task, top level
        auto topLevelTask = Domain::Task::Ptr::create();
        topLevelTask->setTitle(QStringLiteral("topLevelTask"));
        auto inboxProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto inboxResult = Domain::QueryResult<Domain::Task::Ptr>::create(inboxProvider);
        inboxProvider->append(topLevelTask);

        // Two tasks under the top level task
        auto childTask = Domain::Task::Ptr::create();
        childTask->setTitle(QStringLiteral("childTask"));
        childTask->setDone(true);
        auto childTask2 = Domain::Task::Ptr::create();
        childTask2->setTitle(QStringLiteral("childTask2"));
        childTask2->setDone(false);
        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(childTask);
        taskProvider->append(childTask2);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findInboxTopLevel).when().thenReturn(inboxResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(topLevelTask).thenReturn(taskResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(childTask).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(childTask2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findDataSource).when(childTask).thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::InboxPageModel inbox(taskQueriesMock.getInstance(),
                                               taskRepositoryMock.getInstance());

        QAbstractItemModel *model = inbox.centralListModel();
        const QModelIndex emptyPartModel = QModelIndex(); // model root, drop on the empty part is equivalent
        FakeErrorHandler errorHandler;
        inbox.setErrorHandler(&errorHandler);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));

        taskRepositoryMock(&Domain::TaskRepository::dissociate).when(childTask).thenReturn(job);
        taskRepositoryMock(&Domain::TaskRepository::dissociate).when(childTask2).thenReturn(new FakeJob(this));

        auto data = std::make_unique<QMimeData>();
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Task::List() << childTask << childTask2)); // both will be DnD on the empty part
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, emptyPartModel);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot deparent task childTask from its parent: Foo"));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::dissociate).when(childTask).exactly(1));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::dissociate).when(childTask2).exactly(1));
    }
};

ZANSHIN_TEST_MAIN(InboxPageModelTest)

#include "inboxpagemodeltest.moc"
