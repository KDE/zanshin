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


#ifndef DOMAIN_TASK_H
#define DOMAIN_TASK_H

#include "artifact.h"
#include <QDateTime>
#include <QUrl>

namespace Domain {

class Task : public Artifact
{
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(bool done READ isDone WRITE setDone NOTIFY doneChanged)
    Q_PROPERTY(QDateTime startDate READ startDate WRITE setStartDate NOTIFY startDateChanged)
    Q_PROPERTY(QDateTime dueDate READ dueDate WRITE setDueDate NOTIFY dueDateChanged)
    Q_PROPERTY(Domain::Task::Recurrence recurrence READ recurrence WRITE setRecurrence NOTIFY recurrenceChanged)
    Q_PROPERTY(Domain::Task::Delegate delegate READ delegate WRITE setDelegate NOTIFY delegateChanged)
    Q_PROPERTY(Domain::Task::Attachments attachements READ attachments WRITE setAttachments NOTIFY attachmentsChanged)
public:
    typedef QSharedPointer<Task> Ptr;
    typedef QList<Task::Ptr> List;

    enum Recurrence {
        NoRecurrence = 0,
        RecursDaily,
        RecursWeekly,
        RecursMonthly // for now only monthly on the same day (say 11th day of the month)
    };
    Q_ENUM(Recurrence)

    class Attachment
    {
    public:
        Attachment();
        explicit Attachment(const QByteArray &data);
        explicit Attachment(const QUrl &uri);
        Attachment(const Attachment &other);
        ~Attachment();

        Attachment &operator=(const Attachment &other);
        bool operator==(const Attachment &other) const;

        bool isValid() const;
        bool isUri() const;

        QUrl uri() const;
        void setUri(const QUrl &uri);

        QByteArray data() const;
        void setData(const QByteArray &data);

        QString label() const;
        void setLabel(const QString &label);

        QString mimeType() const;
        void setMimeType(const QString &mimeType);

        QString iconName() const;
        void setIconName(const QString &iconName);

    private:
        QUrl m_uri;
        QByteArray m_data;
        QString m_label;
        QString m_mimeType;
        QString m_iconName;
    };

    typedef QList<Attachment> Attachments;

    class Delegate
    {
    public:
        Delegate();
        Delegate(const QString &name, const QString &email);
        Delegate(const Delegate &other);
        ~Delegate();

        Delegate &operator=(const Delegate &other);
        bool operator==(const Delegate &other) const;

        bool isValid() const;
        QString display() const;

        QString name() const;
        void setName(const QString &name);

        QString email() const;
        void setEmail(const QString &email);

    private:
        QString m_name;
        QString m_email;
    };

    explicit Task(QObject *parent = Q_NULLPTR);
    virtual ~Task();

    bool isRunning() const;
    bool isDone() const;
    QDateTime startDate() const;
    QDateTime dueDate() const;
    QDateTime doneDate() const;
    Recurrence recurrence() const;
    Attachments attachments() const;
    Delegate delegate() const;

public slots:
    void setRunning(bool running);
    void setDone(bool done);
    void setDoneDate(const QDateTime &doneDate);
    void setStartDate(const QDateTime &startDate);
    void setDueDate(const QDateTime &dueDate);
    void setRecurrence(Domain::Task::Recurrence recurrence);
    void setAttachments(const Domain::Task::Attachments &attachments);
    void setDelegate(const Domain::Task::Delegate &delegate);

signals:
    void runningChanged(bool isRunning);
    void doneChanged(bool isDone);
    void doneDateChanged(const QDateTime &doneDate);
    void startDateChanged(const QDateTime &startDate);
    void dueDateChanged(const QDateTime &dueDate);
    void recurrenceChanged(Domain::Task::Recurrence recurrence);
    void attachmentsChanged(const Domain::Task::Attachments &attachments);
    void delegateChanged(const Domain::Task::Delegate &delegate);

private:
    bool m_running;
    bool m_done;
    QDateTime m_startDate;
    QDateTime m_dueDate;
    QDateTime m_doneDate;
    Recurrence m_recurrence;
    Attachments m_attachments;
    Delegate m_delegate;
};

}

Q_DECLARE_METATYPE(Domain::Task::Ptr)
Q_DECLARE_METATYPE(Domain::Task::List)
Q_DECLARE_METATYPE(Domain::Task::Attachment)
Q_DECLARE_METATYPE(Domain::Task::Attachments)
Q_DECLARE_METATYPE(Domain::Task::Delegate)

#endif // DOMAIN_TASK_H
