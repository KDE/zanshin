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

namespace Domain {

class Task : public Artifact
{
    Q_OBJECT
    Q_PROPERTY(bool done READ isDone WRITE setDone NOTIFY doneChanged)
    Q_PROPERTY(QDateTime startDate READ startDate WRITE setStartDate NOTIFY startDateChanged)
    Q_PROPERTY(QDateTime dueDate READ dueDate WRITE setDueDate NOTIFY dueDateChanged)
    Q_PROPERTY(Domain::Task::Delegate delegate READ delegate WRITE setDelegate NOTIFY delegateChanged)
public:
    typedef QSharedPointer<Task> Ptr;
    typedef QList<Task::Ptr> List;

    class Delegate
    {
    public:
        Delegate();
        Delegate(const QString &name, const QString &email);
        Delegate(const Delegate &other);

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

    bool isDone() const;
    QDateTime startDate() const;
    QDateTime dueDate() const;
    Delegate delegate() const;

public slots:
    void setDone(bool done);
    void setStartDate(const QDateTime &startDate);
    void setDueDate(const QDateTime &dueDate);
    void setDelegate(const Domain::Task::Delegate &delegate);

signals:
    void doneChanged(bool isDone);
    void startDateChanged(const QDateTime &startDate);
    void dueDateChanged(const QDateTime &dueDate);
    void delegateChanged(const Domain::Task::Delegate &delegate);

private:
    bool m_done;
    QDateTime m_startDate;
    QDateTime m_dueDate;
    Delegate m_delegate;
};

}

Q_DECLARE_METATYPE(Domain::Task::Ptr)
Q_DECLARE_METATYPE(Domain::Task::List)
Q_DECLARE_METATYPE(Domain::Task::Delegate)

#endif // DOMAIN_TASK_H
