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
      m_done(false),
      m_recurrence(NoRecurrence)
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

    const QDate doneDate = done ? Utils::DateTime::currentDate() : QDate();

    m_done = done;
    m_doneDate = doneDate;

    emit doneChanged(done);
    emit doneDateChanged(doneDate);
}

void Task::setDoneDate(const QDate &doneDate)
{
    if (m_doneDate == doneDate)
        return;

    m_doneDate = doneDate;
    emit doneDateChanged(doneDate);
}

QDate Task::startDate() const
{
    return m_startDate;
}

void Task::setStartDate(const QDate &startDate)
{
    if (m_startDate == startDate)
        return;

    m_startDate = startDate;
    emit startDateChanged(startDate);
}

QDate Task::dueDate() const
{
    return m_dueDate;
}

QDate Task::doneDate() const
{
    return m_doneDate;
}

Task::Recurrence Task::recurrence() const
{
    return m_recurrence;
}

Task::Attachments Task::attachments() const
{
    return m_attachments;
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

void Task::setDueDate(const QDate &dueDate)
{
    if (m_dueDate == dueDate)
        return;

    m_dueDate = dueDate;
    emit dueDateChanged(dueDate);
}

void Task::setRecurrence(Task::Recurrence recurrence)
{
    if (m_recurrence == recurrence)
        return;

    m_recurrence = recurrence;
    emit recurrenceChanged(recurrence);
}

void Task::setAttachments(const Task::Attachments &attachments)
{
    if (m_attachments == attachments)
        return;

    m_attachments = attachments;
    emit attachmentsChanged(attachments);
}

void Task::setDelegate(const Task::Delegate &delegate)
{
    if (m_delegate == delegate)
        return;

    m_delegate = delegate;
    emit delegateChanged(delegate);
}


Task::Attachment::Attachment()
{
}

Task::Attachment::Attachment(const QByteArray &data)
{
    setData(data);
}

Task::Attachment::Attachment(const QUrl &uri)
{
    setUri(uri);
}

Task::Attachment::Attachment(const Task::Attachment &other)
    : m_uri(other.m_uri),
      m_data(other.m_data),
      m_label(other.m_label),
      m_mimeType(other.m_mimeType),
      m_iconName(other.m_iconName)
{
}

Task::Attachment::~Attachment()
{
}

Task::Attachment &Task::Attachment::operator=(const Task::Attachment &other)
{
    Attachment copy(other);
    std::swap(m_uri, copy.m_uri);
    std::swap(m_data, copy.m_data);
    std::swap(m_label, copy.m_label);
    std::swap(m_mimeType, copy.m_mimeType);
    std::swap(m_iconName, copy.m_iconName);
    return *this;
}

bool Task::Attachment::operator==(const Task::Attachment &other) const
{
    return m_uri == other.m_uri
        && m_data == other.m_data
        && m_label == other.m_label
        && m_mimeType == other.m_mimeType
            && m_iconName == other.m_iconName;
}

bool Task::Attachment::isValid() const
{
    return m_uri.isValid() || !m_data.isEmpty();
}

bool Task::Attachment::isUri() const
{
    return m_uri.isValid();
}

QUrl Task::Attachment::uri() const
{
    return m_uri;
}

void Task::Attachment::setUri(const QUrl &uri)
{
    m_uri = uri;
    m_data.clear();
}

QByteArray Task::Attachment::data() const
{
    return m_data;
}

void Task::Attachment::setData(const QByteArray &data)
{
    m_data = data;
    m_uri.clear();
}

QString Task::Attachment::label() const
{
    return m_label;
}

void Task::Attachment::setLabel(const QString &label)
{
    m_label = label;
}

QString Task::Attachment::mimeType() const
{
    return m_mimeType;
}

void Task::Attachment::setMimeType(const QString &mimeType)
{
    m_mimeType = mimeType;
}

QString Task::Attachment::iconName() const
{
    return m_iconName;
}

void Task::Attachment::setIconName(const QString &iconName)
{
    m_iconName = iconName;
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

