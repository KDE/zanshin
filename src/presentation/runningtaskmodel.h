/*
 * SPDX-FileCopyrightText: 2016-2017 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

    Domain::Task::Ptr runningTask() const override;
    void setRunningTask(const Domain::Task::Ptr &runningTask) override;

    void taskDeleted(const Domain::Task::Ptr &task) override;

public Q_SLOTS:
    void stopTask() override;
    void doneTask() override;

private:
    void taskTitleChanged(const QString &title);

    Domain::Task::Ptr m_runningTask;

    Domain::QueryResult<Domain::Task::Ptr>::Ptr m_taskList;
    Domain::TaskQueries::Ptr m_queries;
    Domain::TaskRepository::Ptr m_taskRepository;
};

}

#endif // PRESENTATION_RUNNINGTASKMODEL_H
