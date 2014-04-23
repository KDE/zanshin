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


#include "task.h"

using namespace Domain;

Task::Task(QObject *parent)
    : Artifact(parent),
      m_done(false)
{
}

Task::~Task()
{
}

bool Task::isDone() const
{
    return m_done;
}

void Task::setDone(bool done)
{
    if (m_done == done)
        return;

    m_done = done;
    emit doneChanged(done);
}

QDateTime Task::startDate() const
{
    return m_startDate;
}

void Task::setStartDate(const QDateTime &startDate)
{
    if (m_startDate == startDate)
        return;

    m_startDate = startDate;
    emit startDateChanged(startDate);
}

QDateTime Task::dueDate() const
{
    return m_dueDate;
}

void Task::setDueDate(const QDateTime &dueDate)
{
    if (m_dueDate == dueDate)
        return;

    m_dueDate = dueDate;
    emit dueDateChanged(dueDate);
}
