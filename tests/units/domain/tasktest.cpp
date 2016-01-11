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

#include <testlib/qtest_zanshin.h>

#include "domain/task.h"

#include "utils/datetime.h"

using namespace Domain;

class TaskTest : public QObject
{
    Q_OBJECT
public:
    explicit TaskTest(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
        qRegisterMetaType<Task::Delegate>();
    }

private slots:
    void shouldHaveEmptyPropertiesByDefault()
    {
        Task t;
        QCOMPARE(t.text(), QString());
        QCOMPARE(t.title(), QString());
        QCOMPARE(t.isDone(), false);
        QCOMPARE(t.startDate(), QDateTime());
        QCOMPARE(t.dueDate(), QDateTime());
        QCOMPARE(t.doneDate(), QDateTime());
        QVERIFY(!t.delegate().isValid());
    }

    void shouldHaveValueBasedDelegate()
    {
        Task::Delegate d;
        QVERIFY(!d.isValid());
        QCOMPARE(d.name(), QString());
        QCOMPARE(d.email(), QString());
        QCOMPARE(d.display(), QString());

        d.setName("John Doe");
        QVERIFY(!d.isValid());
        QCOMPARE(d.name(), QString("John Doe"));
        QCOMPARE(d.email(), QString());
        QCOMPARE(d.display(), QString());

        d.setEmail("doe@somewhere.com");
        QVERIFY(d.isValid());
        QCOMPARE(d.name(), QString("John Doe"));
        QCOMPARE(d.email(), QString("doe@somewhere.com"));
        QCOMPARE(d.display(), QString("John Doe"));

        d.setName(QString());
        QVERIFY(d.isValid());
        QCOMPARE(d.name(), QString());
        QCOMPARE(d.email(), QString("doe@somewhere.com"));
        QCOMPARE(d.display(), QString("doe@somewhere.com"));
    }

    void shouldNotifyStatusChanges()
    {
        Task t;
        QSignalSpy spy(&t, SIGNAL(doneChanged(bool)));
        t.setDone(true);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toBool(), true);
    }

    void shouldNotNotifyIdenticalStatusChanges()
    {
        Task t;
        t.setDone(true);
        QSignalSpy spy(&t, SIGNAL(doneChanged(bool)));
        t.setDone(true);
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyStartDateChanges()
    {
        Task t;
        QSignalSpy spy(&t, SIGNAL(startDateChanged(QDateTime)));
        t.setStartDate(QDateTime(QDate(2014, 1, 13)));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toDateTime(), QDateTime(QDate(2014, 1, 13)));
    }

    void shouldNotNotifyIdenticalStartDateChanges()
    {
        Task t;
        t.setStartDate(QDateTime(QDate(2014, 1, 13)));
        QSignalSpy spy(&t, SIGNAL(startDateChanged(QDateTime)));
        t.setStartDate(QDateTime(QDate(2014, 1, 13)));
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyDueDateChanges()
    {
        Task t;
        QSignalSpy spy(&t, SIGNAL(dueDateChanged(QDateTime)));
        t.setDueDate(QDateTime(QDate(2014, 1, 13)));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toDateTime(), QDateTime(QDate(2014, 1, 13)));
    }

    void shouldNotNotifyIdenticalDueDateChanges()
    {
        Task t;
        t.setDueDate(QDateTime(QDate(2014, 1, 13)));
        QSignalSpy spy(&t, SIGNAL(dueDateChanged(QDateTime)));
        t.setDueDate(QDateTime(QDate(2014, 1, 13)));
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyDelegateChanges()
    {
        Task t;
        QSignalSpy spy(&t, SIGNAL(delegateChanged(Domain::Task::Delegate)));
        t.setDelegate(Task::Delegate("John Doe", "doe@somewhere.com"));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().value<Task::Delegate>(),
                 Task::Delegate("John Doe", "doe@somewhere.com"));
    }

    void shouldNotNotifyIdenticalDelegateChanges()
    {
        Task t;
        t.setDelegate(Task::Delegate("John Doe", "doe@somewhere.com"));
        QSignalSpy spy(&t, SIGNAL(delegateChanged(Domain::Task::Delegate)));
        t.setDelegate(Task::Delegate("John Doe", "doe@somewhere.com"));
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyDoneDateChanges()
    {
        Task t;
        QSignalSpy spy(&t, SIGNAL(doneDateChanged(QDateTime)));
        t.setDoneDate(QDateTime(QDate(2014, 1, 13)));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toDateTime(), QDateTime(QDate(2014, 1, 13)));
    }

    void shouldNotNotifyIdenticalDoneDateChanges()
    {
        Task t;
        t.setDoneDate(QDateTime(QDate(2014, 1, 13)));
        QSignalSpy spy(&t, SIGNAL(doneDateChanged(QDateTime)));
        t.setDoneDate(QDateTime(QDate(2014, 1, 13)));
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyDoneDateSet()
    {
        Task t;
        QSignalSpy spy(&t, SIGNAL(doneDateChanged(QDateTime)));
        t.setDone(true);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toDateTime().date(), Utils::DateTime::currentDateTime().date());
    }

    void shouldNotifyDoneDateUnset()
    {
        Task t;

        t.setDone(true);
        QSignalSpy spy(&t, SIGNAL(doneDateChanged(QDateTime)));
        t.setDone(false);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toDateTime(), QDateTime());
    }
};

ZANSHIN_TEST_MAIN(TaskTest)

#include "tasktest.moc"
