/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include "fakejob.h"

#include <QTimer>

FakeJob::FakeJob(QObject *parent)
    : KJob(parent),
      m_timer(new QTimer(this)),
      m_done(false),
      m_launched(false),
      m_errorCode(KJob::NoError)
{
    m_timer->setTimerType(Qt::PreciseTimer);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &FakeJob::onTimeout);
}

void FakeJob::setExpectedError(int errorCode, const QString &errorText)
{
    m_errorCode = errorCode;
    m_errorText = errorText;
}

void FakeJob::start()
{
    if (!m_launched) {
        m_launched = true;
        m_timer->start(DURATION);
    }
}

void FakeJob::onTimeout()
{
    if (m_errorCode == KJob::NoError)
        m_done = true;

    setError(m_errorCode);
    setErrorText(m_errorText);
    emitResult();
}

bool FakeJob::isDone() const
{
    return m_done;
}

int FakeJob::expectedError() const
{
    return m_errorCode;
}

QString FakeJob::expectedErrorText() const
{
    return m_errorText;
}

bool FakeJob::doKill()
{
    setError(KJob::KilledJobError);
    emitResult();
    return true;
}

#include "moc_fakejob.cpp"
