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
    }
    m_runningTask = runningTask;
    if (m_runningTask) {
        m_runningTask->setRunning(true);
        KJob *job = m_taskRepository->update(m_runningTask);
        installHandler(job, i18n("Cannot update task %1 to 'running'", m_runningTask->title()));
    }
    emit runningTaskChanged(m_runningTask);
}

void RunningTaskModel::taskDeleted(const Domain::Task::Ptr &task)
{
    if (m_runningTask == task)
        setRunningTask({});
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
