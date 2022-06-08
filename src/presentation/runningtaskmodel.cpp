/*
 * SPDX-FileCopyrightText: 2016-2017 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "runningtaskmodel.h"

#include <KLocalizedString>

using namespace Presentation;

RunningTaskModel::RunningTaskModel(const Domain::TaskQueries::Ptr &taskQueries,
                                   const Domain::TaskRepository::Ptr &taskRepository,
                                   QObject *parent)
    : RunningTaskModelInterface(parent),
      m_queries(taskQueries),
      m_taskRepository(taskRepository)
{
    // List all tasks to find if any one is already set to "running"
    if (m_queries) {
        m_taskList = m_queries->findAll();
        Q_ASSERT(m_taskList);
        m_taskList->addPostInsertHandler([this](const Domain::Task::Ptr &task, int) {
            if (task->isRunning()) {
                setRunningTask(task);
            }
        });
        // if there was a addFinishedHandler, we could reset m_queries and m_taskList to nullptr there to free memory.
    }
}

RunningTaskModel::~RunningTaskModel()
{
}

Domain::Task::Ptr RunningTaskModel::runningTask() const
{
    return m_runningTask;
}

void RunningTaskModel::setRunningTask(const Domain::Task::Ptr &runningTask)
{
    if (m_runningTask) {
        m_runningTask->setRunning(false);
        KJob *job = m_taskRepository->update(m_runningTask);
        installHandler(job, i18n("Cannot update task %1 to 'not running'", m_runningTask->title()));
        disconnect(runningTask.data(), &Domain::Task::titleChanged,
                this, &RunningTaskModel::taskTitleChanged);
    }
    m_runningTask = runningTask;
    if (m_runningTask) {
        m_runningTask->setRunning(true);
        KJob *job = m_taskRepository->update(m_runningTask);
        installHandler(job, i18n("Cannot update task %1 to 'running'", m_runningTask->title()));
        connect(runningTask.data(), &Domain::Task::titleChanged,
                this, &RunningTaskModel::taskTitleChanged);
    }
    Q_EMIT runningTaskChanged(m_runningTask);
}

void RunningTaskModel::taskDeleted(const Domain::Task::Ptr &task)
{
    if (m_runningTask == task)
        setRunningTask({});
}

void RunningTaskModel::taskTitleChanged(const QString &title)
{
    Q_UNUSED(title)
    Q_EMIT runningTaskChanged(m_runningTask);
}

void RunningTaskModel::stopTask()
{
    setRunningTask(Domain::Task::Ptr());
}

void RunningTaskModel::doneTask()
{
    m_runningTask->setDone(true);
    stopTask();
}
