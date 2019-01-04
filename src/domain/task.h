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

#include <QDate>
#include <QMetaType>
#include <QSharedPointer>
#include <QString>
#include <QUrl>

namespace Domain {

class Task : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(bool done READ isDone WRITE setDone NOTIFY doneChanged)
    Q_PROPERTY(QDate startDate READ startDate WRITE setStartDate NOTIFY startDateChanged)
    Q_PROPERTY(QDate dueDate READ dueDate WRITE setDueDate NOTIFY dueDateChanged)
    Q_PROPERTY(Domain::Task::Recurrence recurrence READ recurrence WRITE setRecurrence NOTIFY recurrenceChanged)
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

    explicit Task(QObject *parent = Q_NULLPTR);
    virtual ~Task();

    QString text() const;
    QString title() const;
    bool isRunning() const;
    bool isDone() const;
    QDate startDate() const;
    QDate dueDate() const;
    QDate doneDate() const;
    Recurrence recurrence() const;
    Attachments attachments() const;

public slots:
    void setText(const QString &text);
    void setTitle(const QString &title);
    void setRunning(bool running);
    void setDone(bool done);
    void setDoneDate(const QDate &doneDate);
    void setStartDate(const QDate &startDate);
    void setDueDate(const QDate &dueDate);
    void setRecurrence(Domain::Task::Recurrence recurrence);
    void setAttachments(const Domain::Task::Attachments &attachments);

signals:
    void textChanged(const QString &text);
    void titleChanged(const QString &title);
    void runningChanged(bool isRunning);
    void doneChanged(bool isDone);
    void doneDateChanged(const QDate &doneDate);
    void startDateChanged(const QDate &startDate);
    void dueDateChanged(const QDate &dueDate);
    void recurrenceChanged(Domain::Task::Recurrence recurrence);
    void attachmentsChanged(const Domain::Task::Attachments &attachments);

private:
    QString m_text;
    QString m_title;
    bool m_running;
    bool m_done;
    QDate m_startDate;
    QDate m_dueDate;
    QDate m_doneDate;
    Recurrence m_recurrence;
    Attachments m_attachments;
};

}

Q_DECLARE_METATYPE(Domain::Task::Ptr)
Q_DECLARE_METATYPE(Domain::Task::List)
Q_DECLARE_METATYPE(Domain::Task::Attachment)
Q_DECLARE_METATYPE(Domain::Task::Attachments)

#endif // DOMAIN_TASK_H
