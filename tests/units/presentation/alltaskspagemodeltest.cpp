/*
 * SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include <memory>

#include <QMimeData>

#include "utils/datetime.h"

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

class AllTasksPageModelTest : public QObject
{
    Q_OBJECT
private slots:
    void cleanup()
    {
        // The first call to QueryTreeModelBase::data triggers fetchTaskExtraData which creates jobs.
        // Wait for these to finish so they don't mess up subsequent tests
        TestHelpers::waitForEmptyJobQueue();
    }

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
        QCOMPARE(model->rowCount(), 3);

        const QModelIndex task1Index = model->index(0, 0);
        const QModelIndex task2Index = model->index(1, 0);
        const QModelIndex task3Index = model->index(2, 0);

        const QModelIndex childTask11Index = model->index(0, 0, task1Index);
        const QModelIndex childTask12Index = model->index(1, 0, task1Index);

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
        AkonadiFakeData data;
        auto deps = data.createDependencies();
        Integration::initializeDefaultDomainDependencies(*deps.get());
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        Presentation::AllTasksPageModel pageModel(deps->create<Domain::TaskQueries>(),
                                                  deps->create<Domain::TaskRepository>());
        QAbstractItemModel *model = pageModel.centralListModel();

        // WHEN
        auto title = QStringLiteral("New task");
        auto createdTask = pageModel.addItem(title);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QVERIFY(createdTask);
        QCOMPARE(createdTask->title(), title);
        QCOMPARE(createdTask->startDate(), QDate()); // that's one difference with the workday, which would set it to Utils::DateTime::currentDate()

        QCOMPARE(model->rowCount(), 1);
        auto taskInModel = model->data(model->index(0, 0), Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        QVERIFY(taskInModel);
        QCOMPARE(taskInModel->title(), createdTask->title());
        QCOMPARE(taskInModel->startDate(), createdTask->startDate());

        QCOMPARE(data.items().count(), 1);
    }

    void shouldAddChildTask()
    {
        // GIVEN
        AkonadiFakeData data;
        auto deps = data.createDependencies();
        Integration::initializeDefaultDomainDependencies(*deps.get());
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // One project
        data.createItem(GenTodo().withId(47).withParent(42).withUid("47").withTitle(QStringLiteral("KDE")).asProject());

        // One task
        data.createItem(GenTodo().withId(42).withParent(42).withUid("42").withTitle(QStringLiteral("task1")));
        QCOMPARE(data.items().count(), 2);

        Presentation::AllTasksPageModel pageModel(deps->create<Domain::TaskQueries>(),
                                                  deps->create<Domain::TaskRepository>());
        QAbstractItemModel *model = pageModel.centralListModel();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(model->rowCount(), 1);

        // WHEN
        const auto title = QStringLiteral("New task");
        const auto parentIndex = model->index(0, 0);
        const auto createdTask = pageModel.addItem(title, parentIndex);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QVERIFY(createdTask);
        QCOMPARE(createdTask->title(), title);
        QVERIFY(!createdTask->startDate().isValid());

        QCOMPARE(model->rowCount(parentIndex), 1);
        auto taskInModel = model->data(model->index(0, 0, parentIndex), Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
        QVERIFY(taskInModel);
        QCOMPARE(taskInModel->title(), createdTask->title());
        QCOMPARE(taskInModel->startDate(), createdTask->startDate());

        QCOMPARE(data.items().count(), 3);
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

        Presentation::AllTasksPageModel pageModel(deps->create<Domain::TaskQueries>(),
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

        Presentation::AllTasksPageModel pageModel(deps->create<Domain::TaskQueries>(),
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
};

ZANSHIN_TEST_MAIN(AllTasksPageModelTest)

#include "alltaskspagemodeltest.moc"
