/* This file is part of Zanshin

   Copyright 2014 Mario Bensi <mbensi@ipsquad.net>

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

#include "domain/taskqueries.h"
#include "domain/taskrepository.h"
#include "presentation/tasktreemodel.h"
#include "testlib/modeltest.h"

using namespace mockitopp;

Q_DECLARE_METATYPE(QModelIndex)

class TaskTreeModelTest : public QObject
{
    Q_OBJECT
public:
    explicit TaskTreeModelTest(QObject *parent = 0)
        : QObject(parent)
    {
        qRegisterMetaType<QModelIndex>();
    }

private:
    Domain::Task::List createTasks() const
    {
        Domain::Task::List result;

        const QStringList titles = {"first", "second", "third"};
        const QList<bool> doneStates = {true, false, false};
        Q_ASSERT(titles.size() == doneStates.size());

        for (int i = 0; i < titles.size(); i++) {
            auto task = Domain::Task::Ptr::create();
            task->setTitle(titles.at(i));
            task->setDone(doneStates.at(i));
            result << task;
        }

        return result;
    }

    Domain::Task::List createChildrenTasks() const
    {
        Domain::Task::List result;

        const QStringList titles = {"childFirst", "childSecond", "childThird"};
        const QList<bool> doneStates = {true, false, false};
        Q_ASSERT(titles.size() == doneStates.size());

        for (int i = 0; i < titles.size(); i++) {
            auto task = Domain::Task::Ptr::create();
            task->setTitle(titles.at(i));
            task->setDone(doneStates.at(i));
            result << task;
        }

        return result;
    }

private slots:
    void shouldListTasks()
    {
        // GIVEN
        auto tasks = createTasks();
        auto provider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        for (auto task : tasks)
            provider->append(task);
        auto list = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(provider);

        auto childrenTasks = createChildrenTasks();
        auto childrenProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        for (auto task : childrenTasks)
            childrenProvider->append(task);

        auto childrenList = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(childrenProvider);
        auto emptyProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto emptyList = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(emptyProvider);

        mock_object<Domain::TaskQueries> queryMock;
        queryMock(&Domain::TaskQueries::findChildren).when(tasks.at(0)).thenReturn(childrenList);
        queryMock(&Domain::TaskQueries::findChildren).when(tasks.at(1)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(tasks.at(2)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(childrenTasks.at(0)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(childrenTasks.at(1)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(childrenTasks.at(2)).thenReturn(emptyList);

        // WHEN
        Presentation::TaskTreeModel model(list, &queryMock.getInstance(), 0);
        new ModelTest(&model);

        // THEN
        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(model.rowCount(model.index(0, 0)), 3);
        QCOMPARE(model.rowCount(model.index(1, 0)), 0);
        QCOMPARE(model.rowCount(model.index(2, 0)), 0);
        QCOMPARE(model.rowCount(model.index(0, 0, model.index(0, 0))), 0);
        QCOMPARE(model.rowCount(model.index(1, 0, model.index(0, 0))), 0);
        QCOMPARE(model.rowCount(model.index(2, 0, model.index(0, 0))), 0);
        QCOMPARE(model.rowCount(model.index(3, 0, model.index(0, 0))), 3);

        for (int i = 0; i < tasks.size(); i++) {
            auto task = tasks.at(i);
            auto index = model.index(i, 0);

            QCOMPARE(model.data(index), model.data(index, Qt::DisplayRole));
            QCOMPARE(model.data(index).toString(), task->title());
            QCOMPARE(model.data(index, Qt::CheckStateRole).toInt() == Qt::Checked, task->isDone());
            QCOMPARE(model.data(index, Qt::CheckStateRole).toInt() == Qt::Unchecked, !task->isDone());
        }

        for (int i = 0; i < childrenTasks.size(); i++) {
            auto task = childrenTasks.at(i);
            auto index = model.index(i, 0, model.index(0, 0));

            QCOMPARE(model.data(index), model.data(index, Qt::DisplayRole));
            QCOMPARE(model.data(index).toString(), task->title());
            QCOMPARE(model.data(index, Qt::CheckStateRole).toInt() == Qt::Checked, task->isDone());
            QCOMPARE(model.data(index, Qt::CheckStateRole).toInt() == Qt::Unchecked, !task->isDone());
        }
    }

    void shouldReactToTaskAdd()
    {
        // GIVEN
        auto tasks = createTasks();
        auto  provider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        provider->append(tasks.at(1));
        provider->append(tasks.at(2));
        auto list = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(provider);

        auto childrenTasks = createChildrenTasks();
        auto childrenProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        for (auto task : childrenTasks)
            childrenProvider->append(task);

        auto childrenList = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(childrenProvider);
        auto emptyProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto emptyList = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(emptyProvider);

        mock_object<Domain::TaskQueries> queryMock;
        queryMock(&Domain::TaskQueries::findChildren).when(tasks.at(1)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(tasks.at(2)).thenReturn(emptyList);

        Presentation::TaskTreeModel model(list, &queryMock.getInstance(), 0);
        new ModelTest(&model);
        QSignalSpy aboutToBeInsertedSpy(&model, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)));
        QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));

        // WHEN
        queryMock(&Domain::TaskQueries::findChildren).when(tasks.at(0)).thenReturn(childrenList);
        queryMock(&Domain::TaskQueries::findChildren).when(childrenTasks.at(0)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(childrenTasks.at(1)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(childrenTasks.at(2)).thenReturn(emptyList);
        provider->insert(0, tasks.at(0));

        // THEN
        QCOMPARE(aboutToBeInsertedSpy.size(), 1);
        QCOMPARE(aboutToBeInsertedSpy.first().at(0).value<QModelIndex>(), QModelIndex());
        QCOMPARE(aboutToBeInsertedSpy.first().at(1).toInt(), 0);
        QCOMPARE(aboutToBeInsertedSpy.first().at(2).toInt(), 0);
        QCOMPARE(insertedSpy.size(), 1);
        QCOMPARE(insertedSpy.first().at(0).value<QModelIndex>(), QModelIndex());
        QCOMPARE(insertedSpy.first().at(1).toInt(), 0);
        QCOMPARE(insertedSpy.first().at(2).toInt(), 0);
    }

    void shouldReactToChilrenTaskAdd()
    {
        // GIVEN
        auto tasks = createTasks();
        auto provider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        for (auto task : tasks)
            provider->append(task);
        auto list = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(provider);

        auto childrenTasks = createChildrenTasks();
        auto childrenProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        childrenProvider->append(childrenTasks.at(0));
        childrenProvider->append(childrenTasks.at(1));

        auto childrenList = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(childrenProvider);
        auto emptyProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto emptyList = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(emptyProvider);

        mock_object<Domain::TaskQueries> queryMock;
        queryMock(&Domain::TaskQueries::findChildren).when(tasks.at(0)).thenReturn(childrenList);
        queryMock(&Domain::TaskQueries::findChildren).when(tasks.at(1)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(tasks.at(2)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(childrenTasks.at(0)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(childrenTasks.at(1)).thenReturn(emptyList);

        Presentation::TaskTreeModel model(list, &queryMock.getInstance(), 0);
        new ModelTest(&model);
        QSignalSpy aboutToBeInsertedSpy(&model, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)));
        QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));

        // WHEN
        queryMock(&Domain::TaskQueries::findChildren).when(childrenTasks.at(2)).thenReturn(emptyList);
        childrenProvider->insert(1, tasks.at(2));

        // THEN
        QCOMPARE(aboutToBeInsertedSpy.size(), 1);
        QCOMPARE(aboutToBeInsertedSpy.first().at(0).value<QModelIndex>(), model.index(0, 0));
        QCOMPARE(aboutToBeInsertedSpy.first().at(1).toInt(), 1);
        QCOMPARE(aboutToBeInsertedSpy.first().at(2).toInt(), 1);
        QCOMPARE(insertedSpy.size(), 1);
        QCOMPARE(insertedSpy.first().at(0).value<QModelIndex>(), model.index(0, 0));
        QCOMPARE(insertedSpy.first().at(1).toInt(), 1);
        QCOMPARE(insertedSpy.first().at(2).toInt(), 1);
    }

    void shouldReactToTaskRemove()
    {
        // GIVEN
        auto tasks = createTasks();
        auto provider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        for (auto task : tasks)
            provider->append(task);
        auto list = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(provider);

        auto childrenTasks = createChildrenTasks();
        auto childrenProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        for (auto task : childrenTasks)
            childrenProvider->append(task);

        auto childrenList = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(childrenProvider);
        auto emptyProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto emptyList = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(emptyProvider);

        mock_object<Domain::TaskQueries> queryMock;
        queryMock(&Domain::TaskQueries::findChildren).when(tasks.at(0)).thenReturn(childrenList);
        queryMock(&Domain::TaskQueries::findChildren).when(tasks.at(1)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(tasks.at(2)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(childrenTasks.at(0)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(childrenTasks.at(1)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(childrenTasks.at(2)).thenReturn(emptyList);

        Presentation::TaskTreeModel model(list, &queryMock.getInstance(), 0);
        new ModelTest(&model);
        QSignalSpy aboutToBeRemovedSpy(&model, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)));
        QSignalSpy removedSpy(&model, SIGNAL(rowsRemoved(QModelIndex, int, int)));
        QSignalSpy aboutToBeInsertedSpy(&model, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)));
        QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));

        QModelIndex removeIndex = model.index(0, 0);

        // WHEN
        // Remove children
        childrenProvider->removeAt(0);
        childrenProvider->removeAt(0);
        childrenProvider->removeAt(0);
        // Move children to Top Level
        provider->append(childrenTasks.at(0));
        provider->append(childrenTasks.at(1));
        provider->append(childrenTasks.at(2));
        // Remove firt element from topLevel
        provider->removeAt(0);

        // THEN
        QCOMPARE(aboutToBeRemovedSpy.size(), 4);
        QCOMPARE(removedSpy.size(), 4);
        for (int i = 0; i < aboutToBeRemovedSpy.size(); i++) {
            if (i != 3)
                QCOMPARE(aboutToBeRemovedSpy.at(i).at(0).value<QModelIndex>(), removeIndex);
            else
                QCOMPARE(aboutToBeRemovedSpy.at(i).at(0).value<QModelIndex>(), QModelIndex());
            QCOMPARE(aboutToBeRemovedSpy.at(i).at(1).toInt(), 0);
            QCOMPARE(aboutToBeRemovedSpy.at(i).at(2).toInt(), 0);

            if (i != 3)
                QCOMPARE(removedSpy.at(i).at(0).value<QModelIndex>(), removeIndex);
            else
                QCOMPARE(removedSpy.at(i).at(0).value<QModelIndex>(), QModelIndex());
            QCOMPARE(removedSpy.at(i).at(1).toInt(), 0);
            QCOMPARE(removedSpy.at(i).at(2).toInt(), 0);
        }

        QCOMPARE(aboutToBeInsertedSpy.size(), 3);
        QCOMPARE(insertedSpy.size(), 3);
        for (int i = 0; i < aboutToBeInsertedSpy.size(); i++) {
            QCOMPARE(aboutToBeInsertedSpy.at(i).at(0).value<QModelIndex>(), QModelIndex());
            QCOMPARE(aboutToBeInsertedSpy.at(i).at(1).toInt(), i + 3);
            QCOMPARE(aboutToBeInsertedSpy.at(i).at(2).toInt(), i + 3);
            QCOMPARE(insertedSpy.at(i).at(0).value<QModelIndex>(), QModelIndex());
            QCOMPARE(insertedSpy.at(i).at(1).toInt(), i + 3);
            QCOMPARE(insertedSpy.at(i).at(1).toInt(), i + 3);
        }
    }

    void shouldReactToTaskChange()
    {
        // GIVEN
        // GIVEN
        auto tasks = createTasks();
        auto provider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        for (auto task : tasks)
            provider->append(task);
        auto list = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(provider);

        auto childrenTasks = createChildrenTasks();
        auto childrenProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        for (auto task : childrenTasks)
            childrenProvider->append(task);

        auto childrenList = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(childrenProvider);
        auto emptyProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto emptyList = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(emptyProvider);

        mock_object<Domain::TaskQueries> queryMock;
        queryMock(&Domain::TaskQueries::findChildren).when(tasks.at(0)).thenReturn(childrenList);
        queryMock(&Domain::TaskQueries::findChildren).when(tasks.at(1)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(tasks.at(2)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(childrenTasks.at(0)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(childrenTasks.at(1)).thenReturn(emptyList);
        queryMock(&Domain::TaskQueries::findChildren).when(childrenTasks.at(2)).thenReturn(emptyList);

        // WHEN
        Presentation::TaskTreeModel model(list, &queryMock.getInstance(), 0);
        new ModelTest(&model);
        QSignalSpy dataChangedSpy(&model, SIGNAL(dataChanged(QModelIndex, QModelIndex)));

        // WHEN
        tasks.at(2)->setDone(true);
        childrenTasks.at(2)->setDone(true);
        provider->replace(2, tasks.at(2));
        childrenProvider->replace(2, tasks.at(2));

        // THEN
        QCOMPARE(dataChangedSpy.size(), 2);
        QCOMPARE(dataChangedSpy.first().at(0).value<QModelIndex>(), model.index(2, 0));
        QCOMPARE(dataChangedSpy.first().at(1).value<QModelIndex>(), model.index(2, 0));
        QCOMPARE(dataChangedSpy.last().at(0).value<QModelIndex>(), model.index(2, 0, model.index(0, 0)));
        QCOMPARE(dataChangedSpy.last().at(1).value<QModelIndex>(), model.index(2, 0, model.index(0, 0)));
    }
};

QTEST_MAIN(TaskTreeModelTest)

#include "tasktreemodeltest.moc"
