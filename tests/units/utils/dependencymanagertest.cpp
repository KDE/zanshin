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

class Interface0
{
public:
    Interface0() {}
    virtual ~Interface0() {}
    virtual void doSomething() = 0;
};

class FirstImplementation0 : public Interface0
{
public:
    virtual void doSomething() { qDebug() << "FirstImplementation"; }
};

class SecondImplementation0 : public Interface0
{
public:
    virtual void doSomething() { qDebug() << "SecondImplementation"; }
};

#define DECLARE_IMPLEMENTED_INTERFACE(N) \
class Interface##N \
{ \
public: \
    Interface##N() {} \
    virtual ~Interface##N() {} \
    virtual void doSomething() = 0; \
}; \
 \
class Implementation##N : public Interface##N \
{ \
public: \
    virtual void doSomething() { qDebug() << "Implementation##N"; } \
};

DECLARE_IMPLEMENTED_INTERFACE(1)
DECLARE_IMPLEMENTED_INTERFACE(2)
DECLARE_IMPLEMENTED_INTERFACE(3)
DECLARE_IMPLEMENTED_INTERFACE(4)
DECLARE_IMPLEMENTED_INTERFACE(5)
DECLARE_IMPLEMENTED_INTERFACE(6)
DECLARE_IMPLEMENTED_INTERFACE(7)
DECLARE_IMPLEMENTED_INTERFACE(8)
DECLARE_IMPLEMENTED_INTERFACE(9)

class AnotherInterface
{
public:
    AnotherInterface() {}
    virtual ~AnotherInterface() {}
    virtual void doSomethingDelegated() = 0;
};

class AnotherFirstImplementation : public AnotherInterface
{
public:
    AnotherFirstImplementation(Interface0 *iface)
        : m_iface(iface) {}

    void doSomethingDelegated() Q_DECL_OVERRIDE { m_iface->doSomething(); }

    Interface0 *iface() const { return m_iface; }

private:
    Interface0 *m_iface;
};

class AnotherSecondImplementation : public AnotherInterface
{
public:
    AnotherSecondImplementation(Interface0 *iface0,
                                Interface1 *iface1,
                                Interface2 *iface2,
                                Interface3 *iface3,
                                Interface4 *iface4,
                                Interface5 *iface5,
                                Interface6 *iface6,
                                Interface7 *iface7,
                                Interface8 *iface8,
                                Interface9 *iface9)
        : m_iface0(iface0),
          m_iface1(iface1),
          m_iface2(iface2),
          m_iface3(iface3),
          m_iface4(iface4),
          m_iface5(iface5),
          m_iface6(iface6),
          m_iface7(iface7),
          m_iface8(iface8),
          m_iface9(iface9)
    {
    }

    void doSomethingDelegated() Q_DECL_OVERRIDE { m_iface1->doSomething(); }

    Interface0 *iface0() const { return m_iface0; }
    Interface1 *iface1() const { return m_iface1; }
    Interface2 *iface2() const { return m_iface2; }
    Interface3 *iface3() const { return m_iface3; }
    Interface4 *iface4() const { return m_iface4; }
    Interface5 *iface5() const { return m_iface5; }
    Interface6 *iface6() const { return m_iface6; }
    Interface7 *iface7() const { return m_iface7; }
    Interface8 *iface8() const { return m_iface8; }
    Interface9 *iface9() const { return m_iface9; }

private:
    Interface0 *m_iface0;
    Interface1 *m_iface1;
    Interface2 *m_iface2;
    Interface3 *m_iface3;
    Interface4 *m_iface4;
    Interface5 *m_iface5;
    Interface6 *m_iface6;
    Interface7 *m_iface7;
    Interface8 *m_iface8;
    Interface9 *m_iface9;
};

class DependencyManagerTest : public QObject
{
    Q_OBJECT
private:
    static bool s_firstImplFactoryCalled;
    static DependencyManager *s_manager;

    static Interface0 *firstImplFactory(DependencyManager *manager)
    {
        s_firstImplFactoryCalled = true;
        s_manager = manager;
        return new FirstImplementation0;
    }

private slots:
    void shouldMemorizeDependency()
    {
        DependencyManager deps;
        deps.add<Interface0, FirstImplementation0>();
        Interface0 *object = deps.create<Interface0>();
        QVERIFY(dynamic_cast<FirstImplementation0*>(object) != 0);
    }

    void shouldAllowOurOwnFactory()
    {
        s_firstImplFactoryCalled = false;
        s_manager = nullptr;
        DependencyManager deps;
        deps.add<Interface0>(&DependencyManagerTest::firstImplFactory);
        Interface0 *object = deps.create<Interface0>();
        QVERIFY(dynamic_cast<FirstImplementation0*>(object) != 0);
        QVERIFY(s_firstImplFactoryCalled);
        QVERIFY(s_manager == &deps);
    }

