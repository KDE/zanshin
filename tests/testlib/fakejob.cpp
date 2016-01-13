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

#include "fakejob.h"

#include <QTimer>

FakeJob::FakeJob(QObject *parent)
    : KJob(parent), m_done(false), m_launched(false), m_errorCode(KJob::NoError)
{
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
        QTimer::singleShot(DURATION, Qt::PreciseTimer, this, SLOT(onTimeout()));
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
