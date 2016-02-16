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

#include <mockitopp/mockitopp.hpp>

class FakeInterface
{
public:
    virtual ~FakeInterface();
    virtual void doSomething() = 0;
    virtual int computeMe(QString input) = 0;
};

using namespace mockitopp;
using mockitopp::matcher::any;

class MockitoTest : public QObject
{
    Q_OBJECT
private slots:
    void testBasics()
    {
        mock_object<FakeInterface> mock;
        mock(&FakeInterface::doSomething).when().thenReturn();
        mock(&FakeInterface::doSomething).when().thenThrow("exception");
        for (int i = 0; i < 10; i++) {
            mock(&FakeInterface::computeMe).when(QStringLiteral("A")).thenReturn(0);
            mock(&FakeInterface::computeMe).when(QStringLiteral("B")).thenReturn(1);
            mock(&FakeInterface::computeMe).when(QStringLiteral("C")).thenReturn(-1);
            mock(&FakeInterface::computeMe).when(QStringLiteral("Foo")).thenReturn(-1);
        }
        FakeInterface &iface = mock.getInstance();

        iface.doSomething();
        try {
            iface.doSomething();
            QFAIL("No exception thrown");
        } catch (...) {
            // We expect to catch something
        }

        for (int i = 0; i < 10; i++) {
            QCOMPARE(iface.computeMe(QStringLiteral("A")), 0);
            QCOMPARE(iface.computeMe(QStringLiteral("B")), 1);
            QCOMPARE(iface.computeMe(QStringLiteral("C")), -1);
            QCOMPARE(iface.computeMe(QStringLiteral("Foo")), -1);
        }

        QVERIFY(mock(&FakeInterface::doSomething).when().exactly(2));
        QVERIFY(mock(&FakeInterface::computeMe).when(any<QString>()).exactly(40));
    }
};

ZANSHIN_TEST_MAIN(MockitoTest)

#include "mockitotest.moc"
