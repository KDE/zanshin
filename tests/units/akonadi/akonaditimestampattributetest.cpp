/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
