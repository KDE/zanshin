/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef ZANSHIN_TESTLIB_MONITORSPY_H
#define ZANSHIN_TESTLIB_MONITORSPY_H

#include "akonadi/akonadimonitorimpl.h"

#include <QObject>
#include <QTimer>

class MonitorSpy : public QObject
{
    Q_OBJECT
public:
    static int expirationDelay();
    static void setExpirationDelay(int value);

    explicit MonitorSpy(Akonadi::MonitorInterface *monitor, QObject *parent = nullptr);

    void waitForStableState();

private slots:
    void restartTimer();
    void onDelayExpired();

private:
    Akonadi::MonitorInterface *m_monitor;
    QTimer *m_timer;
    bool m_isFinished;
};

#endif
