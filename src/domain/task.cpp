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

#include "utils/datetime.h"

using namespace Domain;

Task::Task(QObject *parent)
    : Artifact(parent),
      m_running(false),
      m_done(false)
{
}

Task::~Task()
{
}

bool Task::isRunning() const
{
    return m_running;
}

bool Task::isDone() const
{
    return m_done;
}

void Task::setDone(bool done)
{
    if (m_done == done)
        return;

    const QDateTime doneDate = done ? Utils::DateTime::currentDateTime() : QDateTime();

    m_done = done;
    m_doneDate = doneDate;

    emit doneChanged(done);
    emit doneDateChanged(doneDate);
}

void Task::setDoneDate(const QDateTime &doneDate)
{
    if (m_doneDate == doneDate)
        return;

    m_doneDate = doneDate;
    emit doneDateChanged(doneDate);
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

QDateTime Task::doneDate() const
{
    return m_doneDate;
}

Task::Delegate Task::delegate() const
{
    return m_delegate;
}

void Task::setRunning(bool running)
{
    if (m_running == running)
        return;
    m_running = running;
    emit runningChanged(running);
}

void Task::setDueDate(const QDateTime &dueDate)
{
    if (m_dueDate == dueDate)
        return;

    m_dueDate = dueDate;
    emit dueDateChanged(dueDate);
}

void Task::setDelegate(const Task::Delegate &delegate)
{
    if (m_delegate == delegate)
        return;

    m_delegate = delegate;
    emit delegateChanged(delegate);
}


Task::Delegate::Delegate()
{
}

Task::Delegate::Delegate(const QString &name, const QString &email)
    : m_name(name), m_email(email)
{
}

Task::Delegate::Delegate(const Task::Delegate &other)
    : m_name(other.m_name), m_email(other.m_email)
{
}

Task::Delegate::~Delegate()
{
}

Task::Delegate &Task::Delegate::operator=(const Task::Delegate &other)
{
    Delegate copy(other);
    std::swap(m_name, copy.m_name);
    std::swap(m_email, copy.m_email);
    return *this;
}

bool Task::Delegate::operator==(const Task::Delegate &other) const
{
    return m_name == other.m_name
        && m_email == other.m_email;
}

bool Task::Delegate::isValid() const
{
    return !m_email.isEmpty();
}

QString Task::Delegate::display() const
{
    return !isValid() ? QString()
         : !m_name.isEmpty() ? m_name
         : m_email;
}

QString Task::Delegate::name() const
{
    return m_name;
}

void Task::Delegate::setName(const QString &name)
{
    m_name = name;
}

QString Task::Delegate::email() const
{
    return m_email;
}

void Task::Delegate::setEmail(const QString &email)
{
    m_email = email;
}

