/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include "monitorspy.h"
#include <QTest>

static int s_expirationDelay = 1000;

int MonitorSpy::expirationDelay()
{
    return s_expirationDelay;
}

void MonitorSpy::setExpirationDelay(int value)
{
    s_expirationDelay = value;
}

MonitorSpy::MonitorSpy(Akonadi::MonitorInterface* monitor, QObject *parent)
    : QObject(parent),
      m_monitor(monitor),
      m_timer(new QTimer(this)),
      m_isFinished(false)
{
    m_timer->setTimerType(Qt::PreciseTimer);

    connect(m_monitor, &Akonadi::MonitorInterface::collectionAdded, this, &MonitorSpy::restartTimer);
    connect(m_monitor, &Akonadi::MonitorInterface::collectionRemoved, this, &MonitorSpy::restartTimer);
    connect(m_monitor, &Akonadi::MonitorInterface::collectionChanged, this, &MonitorSpy::restartTimer);
    connect(m_monitor, &Akonadi::MonitorInterface::collectionSelectionChanged, this, &MonitorSpy::restartTimer);

    connect(m_monitor, &Akonadi::MonitorInterface::itemAdded, this, &MonitorSpy::restartTimer);
    connect(m_monitor, &Akonadi::MonitorInterface::itemRemoved, this, &MonitorSpy::restartTimer);
    connect(m_monitor, &Akonadi::MonitorInterface::itemChanged, this, &MonitorSpy::restartTimer);
    connect(m_monitor, &Akonadi::MonitorInterface::itemMoved, this, &MonitorSpy::restartTimer);

    connect(m_timer, &QTimer::timeout, this, &MonitorSpy::onDelayExpired);
}

void MonitorSpy::waitForStableState()
{
    m_isFinished = false;
    m_timer->start(s_expirationDelay);
    while (!m_isFinished)
        QTest::qWait(20);
}

void MonitorSpy::restartTimer()
{
    m_timer->start(s_expirationDelay);
}

void MonitorSpy::onDelayExpired()
{
    m_isFinished = true;
}
