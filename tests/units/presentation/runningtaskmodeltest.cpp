/* This file is part of Zanshin

   Copyright 2016-2017 David Faure <faure@kde.org>

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

#include <testlib/qtest_gui_zanshin.h>

#include "utils/mockobject.h"
#include "utils/jobhandler.h"

#include "presentation/runningtaskmodel.h"

#include "testhelpers.h"

using namespace mockitopp;
using namespace mockitopp::matcher;

class TestDependencies
{
public:
    TestDependencies()
    {
        m_taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(m_taskProvider);

        m_taskQueriesMock(&Domain::TaskQueries::findAll).when().thenReturn(taskResult);
        m_taskQueriesMockInstance = m_taskQueriesMock.getInstance();

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::update).when(any<Domain::Task::Ptr>())
                                                                .thenReturn(0);
        m_taskRepositoryMockInstance = taskRepositoryMock.getInstance();
    }
    Utils::MockObject<Domain::TaskQueries> m_taskQueriesMock;

    Domain::TaskQueries::Ptr m_taskQueriesMockInstance;
    Domain::TaskRepository::Ptr m_taskRepositoryMockInstance;
    Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr m_taskProvider;
};

class RunningTaskModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldDoInitialListing()
    {
        // GIVEN
        TestDependencies deps;
        // Three tasks, one being marked as running
        auto firstTask = Domain::Task::Ptr::create();
        firstTask->setTitle(QStringLiteral("rootTask"));
        auto initialTask = Domain::Task::Ptr::create();
        initialTask->setTitle(QStringLiteral("initialTask"));
        initialTask->setRunning(true);
        auto otherTask = Domain::Task::Ptr::create();
        otherTask->setTitle(QStringLiteral("otherTask"));

        // WHEN
        Presentation::RunningTaskModel model(deps.m_taskQueriesMockInstance, deps.m_taskRepositoryMockInstance);
        QVERIFY(!model.runningTask());
        deps.m_taskProvider->append(firstTask);
        deps.m_taskProvider->append(initialTask);
        deps.m_taskProvider->append(otherTask);

        // THEN
        QCOMPARE(model.runningTask(), initialTask);
    }

    void shouldStartTask()
    {
        // GIVEN
        TestDependencies deps;
        Presentation::RunningTaskModel model(deps.m_taskQueriesMockInstance, deps.m_taskRepositoryMockInstance);
        Domain::Task::Ptr task = Domain::Task::Ptr::create();
        QSignalSpy spy(&model, &Presentation::RunningTaskModel::runningTaskChanged);

        // WHEN
        model.setRunningTask(task);

        // THEN
        QCOMPARE(model.runningTask(), task);
        QVERIFY(task->isRunning());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).value<Domain::Task::Ptr>(), task);
    }

    void shouldHandleStopTask()
    {
        // GIVEN
        TestDependencies deps;
        Presentation::RunningTaskModel model(deps.m_taskQueriesMockInstance, deps.m_taskRepositoryMockInstance);
        Domain::Task::Ptr task = Domain::Task::Ptr::create();
        model.setRunningTask(task);
        QSignalSpy spy(&model, &Presentation::RunningTaskModel::runningTaskChanged);

        // WHEN
        model.stopTask();

        // THEN
        QCOMPARE(model.runningTask(), Domain::Task::Ptr(nullptr));
        QVERIFY(!task->isRunning());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).value<Domain::Task::Ptr>(), Domain::Task::Ptr(nullptr));
    }

    void shouldHandleDoneTask()
    {
        // GIVEN
        TestDependencies deps;
        Presentation::RunningTaskModel model(deps.m_taskQueriesMockInstance, deps.m_taskRepositoryMockInstance);
        Domain::Task::Ptr task = Domain::Task::Ptr::create();
        model.setRunningTask(task);
        QSignalSpy spy(&model, &Presentation::RunningTaskModel::runningTaskChanged);

        // WHEN
        model.doneTask();

        // THEN
        QCOMPARE(model.runningTask(), Domain::Task::Ptr(nullptr));
        QVERIFY(!task->isRunning());
        QVERIFY(task->isDone());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).value<Domain::Task::Ptr>(), Domain::Task::Ptr(nullptr));
    }

    void shouldHandleSwitchingToAnotherTask()
    {
        // GIVEN
        TestDependencies deps;
        Presentation::RunningTaskModel model(deps.m_taskQueriesMockInstance, deps.m_taskRepositoryMockInstance);
        Domain::Task::Ptr task = Domain::Task::Ptr::create();
        model.setRunningTask(task);
        Domain::Task::Ptr task2 = Domain::Task::Ptr::create();
        QSignalSpy spy(&model, &Presentation::RunningTaskModel::runningTaskChanged);

        // WHEN
        model.setRunningTask(task2);

        // THEN
        QCOMPARE(model.runningTask(), task2);
        QVERIFY(!task->isRunning());
        QVERIFY(task2->isRunning());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).value<Domain::Task::Ptr>(), task2);
    }

private:
};

ZANSHIN_TEST_MAIN(RunningTaskModelTest)

#include "runningtaskmodeltest.moc"
