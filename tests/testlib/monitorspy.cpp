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