    void shouldAllowOurOwnFactoryAsLambda()
    {
#ifdef Q_COMPILER_LAMBDA
        bool ownFactoryCalled = false;
        DependencyManager *managerCalled = nullptr;

        DependencyManager deps;
        deps.add<Interface0>([&](DependencyManager *manager) -> Interface0* {
            ownFactoryCalled = true;
            managerCalled = manager;
            return new FirstImplementation0;
        });
        Interface0 *object = deps.create<Interface0>();
        QVERIFY(dynamic_cast<FirstImplementation0*>(object) != 0);
        QVERIFY(ownFactoryCalled);
        QVERIFY(managerCalled == &deps);
#endif
    }

    void shouldMakeManagerSpecificDependencies()
    {
        DependencyManager deps1;
        deps1.add<Interface0, FirstImplementation0>();
        DependencyManager deps2;
        deps2.add<Interface0, SecondImplementation0>();

        Interface0 *object1 = deps1.create<Interface0>();
        Interface0 *object2 = deps2.create<Interface0>();

        QVERIFY(dynamic_cast<FirstImplementation0*>(object1) != 0);
        QVERIFY(dynamic_cast<SecondImplementation0*>(object2) != 0);
    }

    void shouldCleanupProviders()
    {
        QCOMPARE(Internal::Supplier<Interface0>::providersCount(), 0);

        {
            DependencyManager deps1;
            deps1.add<Interface0, FirstImplementation0>();
            QCOMPARE(Internal::Supplier<Interface0>::providersCount(), 1);

            {
                DependencyManager deps2;
                deps2.add<Interface0, SecondImplementation0>();
                QCOMPARE(Internal::Supplier<Interface0>::providersCount(), 2);
            }

            QCOMPARE(Internal::Supplier<Interface0>::providersCount(), 1);
        }

        QCOMPARE(Internal::Supplier<Interface0>::providersCount(), 0);
    }

    void shouldInjectDependencyInConstructor()
    {
        DependencyManager deps;
        deps.add<Interface0, FirstImplementation0>();
        deps.add<AnotherInterface, AnotherFirstImplementation(Interface0*)>();

        auto object = deps.create<AnotherInterface>();
        auto impl = dynamic_cast<AnotherFirstImplementation*>(object);
        QVERIFY(impl != 0);
        QVERIFY(dynamic_cast<FirstImplementation0*>(impl->iface()) != 0);
    }

    void shouldInjectDependenciesInConstructor()
    {
        DependencyManager deps;
        deps.add<Interface0, FirstImplementation0>();
        deps.add<Interface1, Implementation1>();
        deps.add<Interface2, Implementation2>();
        deps.add<Interface3, Implementation3>();
        deps.add<Interface4, Implementation4>();
        deps.add<Interface5, Implementation5>();
        deps.add<Interface6, Implementation6>();
        deps.add<Interface7, Implementation7>();
        deps.add<Interface8, Implementation8>();
        deps.add<Interface9, Implementation9>();
        deps.add<AnotherInterface, AnotherSecondImplementation(Interface0*,
                                                               Interface1*,
                                                               Interface2*,
                                                               Interface3*,
                                                               Interface4*,
                                                               Interface5*,
                                                               Interface6*,
                                                               Interface7*,
                                                               Interface8*,
                                                               Interface9*)>();
        auto object = deps.create<AnotherInterface>();
        auto impl = dynamic_cast<AnotherSecondImplementation*>(object);
        QVERIFY(impl != 0);
        QVERIFY(dynamic_cast<FirstImplementation0*>(impl->iface0()) != 0);
        QVERIFY(dynamic_cast<Implementation1*>(impl->iface1()) != 0);
        QVERIFY(dynamic_cast<Implementation2*>(impl->iface2()) != 0);
        QVERIFY(dynamic_cast<Implementation3*>(impl->iface3()) != 0);
        QVERIFY(dynamic_cast<Implementation4*>(impl->iface4()) != 0);
        QVERIFY(dynamic_cast<Implementation5*>(impl->iface5()) != 0);
        QVERIFY(dynamic_cast<Implementation6*>(impl->iface6()) != 0);
        QVERIFY(dynamic_cast<Implementation7*>(impl->iface7()) != 0);
        QVERIFY(dynamic_cast<Implementation8*>(impl->iface8()) != 0);
        QVERIFY(dynamic_cast<Implementation9*>(impl->iface9()) != 0);
    }
};

bool DependencyManagerTest::s_firstImplFactoryCalled = false;
DependencyManager *DependencyManagerTest::s_manager = nullptr;

QTEST_MAIN(DependencyManagerTest)

#include "dependencymanagertest.moc"
