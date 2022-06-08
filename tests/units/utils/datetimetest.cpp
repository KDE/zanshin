/*
 * SPDX-FileCopyrightText: 2015 Theo Vaucher <theo.vaucher@gmail.com>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "utils/datetime.h"

using namespace Utils;

class DateTimeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
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
