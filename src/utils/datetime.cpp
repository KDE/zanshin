/*
 * SPDX-FileCopyrightText: 2015 Theo Vaucher <theo.vaucher@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "datetime.h"

#include <QDateTime>

using namespace Utils;

QDate DateTime::currentDate()
{
    QByteArray overrideDate = qgetenv("ZANSHIN_OVERRIDE_DATE");
    QDate customDate = QDate::fromString(QString::fromLocal8Bit(overrideDate), Qt::ISODate);

    return customDate.isValid() ? customDate : QDate::currentDate();
}
