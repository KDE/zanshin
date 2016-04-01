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

#include <testlib/qtest_zanshin.h>

#include <memory>

#include "akonadi/akonaditimestampattribute.h"

class AkonadiTimestampAttributeTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldHaveDefaultState()
    {
        // GIVEN
        const auto timestamp = QDateTime::currentMSecsSinceEpoch();
        Akonadi::TimestampAttribute attr;

        // THEN
        QVERIFY(attr.timestamp() >= timestamp);
        QCOMPARE(attr.type(), QByteArray("ZanshinTimestamp"));
    }

    void shouldRefreshTimestamp()
    {
        // GIVEN
        Akonadi::TimestampAttribute attr;
        QTest::qWait(50);
        auto timestamp = QDateTime::currentMSecsSinceEpoch();
        QVERIFY(attr.timestamp() < timestamp);

        // WHEN
        QTest::qWait(50);
        attr.refreshTimestamp();

        // THEN
        QVERIFY(attr.timestamp() > timestamp);
    }

    void shouldBeCloneable()
    {
        // GIVEN
        Akonadi::TimestampAttribute attr;

        // WHEN
        auto clone = std::unique_ptr<Akonadi::TimestampAttribute>(attr.clone());

        // THEN
        QCOMPARE(clone->timestamp(), attr.timestamp());
        QCOMPARE(clone->type(), attr.type());
    }

    void shouldDeserialize()
    {
        // GIVEN
        Akonadi::TimestampAttribute attr;

        // WHEN
        attr.deserialize("42");

        // THEN
        QCOMPARE(attr.timestamp(), qint64(42));
    }

    void shouldSerialize()
    {
        // GIVEN
        Akonadi::TimestampAttribute attr;

        // WHEN
        const auto data = attr.serialized();

        // THEN
        QCOMPARE(data, QByteArray::number(attr.timestamp()));
    }
};

ZANSHIN_TEST_MAIN(AkonadiTimestampAttributeTest)

#include "akonaditimestampattributetest.moc"
