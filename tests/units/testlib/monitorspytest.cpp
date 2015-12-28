/* This file is part of Zanshin

   Copyright 2014 Mario Bensi <mbensi@ipsquad.net>

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

#include <QtTest>

#include "testlib/monitorspy.h"
#include "testlib/akonadifakemonitor.h"
#include <AkonadiCore/Item>

using namespace Testlib;

class TimerTest: public QObject
{
    Q_OBJECT
public:
    TimerTest(AkonadiFakeMonitor *monitor, int duration)
        : m_monitor(monitor)
    {
        QTimer::singleShot(duration, this, SLOT(addItem()));
    }

private slots:
    void addItem()
    {
        Akonadi::Item item;
        m_monitor->addItem(item);
    }

private:
    AkonadiFakeMonitor *m_monitor;
};

class MonitorSpyTest : public QObject
{
    Q_OBJECT
public:
    explicit MonitorSpyTest(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
    }

private slots:
    void shouldWaitOneSecondWithoutSignal()
    {
        // GIVEN
        AkonadiFakeMonitor monitor;
        MonitorSpy monitorSpy(&monitor);
        Akonadi::Item item;
        QDateTime now = QDateTime::currentDateTime();

        // WHEN
        monitorSpy.waitForStableState();

        // THEN
        QVERIFY(now.msecsTo(QDateTime::currentDateTime()) >= 1000ll);
    }

    void shouldWaitOneSecondAfterLastSignal()
    {
        // GIVEN
        AkonadiFakeMonitor monitor;
        MonitorSpy monitorSpy(&monitor);
        QDateTime now = QDateTime::currentDateTime();

        // WHEN
        TimerTest timer(&monitor, 500);
        monitorSpy.waitForStableState();

        // THEN
        QVERIFY(now.msecsTo(QDateTime::currentDateTime()) >= 1500ll);
    }
};

QTEST_MAIN(MonitorSpyTest)

#include "monitorspytest.moc"
