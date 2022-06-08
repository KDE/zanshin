/*
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef PRESENTATION_RUNNINGTASKMODELINTERFACE_H
#define PRESENTATION_RUNNINGTASKMODELINTERFACE_H

#include <QObject>

#include "domain/task.h"

#include "errorhandlingmodelbase.h"

namespace Presentation {

class RunningTaskModelInterface : public QObject, public ErrorHandlingModelBase
{
    Q_OBJECT
    Q_PROPERTY(Domain::Task::Ptr runningTask READ runningTask WRITE setRunningTask NOTIFY runningTaskChanged)
public:
    typedef QSharedPointer<RunningTaskModelInterface> Ptr;

    explicit RunningTaskModelInterface(QObject *parent = nullptr);
    ~RunningTaskModelInterface();

    virtual Domain::Task::Ptr runningTask() const = 0;
    virtual void setRunningTask(const Domain::Task::Ptr &runningTask) = 0;

    virtual void taskDeleted(const Domain::Task::Ptr &task) = 0;

signals:
    void runningTaskChanged(const Domain::Task::Ptr &task);

public Q_SLOTS:
    virtual void stopTask() = 0;
    virtual void doneTask() = 0;
};

}

#endif // PRESENTATION_RUNNINGTASKMODEL_H
