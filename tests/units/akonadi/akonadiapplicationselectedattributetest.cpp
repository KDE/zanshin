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

#include <QtTest>

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
        auto clone = attr.clone();

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

QTEST_MAIN(AkonadiApplicationSelectedAttributeTest)

#include "akonadiapplicationselectedattributetest.moc"
