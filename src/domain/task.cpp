/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "task.h"

#include "utils/datetime.h"

using namespace Domain;

Task::Task(QObject *parent)
    : QObject(parent),
      m_running(false),
      m_done(false),
      m_recurrence(NoRecurrence)
{
}

Task::~Task()
{
}

QString Task::text() const
{
    return m_text;
}

QString Task::title() const
{
    return m_title;
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

void Task::setText(const QString &text)
{
    if (m_text == text)
        return;

    m_text = text;
    emit textChanged(text);
}

void Task::setTitle(const QString &title)
{
    if (m_title == title)
        return;

    m_title = title;
    emit titleChanged(title);
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

QDebug operator<<(QDebug dbg, const Domain::Task &task)
{
    dbg << "Task" << task.title();
    return dbg;
}

QDebug operator<<(QDebug dbg, const Domain::Task::Ptr &task)
{
    if (!task)
        dbg << "Ptr to null task";
    else
        dbg << "Ptr to" << *task;
    return dbg;
}

#include "moc_task.cpp"
