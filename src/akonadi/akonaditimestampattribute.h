/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_TIMESTAMPATTRIBUTE_H
#define AKONADI_TIMESTAMPATTRIBUTE_H

#include <QByteArray>

#include <Akonadi/Attribute>

namespace Akonadi {

class TimestampAttribute : public Attribute
{
public:
    TimestampAttribute();
    ~TimestampAttribute();

    qint64 timestamp() const;
    void refreshTimestamp();

    QByteArray type() const override;
    TimestampAttribute *clone() const override;

    QByteArray serialized() const override;
    void deserialize(const QByteArray &data) override;

private:
    qint64 m_timestamp;
};

}

#endif
