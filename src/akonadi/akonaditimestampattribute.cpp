/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "akonaditimestampattribute.h"

#include <QDateTime>

using namespace Akonadi;

TimestampAttribute::TimestampAttribute()
    : Attribute(),
      m_timestamp(0)
{
    refreshTimestamp();
}

TimestampAttribute::~TimestampAttribute()
{
}

qint64 TimestampAttribute::timestamp() const
{
    return m_timestamp;
}

void TimestampAttribute::refreshTimestamp()
{
    m_timestamp = QDateTime::currentMSecsSinceEpoch();
}

TimestampAttribute *TimestampAttribute::clone() const
{
    auto attr = new TimestampAttribute();
    attr->m_timestamp = m_timestamp;
    return attr;
}

QByteArray TimestampAttribute::type() const
{
    return "ZanshinTimestamp";
}

QByteArray TimestampAttribute::serialized() const
{
    return QByteArray::number(m_timestamp);
}

void TimestampAttribute::deserialize(const QByteArray &data)
{
    m_timestamp = data.toLongLong();
}
