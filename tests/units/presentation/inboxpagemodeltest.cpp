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
    void cleanup()
    {
        // The first call to QueryTreeModelBase::data triggers fetchTaskExtraData which creates jobs.
        // Wait for these to finish so they don't mess up subsequent tests
        TestHelpers::waitForEmptyJobQueue();
    }

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
        AkonadiFakeData data;
        auto deps = data.createDependencies();
        Integration::initializeDefaultDomainDependencies(*deps.get());
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        Presentation::InboxPageModel pageModel(deps->create<Domain::TaskQueries>(),
                                               deps->create<Domain::TaskRepository>());
        QAbstractItemModel *model = pageModel.centralListModel();

        // WHEN
        auto title = QStringLiteral("New task");
        auto createdTask = pageModel.addItem(title);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QVERIFY(createdTask);
        QCOMPARE(createdTask->title(), title);
        QCOMPARE(model->rowCount(), 1);
        auto taskInModel = model->data(model->index(0, 0), Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        QVERIFY(taskInModel);
        QCOMPARE(taskInModel->title(), createdTask->title());
        QCOMPARE(taskInModel->startDate(), createdTask->startDate());

        QCOMPARE(data.items().count(), 1);
        TestHelpers::waitForEmptyJobQueue();
    }

    void shouldAddChildTask()
    {
        // GIVEN
        AkonadiFakeData data;
        auto deps = data.createDependencies();
        Integration::initializeDefaultDomainDependencies(*deps.get());
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Two tasks
        data.createItem(GenTodo().withId(42).withParent(42).withUid("42").withTitle(QStringLiteral("task1")));
        data.createItem(GenTodo().withId(43).withParent(42).withUid("43").withTitle(QStringLiteral("task2")));
        QCOMPARE(data.items().count(), 2);

        Presentation::InboxPageModel pageModel(deps->create<Domain::TaskQueries>(),
                                                  deps->create<Domain::TaskRepository>());
        QAbstractItemModel *model = pageModel.centralListModel();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(model->rowCount(), 2);

        // WHEN
        const auto title = QStringLiteral("New task");
        const auto parentIndex = model->index(0, 0);
        const auto createdTask = pageModel.addItem(title, parentIndex);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QVERIFY(createdTask);
        QCOMPARE(createdTask->title(), title);

        QCOMPARE(model->rowCount(parentIndex), 1);
        auto taskInModel = model->data(model->index(0, 0, parentIndex), Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        QVERIFY(taskInModel);
        QCOMPARE(taskInModel->title(), createdTask->title());
        QCOMPARE(taskInModel->startDate(), createdTask->startDate());

        QCOMPARE(data.items().count(), 3);
        TestHelpers::waitForEmptyJobQueue();
    }

    void shouldGetAnErrorMessageWhenAddTaskFailed()
    {
        // GIVEN
        AkonadiFakeData data;
        auto deps = data.createDependencies();
        Integration::initializeDefaultDomainDependencies(*deps.get());
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        Presentation::InboxPageModel pageModel(deps->create<Domain::TaskQueries>(),
                                               deps->create<Domain::TaskRepository>());

        FakeErrorHandler errorHandler;
        pageModel.setErrorHandler(&errorHandler);

        // WHEN
        data.storageBehavior().setCreateNextItemError(1, QStringLiteral("Foo"));
        pageModel.addItem(QStringLiteral("New task"));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot add task New task in Inbox: Foo"));
    }

    void shouldDeleteItems()
    {
        // GIVEN
        AkonadiFakeData data;
        auto deps = data.createDependencies();
        Integration::initializeDefaultDomainDependencies(*deps.get());
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Two tasks
        data.createItem(GenTodo().withId(42).withParent(42).withUid("42").withTitle(QStringLiteral("task1")));
        data.createItem(GenTodo().withId(43).withParent(42).withUid("43").withTitle(QStringLiteral("task2")));
        QCOMPARE(data.items().count(), 2);

        Presentation::InboxPageModel pageModel(deps->create<Domain::TaskQueries>(),
                                                  deps->create<Domain::TaskRepository>());
        QAbstractItemModel *model = pageModel.centralListModel();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(model->rowCount(), 2);

        // WHEN
        const QModelIndex index = model->index(1, 0);
        QVERIFY(index.isValid());
        pageModel.removeItem(index);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(model->rowCount(), 1);
        QCOMPARE(data.items().count(), 1);
    }

    void shouldGetAnErrorMessageWhenDeleteItemsFailed()
    {
        // GIVEN
        AkonadiFakeData data;
        auto deps = data.createDependencies();
        Integration::initializeDefaultDomainDependencies(*deps.get());
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Two tasks
        data.createItem(GenTodo().withId(42).withParent(42).withUid("42").withTitle(QStringLiteral("task1")));
        data.createItem(GenTodo().withId(43).withParent(42).withUid("43").withTitle(QStringLiteral("task2")));
        QCOMPARE(data.items().count(), 2);

        Presentation::InboxPageModel pageModel(deps->create<Domain::TaskQueries>(),
                                                  deps->create<Domain::TaskRepository>());
        QAbstractItemModel *model = pageModel.centralListModel();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(model->rowCount(), 2);

        FakeErrorHandler errorHandler;
        pageModel.setErrorHandler(&errorHandler);

        // WHEN
        data.storageBehavior().setDeleteNextItemError(1, QStringLiteral("Error deleting"));
        const QModelIndex index = model->index(1, 0);
        pageModel.removeItem(index);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot remove task task2 from Inbox: Error deleting"));
    }

    void shouldPromoteItem()
    {
        // GIVEN

        AkonadiFakeData data;
        auto deps = data.createDependencies();
        Integration::initializeDefaultDomainDependencies(*deps.get());
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Two tasks
        data.createItem(GenTodo().withId(42).withParent(42).withUid("42").withTitle(QStringLiteral("task1")));
        data.createItem(GenTodo().withId(43).withParent(42).withUid("43").withTitle(QStringLiteral("task2")));
        QCOMPARE(data.items().count(), 2);

        auto serializer = deps->create<Akonadi::SerializerInterface>();

        QVERIFY(!serializer->isProjectItem(data.items().at(1)));

        Presentation::InboxPageModel pageModel(deps->create<Domain::TaskQueries>(),
                                                  deps->create<Domain::TaskRepository>());
        QAbstractItemModel *model = pageModel.centralListModel();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(model->rowCount(), 2);

        // WHEN
        const QModelIndex index = model->index(1, 0);
        pageModel.promoteItem(index);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(model->rowCount(), 1);
        QCOMPARE(data.items().count(), 2);
        QVERIFY(serializer->isProjectItem(data.items().at(1)));
    }

    void shouldGetAnErrorMessageWhenPromoteItemFailed()
    {
        // GIVEN
        AkonadiFakeData data;
        auto deps = data.createDependencies();
        Integration::initializeDefaultDomainDependencies(*deps.get());
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Two tasks
        data.createItem(GenTodo().withId(42).withParent(42).withUid("42").withTitle(QStringLiteral("task1")));
        data.createItem(GenTodo().withId(43).withParent(42).withUid("43").withTitle(QStringLiteral("task2")));
        QCOMPARE(data.items().count(), 2);

        Presentation::InboxPageModel pageModel(deps->create<Domain::TaskQueries>(),
                                               deps->create<Domain::TaskRepository>());
        QAbstractItemModel *model = pageModel.centralListModel();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(model->rowCount(), 2);

        FakeErrorHandler errorHandler;
        pageModel.setErrorHandler(&errorHandler);

        // WHEN
        data.storageBehavior().setUpdateNextItemError(44, QStringLiteral("Foo"));
        const QModelIndex index = model->index(1, 0);
        pageModel.promoteItem(index);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot promote task task2 to be a project: Foo"));
    }

    void shouldGetAnErrorMessageWhenUpdateTaskFailed()
    {
        // GIVEN
        AkonadiFakeData data;
        auto deps = data.createDependencies();
        Integration::initializeDefaultDomainDependencies(*deps.get());
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One task
        data.createItem(GenTodo().withId(42).withParent(42).withUid("42").withTitle(QStringLiteral("rootTask")));

        auto serializer = deps->create<Akonadi::SerializerInterface>();
        Presentation::InboxPageModel pageModel(deps->create<Domain::TaskQueries>(),
                                               deps->create<Domain::TaskRepository>());
        QAbstractItemModel *model = pageModel.centralListModel();
        TestHelpers::waitForEmptyJobQueue();

        FakeErrorHandler errorHandler;
        pageModel.setErrorHandler(&errorHandler);

        const QModelIndex index = model->index(0, 0);
        auto rootTask = model->data(index, Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();

        // WHEN
        data.storageBehavior().setUpdateNextItemError(1, QStringLiteral("Update error"));
        QVERIFY(model->setData(index, "newRootTask"));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot modify task rootTask in Inbox: Update error"));
        QCOMPARE(rootTask->title(), QStringLiteral("newRootTask")); // Note that the task *did* get updated
        QCOMPARE(index.data().toString(), QStringLiteral("newRootTask")); // and therefore the model keeps showing the new value
        QCOMPARE(serializer->createTaskFromItem(data.item(42))->title(), QStringLiteral("rootTask")); // but the underlying data wasn't updated
    }

    void shouldGetAnErrorMessageWhenAssociateTaskFailed()
    {
        // GIVEN
        AkonadiFakeData data;
        auto deps = data.createDependencies();
        Integration::initializeDefaultDomainDependencies(*deps.get());
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks
        data.createItem(GenTodo().withId(42).withParent(42).withUid("42").withTitle(QStringLiteral("task1")));
        data.createItem(GenTodo().withId(43).withParent(42).withUid("43").withTitle(QStringLiteral("task2")));
        data.createItem(GenTodo().withId(44).withParent(42).withUid("44").withTitle(QStringLiteral("task3")));

        Presentation::InboxPageModel pageModel(deps->create<Domain::TaskQueries>(),
                                               deps->create<Domain::TaskRepository>());
        QAbstractItemModel *model = pageModel.centralListModel();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(model->rowCount(), 3);

        FakeErrorHandler errorHandler;
        pageModel.setErrorHandler(&errorHandler);
        const QModelIndex rootTaskIndex = model->index(0, 0);

        auto task2 = model->data(model->index(1, 0), Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        auto task3 = model->data(model->index(2, 0), Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();

        // WHEN
        data.storageBehavior().setUpdateNextItemError(1, QStringLiteral("Update error"));

        auto mimeData = std::make_unique<QMimeData>();
        mimeData->setData(QStringLiteral("application/x-zanshin-object"), "object");
        mimeData->setProperty("objects", QVariant::fromValue(Domain::Task::List() << task2 << task3));
        QVERIFY(model->dropMimeData(mimeData.get(), Qt::MoveAction, -1, -1, rootTaskIndex));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot move task task2 as sub-task of task1: Update error"));
        QCOMPARE(model->rowCount(), 2); // One failed, one succeeded
    }

    void shouldDeparentWhenNoErrorHappens()
    {
        // GIVEN
        AkonadiFakeData data;
        auto deps = data.createDependencies();
        Integration::initializeDefaultDomainDependencies(*deps.get());
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One task, top level
        data.createItem(GenTodo().withId(42).withParent(42).withUid("42").withTitle(QStringLiteral("topLevelTask")));

        // Two tasks under the top level task
        data.createItem(GenTodo().withId(43).withParent(42).withUid("43").withParentUid("42").withTitle(QStringLiteral("childTask")).done());
        data.createItem(GenTodo().withId(44).withParent(42).withUid("44").withParentUid("42").withTitle(QStringLiteral("childTask2")));

        Presentation::InboxPageModel pageModel(deps->create<Domain::TaskQueries>(),
                                               deps->create<Domain::TaskRepository>());
        QAbstractItemModel *model = pageModel.centralListModel();
        TestHelpers::waitForEmptyJobQueue();

        const QModelIndex emptyPartModel = QModelIndex(); // model root, drop on the empty part is equivalent
        FakeErrorHandler errorHandler;
        pageModel.setErrorHandler(&errorHandler);

        const auto topLevelIndex = model->index(0, 0);
        QVERIFY(topLevelIndex.isValid());
        const auto childTask = model->data(model->index(0, 0, topLevelIndex), Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        const auto childTask2 = model->data(model->index(1, 0, topLevelIndex), Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();

        // WHEN
        data.storageBehavior().setUpdateNextItemError(1, QStringLiteral("Deparent error"));

        auto mimeData = std::make_unique<QMimeData>();
        mimeData->setData(QStringLiteral("application/x-zanshin-object"), "object");
        mimeData->setProperty("objects", QVariant::fromValue(Domain::Task::List() << childTask << childTask2)); // both will be DnD on the empty part
        QVERIFY(model->dropMimeData(mimeData.get(), Qt::MoveAction, -1, -1, emptyPartModel));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot deparent task childTask from its parent: Deparent error"));
        QCOMPARE(model->rowCount(), 2); // One failed, one succeeded
    }
};

ZANSHIN_TEST_MAIN(InboxPageModelTest)

#include "inboxpagemodeltest.moc"
