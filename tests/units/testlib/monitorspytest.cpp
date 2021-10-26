/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "testlib/monitorspy.h"
#include "testlib/akonadifakemonitor.h"
#include <AkonadiCore/Akonadi/Item>

using namespace Testlib;

class TimerTest: public QObject
{
    Q_OBJECT
public:
    TimerTest(AkonadiFakeMonitor *monitor, int duration)
        : m_monitor(monitor)
    {
        QTimer::singleShot(duration, Qt::PreciseTimer, this, &TimerTest::addItem);
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
    explicit MonitorSpyTest(QObject *parent = nullptr)
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

ZANSHIN_TEST_MAIN(MonitorSpyTest)

#include "monitorspytest.moc"
