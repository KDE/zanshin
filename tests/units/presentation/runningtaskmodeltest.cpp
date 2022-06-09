/*
 * SPDX-FileCopyrightText: 2016-2017 David Faure <faure@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
        QCOMPARE(model.runningTask(), Domain::Task::Ptr());
        QVERIFY(!task->isRunning());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).value<Domain::Task::Ptr>(), Domain::Task::Ptr());
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
        QCOMPARE(model.runningTask(), Domain::Task::Ptr());
        QVERIFY(!task->isRunning());
        QVERIFY(task->isDone());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).value<Domain::Task::Ptr>(), Domain::Task::Ptr());
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

    void shouldHandleTaskDeletion()
    {
        // GIVEN
        TestDependencies deps;
        Presentation::RunningTaskModel model(deps.m_taskQueriesMockInstance, deps.m_taskRepositoryMockInstance);
        Domain::Task::Ptr task = Domain::Task::Ptr::create();
        model.setRunningTask(task);
        QSignalSpy spy(&model, &Presentation::RunningTaskModel::runningTaskChanged);

        // WHEN
        model.taskDeleted(task);

        // THEN
        QCOMPARE(model.runningTask(), Domain::Task::Ptr());
        QVERIFY(!task->isRunning());
        QVERIFY(!task->isDone());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).value<Domain::Task::Ptr>(), Domain::Task::Ptr());
    }

    void shouldIgnoreDeletionOfAnotherTask()
    {
        // GIVEN
        TestDependencies deps;
        Presentation::RunningTaskModel model(deps.m_taskQueriesMockInstance, deps.m_taskRepositoryMockInstance);
        Domain::Task::Ptr task = Domain::Task::Ptr::create();
        model.setRunningTask(task);
        QSignalSpy spy(&model, &Presentation::RunningTaskModel::runningTaskChanged);
        Domain::Task::Ptr task2 = Domain::Task::Ptr::create();

        // WHEN
        model.taskDeleted(task2);

        // THEN
        QCOMPARE(model.runningTask(), task);
        QVERIFY(task->isRunning());
        QCOMPARE(spy.count(), 0);
    }

    void shouldUpdateOnTaskRenaming()
    {
        // GIVEN
        TestDependencies deps;
        Presentation::RunningTaskModel model(deps.m_taskQueriesMockInstance, deps.m_taskRepositoryMockInstance);
        Domain::Task::Ptr task = Domain::Task::Ptr::create();
        task->setTitle(QStringLiteral("Old title"));
        model.setRunningTask(task);
        //Domain::Task::Ptr task2 = Domain::Task::Ptr::create();
        //model.setRunningTask(task2);
        QSignalSpy spy(&model, &Presentation::RunningTaskModel::runningTaskChanged);

        // WHEN
        task->setTitle(QStringLiteral("New title"));

        // THEN
        QCOMPARE(model.runningTask(), task);
        QVERIFY(task->isRunning());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).value<Domain::Task::Ptr>(), task);
    }

    void shouldIgnoreRenamingAnotherTask()
    {
        // GIVEN
        TestDependencies deps;
        Presentation::RunningTaskModel model(deps.m_taskQueriesMockInstance, deps.m_taskRepositoryMockInstance);
        Domain::Task::Ptr task = Domain::Task::Ptr::create();
        task->setTitle(QStringLiteral("Task 1 title"));
        model.setRunningTask(task);
        Domain::Task::Ptr task2 = Domain::Task::Ptr::create();
        task2->setTitle(QStringLiteral("Task 2 title"));
        QSignalSpy spy(&model, &Presentation::RunningTaskModel::runningTaskChanged);

        // WHEN
        task2->setTitle(QStringLiteral("New task 2 title"));

        // THEN
        QCOMPARE(model.runningTask(), task);
        QVERIFY(task->isRunning());
        QCOMPARE(spy.count(), 0);
    }



private:
};

ZANSHIN_TEST_MAIN(RunningTaskModelTest)

#include "runningtaskmodeltest.moc"
