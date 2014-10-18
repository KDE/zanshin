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

#include <mockitopp/mockitopp.hpp>

#include "domain/context.h"
#include "domain/task.h"
#include "domain/contextqueries.h"
#include "domain/taskqueries.h"

#include "domain/contextrepository.h"
#include "domain/taskrepository.h"
#include "domain/noterepository.h"

#include "presentation/contextpagemodel.h"

#include "testlib/fakejob.h"

using namespace mockitopp;

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

        auto childTaskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto childTaskResult = Domain::QueryResult<Domain::Task::Ptr>::create(childTaskProvider);
        childTaskProvider->append(childTask);

        mock_object<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findTopLevelTasks).when(context).thenReturn(taskResult);

        mock_object<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(parentTask).thenReturn(childTaskResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(childTask).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        mock_object<Domain::ContextRepository> contextRepositoryMock;
        mock_object<Domain::TaskRepository> taskRepositoryMock;
        mock_object<Domain::NoteRepository> noteRepositoryMock;

        Presentation::ContextPageModel page(context,
                                            &contextQueriesMock.getInstance(),
                                            &taskQueriesMock.getInstance(),
                                            &taskRepositoryMock.getInstance(),
                                            &noteRepositoryMock.getInstance());

        // WHEN
        QAbstractItemModel *model = page.centralListModel();

        // THEN
        const QModelIndex parentTaskIndex = model->index(0, 0);
        const QModelIndex childTaskIndex = model->index(0, 0, parentTaskIndex);

        QCOMPARE(page.context(), context);

        QCOMPARE(model->rowCount(), 1);
        QCOMPARE(model->rowCount(parentTaskIndex), 1);
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
    }
};

QTEST_MAIN(ContextPageModelTest)

#include "contextpagemodeltest.moc"
