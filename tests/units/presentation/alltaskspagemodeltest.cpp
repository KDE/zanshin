/* This file is part of Zanshin

   Copyright 2019 David Faure <faure@kde.org>

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

#include "utils/datetime.h"
#include "utils/mockobject.h"

#include "domain/taskqueries.h"
#include "domain/taskrepository.h"
#include "presentation/alltaskspagemodel.h"
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

class AllTasksPageModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldListAllTasksInCentralListModel()
    {
        // GIVEN
        const auto today = Utils::DateTime::currentDate();

        AkonadiFakeData data;
        auto deps = data.createDependencies();
        Integration::initializeDefaultDomainDependencies(*deps.get());

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project
        data.createItem(GenTodo().withId(47).withParent(42).withUid("47").withTitle(QStringLiteral("KDE")).asProject());

        // Three tasks
        data.createItem(GenTodo().withId(42).withParent(42).withUid("42").withTitle(QStringLiteral("task1")).withStartDate(today.addDays(-10)).withDueDate(today));
        data.createItem(GenTodo().withId(43).withParent(42).withUid("43").withParentUid("47").withTitle(QStringLiteral("task2")).withStartDate(today).withDueDate(today.addDays(10)));
        data.createItem(GenTodo().withId(44).withParent(42).withUid("44").withTitle(QStringLiteral("task3")).withStartDate(today.addYears(-4)).withDueDate(today.addYears(-3)));

        // Two tasks under the task1
        data.createItem(GenTodo().withId(45).withParent(42).withUid("45").withParentUid("42").withTitle(QStringLiteral("childTask11")));
        data.createItem(GenTodo().withId(46).withParent(42).withUid("46").withParentUid("42").withTitle(QStringLiteral("childTask12")).withStartDate(today).withDueDate(today));

        auto serializer = deps->create<Akonadi::SerializerInterface>();
        Presentation::AllTasksPageModel pageModel(deps->create<Domain::TaskQueries>(),
                                                  deps->create<Domain::TaskRepository>());

        // WHEN
        QAbstractItemModel *model = pageModel.centralListModel();
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        const QModelIndex task1Index = model->index(0, 0);
        const QModelIndex task2Index = model->index(1, 0);
        const QModelIndex task3Index = model->index(2, 0);

        const QModelIndex childTask11Index = model->index(0, 0, task1Index);
        const QModelIndex childTask12Index = model->index(1, 0, task1Index);

        QCOMPARE(model->rowCount(), 3);
        QCOMPARE(model->rowCount(task1Index), 2);
        QCOMPARE(model->rowCount(task2Index), 0);
        QCOMPARE(model->rowCount(task3Index), 0);

        QVERIFY(childTask11Index.isValid());
        QVERIFY(childTask12Index.isValid());
        QCOMPARE(model->rowCount(childTask11Index), 0);
        QCOMPARE(model->rowCount(childTask12Index), 0);

        auto task1 = model->data(task1Index, Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        auto task2 = model->data(task2Index, Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        auto task3 = model->data(task3Index, Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        auto childTask11 = model->data(childTask11Index, Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        auto childTask12 = model->data(childTask12Index, Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();

        TestHelpers::waitForEmptyJobQueue();

        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled
                                         | Qt::ItemIsEditable
                                         | Qt::ItemIsDragEnabled;
        QCOMPARE(model->flags(task1Index), defaultFlags | Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(childTask11Index), defaultFlags | Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(childTask12Index), defaultFlags | Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(task2Index), defaultFlags | Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(task3Index), defaultFlags | Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled);

        QCOMPARE(model->data(task1Index).toString(), task1->title());
        QCOMPARE(model->data(childTask11Index).toString(), childTask11->title());
        QCOMPARE(model->data(childTask12Index).toString(), childTask12->title());
        QCOMPARE(model->data(task2Index).toString(), task2->title());
        QCOMPARE(model->data(task3Index).toString(), task3->title());

        QCOMPARE(model->data(task1Index, Qt::EditRole).toString(), task1->title());
        QCOMPARE(model->data(childTask11Index, Qt::EditRole).toString(), childTask11->title());
        QCOMPARE(model->data(childTask12Index, Qt::EditRole).toString(), childTask12->title());
        QCOMPARE(model->data(task2Index, Qt::EditRole).toString(), task2->title());
        QCOMPARE(model->data(task3Index, Qt::EditRole).toString(), task3->title());

        QVERIFY(model->data(task1Index, Qt::CheckStateRole).isValid());
        QVERIFY(model->data(childTask11Index, Qt::CheckStateRole).isValid());
        QVERIFY(model->data(childTask12Index, Qt::CheckStateRole).isValid());
        QVERIFY(model->data(task2Index, Qt::CheckStateRole).isValid());
        QVERIFY(model->data(task3Index, Qt::CheckStateRole).isValid());

        QCOMPARE(model->data(task1Index, Qt::CheckStateRole).toBool(), false);
        QCOMPARE(model->data(childTask11Index, Qt::CheckStateRole).toBool(), false);
        QCOMPARE(model->data(childTask12Index, Qt::CheckStateRole).toBool(), false);
        QCOMPARE(model->data(task2Index, Qt::CheckStateRole).toBool(), false);
        QCOMPARE(model->data(task3Index, Qt::CheckStateRole).toBool(), false);

        QVERIFY(model->data(task1Index, Presentation::QueryTreeModelBase::ProjectRole).isValid());
        QVERIFY(model->data(task2Index, Presentation::QueryTreeModelBase::ProjectRole).isValid());
        QCOMPARE(model->data(task1Index, Presentation::QueryTreeModelBase::ProjectRole).toString(), QString());
        QCOMPARE(model->data(task2Index, Presentation::QueryTreeModelBase::ProjectRole).toString(), QString("KDE"));
        QCOMPARE(model->data(task1Index, Presentation::QueryTreeModelBase::IsChildRole).toBool(), false);
        QCOMPARE(model->data(task2Index, Presentation::QueryTreeModelBase::IsChildRole).toBool(), false);
        QVERIFY(!model->data(childTask11Index, Presentation::QueryTreeModelBase::ProjectRole).isValid());
        QVERIFY(!model->data(childTask12Index, Presentation::QueryTreeModelBase::ProjectRole).isValid());
        QCOMPARE(model->data(childTask11Index, Presentation::QueryTreeModelBase::IsChildRole).toBool(), true);
        QCOMPARE(model->data(childTask12Index, Presentation::QueryTreeModelBase::IsChildRole).toBool(), true);

        // WHEN
        QVERIFY(model->setData(task1Index, "newTask1"));
        QVERIFY(model->setData(childTask11Index, "newChildTask11"));
        QVERIFY(model->setData(task2Index, "newTask2"));
        QVERIFY(model->setData(task3Index, "newTask3"));
        QVERIFY(model->setData(childTask12Index, "newChildTask12"));

        QVERIFY(model->setData(task1Index, Qt::Unchecked, Qt::CheckStateRole));
        QVERIFY(model->setData(childTask11Index, Qt::Unchecked, Qt::CheckStateRole));
        QVERIFY(model->setData(task2Index, Qt::Checked, Qt::CheckStateRole));
        QVERIFY(model->setData(task3Index, Qt::Unchecked, Qt::CheckStateRole));
        QVERIFY(model->setData(childTask12Index, Qt::Checked, Qt::CheckStateRole));

        // THEN
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
        auto mimeData = std::unique_ptr<QMimeData>(model->mimeData(QModelIndexList() << childTask12Index));

        // THEN
        QVERIFY(mimeData->hasFormat(QStringLiteral("application/x-zanshin-object")));
        QCOMPARE(mimeData->property("objects").value<Domain::Task::List>(),
                 Domain::Task::List() << childTask12);

        // WHEN
        data.createItem(GenTodo().withId(48).withParent(42).withUid("48").withTitle(QStringLiteral("childTask2")));
        auto childTask2 = model->data(model->index(3, 0), Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        mimeData.reset(new QMimeData);
        mimeData->setData(QStringLiteral("application/x-zanshin-object"), "object");
        mimeData->setProperty("objects", QVariant::fromValue(Domain::Task::List() << childTask2));
        model->dropMimeData(mimeData.get(), Qt::MoveAction, -1, -1, childTask11Index);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(serializer->relatedUidFromItem(data.item(48)), QStringLiteral("45"));
        QCOMPARE(model->rowCount(), 3);
        QCOMPARE(model->rowCount(childTask11Index), 1);

        // WHEN
        data.createItem(GenTodo().withId(49).withParent(42).withUid("49").withTitle(QStringLiteral("childTask3")));
        auto childTask3 = model->data(model->index(3, 0), Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        data.createItem(GenTodo().withId(50).withParent(42).withUid("50").withTitle(QStringLiteral("childTask4")));
        auto childTask4 = model->data(model->index(4, 0), Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        mimeData.reset(new QMimeData);
        mimeData->setData(QStringLiteral("application/x-zanshin-object"), "object");
        mimeData->setProperty("objects", QVariant::fromValue(Domain::Task::List() << childTask3 << childTask4));
        model->dropMimeData(mimeData.get(), Qt::MoveAction, -1, -1, childTask12Index);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(serializer->relatedUidFromItem(data.item(49)), QStringLiteral("46"));
        QCOMPARE(serializer->relatedUidFromItem(data.item(50)), QStringLiteral("46"));
        QCOMPARE(model->rowCount(), 3);
        QCOMPARE(model->rowCount(childTask12Index), 2);

        // WHEN
        QVERIFY(!childTask4->startDate().isValid());
        mimeData.reset(new QMimeData);
        mimeData->setData(QStringLiteral("application/x-zanshin-object"), "object");
        mimeData->setProperty("objects", QVariant::fromValue(Domain::Task::List() << childTask4));
        model->dropMimeData(mimeData.get(), Qt::MoveAction, -1, -1, QModelIndex());
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(serializer->relatedUidFromItem(data.item(50)), QString());
        QCOMPARE(model->rowCount(), 4);
        QCOMPARE(model->rowCount(childTask12Index), 1);
        QCOMPARE(childTask4->startDate(), QDate()); // unlike the workday
    }

    void shouldAddTasksInAllTasksPage()
    {
        // GIVEN

        // ... in fact we won't list any model
        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;

        // We'll gladly create a task though
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::create).when(any<Domain::Task::Ptr>()).thenReturn(new FakeJob(this));

        Presentation::AllTasksPageModel pageModel(taskQueriesMock.getInstance(),
                                                  taskRepositoryMock.getInstance());

        // WHEN
        auto title = QStringLiteral("New task");
        auto task = pageModel.addItem(title);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::create).when(any<Domain::Task::Ptr>()).exactly(1));

        QVERIFY(task);
        QCOMPARE(task->title(), title);
        QCOMPARE(task->startDate(), QDate()); // that's one difference with the workday, which would set it to Utils::DateTime::currentDate()
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
        taskQueriesMock(&Domain::TaskQueries::findTopLevel).when().thenReturn(taskResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findProject).when(any<Domain::Task::Ptr>()).thenReturn(Domain::QueryResult<Domain::Project::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::createChild).when(any<Domain::Task::Ptr>(),
                                                                      any<Domain::Task::Ptr>())
                                                                .thenReturn(new FakeJob(this));

        Presentation::AllTasksPageModel pageModel(taskQueriesMock.getInstance(),
                                                  taskRepositoryMock.getInstance());

        // WHEN
        const auto title = QStringLiteral("New task");
        const auto parentIndex = pageModel.centralListModel()->index(0, 0);
        const auto createdTask = pageModel.addItem(title, parentIndex);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::createChild).when(any<Domain::Task::Ptr>(),
                                                                              any<Domain::Task::Ptr>())
                                                                        .exactly(1));

        QVERIFY(createdTask);
        QCOMPARE(createdTask->title(), title);
        QVERIFY(!createdTask->startDate().isValid());
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
        taskQueriesMock(&Domain::TaskQueries::findTopLevel).when().thenReturn(taskResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findProject).when(any<Domain::Task::Ptr>()).thenReturn(Domain::QueryResult<Domain::Project::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::remove).when(task2).thenReturn(new FakeJob(this));

        Presentation::AllTasksPageModel pageModel(taskQueriesMock.getInstance(),
                                               taskRepositoryMock.getInstance());

        // WHEN
        const QModelIndex index = pageModel.centralListModel()->index(1, 0);
        pageModel.removeItem(index);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::remove).when(task2).exactly(1));
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
        taskQueriesMock(&Domain::TaskQueries::findTopLevel).when().thenReturn(taskResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findDataSource).when(any<Domain::Task::Ptr>()).thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findProject).when(task2).thenReturn(Domain::QueryResult<Domain::Project::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::promoteToProject).when(task2).thenReturn(new FakeJob(this));

        Presentation::AllTasksPageModel pageModel(taskQueriesMock.getInstance(),
                                                  taskRepositoryMock.getInstance());

        // WHEN
        const QModelIndex index = pageModel.centralListModel()->index(1, 0);
        pageModel.promoteItem(index);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::promoteToProject).when(task2).exactly(1));
    }
};

ZANSHIN_TEST_MAIN(AllTasksPageModelTest)

#include "alltaskspagemodeltest.moc"
