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

#include "utils/dependencymanager.h"

using namespace Utils;

class Interface
{
public:
    Interface() {}
    virtual ~Interface() {}
    virtual void doSomething() = 0;
};

class FirstImplementation : public Interface
{
public:
    virtual void doSomething() { qDebug() << "FirstImplementation"; }
};

class SecondImplementation : public Interface
{
public:
    virtual void doSomething() { qDebug() << "SecondImplementation"; }
};

class DependencyManagerTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldMemorizeDependency()
    {
        DependencyManager deps;
        deps.add<Interface, FirstImplementation>();
        Interface *object = deps.create<Interface>();
        QVERIFY(dynamic_cast<FirstImplementation*>(object) != 0);
    }

    void shouldMakeManagerSpecificDependencies()
    {
        DependencyManager deps1;
        deps1.add<Interface, FirstImplementation>();
        DependencyManager deps2;
        deps2.add<Interface, SecondImplementation>();

        Interface *object1 = deps1.create<Interface>();
        Interface *object2 = deps2.create<Interface>();

        QVERIFY(dynamic_cast<FirstImplementation*>(object1) != 0);
        QVERIFY(dynamic_cast<SecondImplementation*>(object2) != 0);
    }

    void shouldCleanupProviders()
    {
        QCOMPARE(Internal::Supplier<Interface>::providersCount(), 0);

        {
            DependencyManager deps1;
            deps1.add<Interface, FirstImplementation>();
            QCOMPARE(Internal::Supplier<Interface>::providersCount(), 1);

            {
                DependencyManager deps2;
                deps2.add<Interface, SecondImplementation>();
                QCOMPARE(Internal::Supplier<Interface>::providersCount(), 2);
            }

            QCOMPARE(Internal::Supplier<Interface>::providersCount(), 1);
        }

        QCOMPARE(Internal::Supplier<Interface>::providersCount(), 0);
    }
};

QTEST_MAIN(DependencyManagerTest)

#include "dependencymanagertest.moc"
