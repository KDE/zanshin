/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
