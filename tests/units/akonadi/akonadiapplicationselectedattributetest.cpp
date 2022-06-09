/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include <memory>

#include "akonadi/akonadiapplicationselectedattribute.h"

class AkonadiApplicationSelectedAttributeTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldHaveDefaultState()
    {
        // GIVEN
        Akonadi::ApplicationSelectedAttribute attr;

        // THEN
        QCOMPARE(attr.isSelected(), true);
        QCOMPARE(attr.type(), QByteArray("ZanshinSelected"));
    }

    void shouldBeCloneable()
    {
        // GIVEN
        Akonadi::ApplicationSelectedAttribute attr;
        attr.setSelected(false);

        // WHEN
        auto clone = std::unique_ptr<Akonadi::ApplicationSelectedAttribute>(attr.clone());

        // THEN
        QCOMPARE(clone->isSelected(), attr.isSelected());
        QCOMPARE(clone->type(), attr.type());
    }

    void shouldDeserialize_data()
    {
        QTest::addColumn<QByteArray>("data");
        QTest::addColumn<bool>("selected");

        QTest::newRow("selected") << QByteArray("true") << true;
        QTest::newRow("deselected") << QByteArray("false") << false;
        QTest::newRow("malformed") << QByteArray("foo") << false;
        QTest::newRow("empty") << QByteArray() << false;
    }

    void shouldDeserialize()
    {
        // GIVEN
        QFETCH(QByteArray, data);
        QFETCH(bool, selected);

        Akonadi::ApplicationSelectedAttribute attr;

        // WHEN
        attr.deserialize(data);

        // THEN
        QCOMPARE(attr.isSelected(), selected);
    }

    void shouldSerialize()
    {
        // GIVEN
        Akonadi::ApplicationSelectedAttribute attr;

        // WHEN
        attr.setSelected(true);
        const QByteArray selectedData = attr.serialized();

        attr.setSelected(false);
        const QByteArray deselectedData = attr.serialized();

        // THEN
        QCOMPARE(selectedData, QByteArray("true"));
        QCOMPARE(deselectedData, QByteArray("false"));
    }
};

ZANSHIN_TEST_MAIN(AkonadiApplicationSelectedAttributeTest)

#include "akonadiapplicationselectedattributetest.moc"
