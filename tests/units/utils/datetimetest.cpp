/* This file is part of Zanshin

   Copyright 2015 Theo Vaucher <theo.vaucher@gmail.com>

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

#include <testlib/qtest_zanshin.h>

#include "utils/datetime.h"

using namespace Utils;

class DateTimeTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldNotOverrideCurrentDateTime()
    {
        // GIVEN
        const auto todayDate = QDateTime::currentDateTime();

        // WHEN
        const QDateTime zanshinDate = DateTime::currentDateTime();

        // THEN
        QCOMPARE(zanshinDate.date(), todayDate.date());
    }

    void shouldOverrideCurrentDateTime()
    {
        // GIVEN
        const QByteArray dateExpected = "2015-03-10";
        qputenv("ZANSHIN_OVERRIDE_DATETIME", dateExpected);

        // WHEN
        const QDateTime zanshinDate = DateTime::currentDateTime();

        // THEN
        QCOMPARE(zanshinDate.date(), QDateTime::fromString(QString::fromLocal8Bit(dateExpected), Qt::ISODate).date());
    }

    void shouldNotOverrideCurrentDateTimeWhenInvalidDate()
    {
        // GIVEN
        const QByteArray dateExpected = "Invalid!";
        qputenv("ZANSHIN_OVERRIDE_DATETIME", dateExpected);

        // WHEN
        const QDateTime zanshinDate = DateTime::currentDateTime();

        // THEN
        QCOMPARE(zanshinDate.date(), QDateTime::currentDateTime().date());
    }

    void shouldNotOverrideCurrentDate()
    {
        // GIVEN
        const auto todayDate = QDate::currentDate();

        // WHEN
        const QDate zanshinDate = DateTime::currentDate();

        // THEN
        QCOMPARE(zanshinDate, todayDate);
    }

    void shouldOverrideCurrentDate()
    {
        // GIVEN
        const QByteArray dateExpected = "2015-03-10";
        qputenv("ZANSHIN_OVERRIDE_DATE", dateExpected);

        // WHEN
        const QDate zanshinDate = DateTime::currentDate();

        // THEN
        QCOMPARE(zanshinDate, QDate(2015, 3, 10));
    }

    void shouldNotOverrideCurrentDateWhenInvalidDate()
    {
        // GIVEN
        const QByteArray dateExpected = "Invalid!";
        qputenv("ZANSHIN_OVERRIDE_DATE", dateExpected);

        // WHEN
        const QDate zanshinDate = DateTime::currentDate();

        // THEN
        QCOMPARE(zanshinDate, QDate::currentDate());
    }
};

ZANSHIN_TEST_MAIN(DateTimeTest)

#include "datetimetest.moc"
