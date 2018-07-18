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

#ifndef PRESENTATION_RUNNINGTASKMODEL_H
#define PRESENTATION_RUNNINGTASKMODEL_H

#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "runningtaskmodelinterface.h"

namespace Presentation {

class RunningTaskModel : public RunningTaskModelInterface
{
    Q_OBJECT
public:
    typedef QSharedPointer<RunningTaskModel> Ptr;

    explicit RunningTaskModel(const Domain::TaskQueries::Ptr &taskQueries,
                              const Domain::TaskRepository::Ptr &taskRepository,
                              QObject *parent = nullptr);
    ~RunningTaskModel();

    Domain::Task::Ptr runningTask() const Q_DECL_OVERRIDE;
    void setRunningTask(const Domain::Task::Ptr &runningTask) Q_DECL_OVERRIDE;

    void taskDeleted(const Domain::Task::Ptr &task) Q_DECL_OVERRIDE;

public slots:
    void stopTask() Q_DECL_OVERRIDE;
    void doneTask() Q_DECL_OVERRIDE;

private:
    void taskTitleChanged(const QString &title);

    Domain::Task::Ptr m_runningTask;

    Domain::QueryResult<Domain::Task::Ptr>::Ptr m_taskList;
    Domain::TaskQueries::Ptr m_queries;
    Domain::TaskRepository::Ptr m_taskRepository;
};

}

#endif // PRESENTATION_RUNNINGTASKMODEL_H
