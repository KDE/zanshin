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

#include "presentation/tasklistmodel.h"

Q_DECLARE_METATYPE(QModelIndex)

class TaskListModelTest : public QObject
{
    Q_OBJECT
public:
    explicit TaskListModelTest(QObject *parent = 0)
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

private slots:
    void shouldListTasks()
    {
        // GIVEN
        auto tasks = createTasks();
        auto provider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        for (auto task : tasks)
            provider->append(task);
        auto list = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(provider);

        // WHEN
        Presentation::TaskListModel model(list);

        // THEN
        QCOMPARE(model.rowCount(), tasks.size());
        QCOMPARE(model.rowCount(model.index(0)), 0);
        for (int i = 0; i < tasks.size(); i++) {
            auto task = tasks.at(i);
            auto index = model.index(i);

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
        auto provider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        provider->append(tasks.at(0));
        provider->append(tasks.at(1));
        auto list = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(provider);

        Presentation::TaskListModel model(list);
        QSignalSpy aboutToBeInsertedSpy(&model, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)));
        QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));

        // WHEN
        provider->insert(1, tasks.at(2));

        // THEN
        QCOMPARE(aboutToBeInsertedSpy.size(), 1);
        QCOMPARE(aboutToBeInsertedSpy.first().at(0).value<QModelIndex>(), QModelIndex());
        QCOMPARE(aboutToBeInsertedSpy.first().at(1).toInt(), 1);
        QCOMPARE(aboutToBeInsertedSpy.first().at(2).toInt(), 1);
        QCOMPARE(insertedSpy.size(), 1);
        QCOMPARE(insertedSpy.first().at(0).value<QModelIndex>(), QModelIndex());
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

        Presentation::TaskListModel model(list);
        QSignalSpy aboutToBeRemovedSpy(&model, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)));
        QSignalSpy removedSpy(&model, SIGNAL(rowsRemoved(QModelIndex, int, int)));

        // WHEN
        provider->removeAt(0);

        // THEN
        QCOMPARE(aboutToBeRemovedSpy.size(), 1);
        QCOMPARE(aboutToBeRemovedSpy.first().at(0).value<QModelIndex>(), QModelIndex());
        QCOMPARE(aboutToBeRemovedSpy.first().at(1).toInt(), 0);
        QCOMPARE(aboutToBeRemovedSpy.first().at(2).toInt(), 0);
        QCOMPARE(removedSpy.size(), 1);
        QCOMPARE(removedSpy.first().at(0).value<QModelIndex>(), QModelIndex());
        QCOMPARE(removedSpy.first().at(1).toInt(), 0);
        QCOMPARE(removedSpy.first().at(2).toInt(), 0);
    }

    void shouldReactToTaskChange()
    {
        // GIVEN
        auto tasks = createTasks();
        auto provider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        for (auto task : tasks)
            provider->append(task);
        auto list = Domain::QueryResultProvider<Domain::Task::Ptr>::createResult(provider);

        Presentation::TaskListModel model(list);
        QSignalSpy dataChangedSpy(&model, SIGNAL(dataChanged(QModelIndex, QModelIndex)));

        // WHEN
        tasks.at(2)->setDone(true);
        provider->replace(2, tasks.at(2));

        // THEN
        QCOMPARE(dataChangedSpy.size(), 1);
        QCOMPARE(dataChangedSpy.first().at(0).value<QModelIndex>(), model.index(2));
        QCOMPARE(dataChangedSpy.first().at(1).value<QModelIndex>(), model.index(2));
    }
};

QTEST_MAIN(TaskListModelTest)

#include "tasklistmodeltest.moc"
