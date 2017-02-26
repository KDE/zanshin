/* This file is part of Zanshin

   Copyright 2017 David Faure <faure@kde.org>

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

signals:
    void runningTaskChanged(const Domain::Task::Ptr &task);

public slots:
    virtual void stopTask() = 0;
    virtual void doneTask() = 0;
};

}

#endif // PRESENTATION_RUNNINGTASKMODEL_H
