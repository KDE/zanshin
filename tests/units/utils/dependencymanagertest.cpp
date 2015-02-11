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
    typedef QSharedPointer<Interface0> Ptr;

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
    typedef QSharedPointer<Interface##N> Ptr; \
 \
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
DECLARE_IMPLEMENTED_INTERFACE(10)
DECLARE_IMPLEMENTED_INTERFACE(11)
DECLARE_IMPLEMENTED_INTERFACE(12)
DECLARE_IMPLEMENTED_INTERFACE(13)
DECLARE_IMPLEMENTED_INTERFACE(14)

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
    AnotherFirstImplementation(const Interface0::Ptr &iface)
        : m_iface(iface) {}

    void doSomethingDelegated() Q_DECL_OVERRIDE { m_iface->doSomething(); }

    Interface0::Ptr iface() const { return m_iface; }

private:
    Interface0::Ptr m_iface;
};

class AnotherSecondImplementation : public AnotherInterface
{
public:
    AnotherSecondImplementation(Interface0::Ptr iface0,
                                Interface1::Ptr iface1,
                                Interface2::Ptr iface2,
                                Interface3::Ptr iface3,
                                Interface4::Ptr iface4,
                                Interface5::Ptr iface5,
                                Interface6::Ptr iface6,
                                Interface7::Ptr iface7,
                                Interface8::Ptr iface8,
                                Interface9::Ptr iface9,
                                Interface10::Ptr iface10,
                                Interface11::Ptr iface11,
                                Interface12::Ptr iface12,
                                Interface13::Ptr iface13,
                                Interface14::Ptr iface14)
        : m_iface0(iface0),
          m_iface1(iface1),
          m_iface2(iface2),
          m_iface3(iface3),
          m_iface4(iface4),
          m_iface5(iface5),
          m_iface6(iface6),
          m_iface7(iface7),
          m_iface8(iface8),
          m_iface9(iface9),
          m_iface10(iface10),
          m_iface11(iface11),
          m_iface12(iface12),
          m_iface13(iface13),
          m_iface14(iface14)
    {
    }

    void doSomethingDelegated() Q_DECL_OVERRIDE { m_iface1->doSomething(); }

    Interface0::Ptr iface0() const { return m_iface0; }
    Interface1::Ptr iface1() const { return m_iface1; }
    Interface2::Ptr iface2() const { return m_iface2; }
    Interface3::Ptr iface3() const { return m_iface3; }
    Interface4::Ptr iface4() const { return m_iface4; }
    Interface5::Ptr iface5() const { return m_iface5; }
    Interface6::Ptr iface6() const { return m_iface6; }
    Interface7::Ptr iface7() const { return m_iface7; }
    Interface8::Ptr iface8() const { return m_iface8; }
    Interface9::Ptr iface9() const { return m_iface9; }
    Interface10::Ptr iface10() const { return m_iface10; }
    Interface11::Ptr iface11() const { return m_iface11; }
    Interface12::Ptr iface12() const { return m_iface12; }
    Interface13::Ptr iface13() const { return m_iface13; }
    Interface14::Ptr iface14() const { return m_iface14; }

private:
    Interface0::Ptr m_iface0;
    Interface1::Ptr m_iface1;
    Interface2::Ptr m_iface2;
    Interface3::Ptr m_iface3;
    Interface4::Ptr m_iface4;
    Interface5::Ptr m_iface5;
    Interface6::Ptr m_iface6;
    Interface7::Ptr m_iface7;
    Interface8::Ptr m_iface8;
    Interface9::Ptr m_iface9;
    Interface10::Ptr m_iface10;
    Interface11::Ptr m_iface11;
    Interface12::Ptr m_iface12;
    Interface13::Ptr m_iface13;
    Interface14::Ptr m_iface14;
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
        auto object1 = deps.create<Interface0>();
        QVERIFY(object1.dynamicCast<FirstImplementation0>());
        auto object2 = deps.create<Interface0>();
        QVERIFY(object2.dynamicCast<FirstImplementation0>());
        QVERIFY(object1 != object2);
    }

    void shouldAllowOurOwnFactory()
    {
        s_firstImplFactoryCalled = false;
        s_manager = nullptr;
        DependencyManager deps;
        deps.add<Interface0>(&DependencyManagerTest::firstImplFactory);
        auto object = deps.create<Interface0>();
        QVERIFY(object.dynamicCast<FirstImplementation0>());
        QVERIFY(s_firstImplFactoryCalled);
        QVERIFY(s_manager == &deps);
    }

    void shouldAllowUniqueInstances()
    {
        DependencyManager deps;
        deps.add<Interface0, FirstImplementation0, DependencyManager::UniqueInstance>();
        auto object1 = deps.create<Interface0>();
        QVERIFY(object1.dynamicCast<FirstImplementation0>());
        auto object2 = deps.create<Interface0>();
        QVERIFY(object2.dynamicCast<FirstImplementation0>());
        QVERIFY(object1 == object2);
    }

    void shouldAllowUniqueInstancesWithOurOwnFactory()
    {
        s_firstImplFactoryCalled = false;
        s_manager = nullptr;
        DependencyManager deps;
        deps.add<Interface0, DependencyManager::UniqueInstance>(&DependencyManagerTest::firstImplFactory);
        auto object1 = deps.create<Interface0>();
        QVERIFY(object1.dynamicCast<FirstImplementation0>());
        auto object2 = deps.create<Interface0>();
        QVERIFY(object2.dynamicCast<FirstImplementation0>());
        QVERIFY(s_firstImplFactoryCalled);
        QVERIFY(s_manager == &deps);
        QVERIFY(object1 == object2);
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
        auto object = deps.create<Interface0>();
        QVERIFY(object.dynamicCast<FirstImplementation0>());
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

        auto object1 = deps1.create<Interface0>();
        auto object2 = deps2.create<Interface0>();

        QVERIFY(object1.dynamicCast<FirstImplementation0>());
        QVERIFY(object2.dynamicCast<SecondImplementation0>());
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
        auto impl = object.dynamicCast<AnotherFirstImplementation>();
        QVERIFY(impl != Q_NULLPTR);
        QVERIFY(impl->iface().dynamicCast<FirstImplementation0>());
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
        deps.add<Interface10, Implementation10>();
        deps.add<Interface11, Implementation11>();
        deps.add<Interface12, Implementation12>();
        deps.add<Interface13, Implementation13>();
        deps.add<Interface14, Implementation14>();
        deps.add<AnotherInterface, AnotherSecondImplementation(Interface0*,
                                                               Interface1*,
                                                               Interface2*,
                                                               Interface3*,
                                                               Interface4*,
                                                               Interface5*,
                                                               Interface6*,
                                                               Interface7*,
                                                               Interface8*,
                                                               Interface9*,
                                                               Interface10*,
                                                               Interface11*,
                                                               Interface12*,
                                                               Interface13*,
                                                               Interface14*)>();
        auto object = deps.create<AnotherInterface>();
        auto impl = object.dynamicCast<AnotherSecondImplementation>();
        QVERIFY(impl != Q_NULLPTR);
        QVERIFY(impl->iface0().dynamicCast<FirstImplementation0>());
        QVERIFY(impl->iface1().dynamicCast<Implementation1>());
        QVERIFY(impl->iface2().dynamicCast<Implementation2>());
        QVERIFY(impl->iface3().dynamicCast<Implementation3>());
        QVERIFY(impl->iface4().dynamicCast<Implementation4>());
        QVERIFY(impl->iface5().dynamicCast<Implementation5>());
        QVERIFY(impl->iface6().dynamicCast<Implementation6>());
        QVERIFY(impl->iface7().dynamicCast<Implementation7>());
        QVERIFY(impl->iface8().dynamicCast<Implementation8>());
        QVERIFY(impl->iface9().dynamicCast<Implementation9>());
        QVERIFY(impl->iface10().dynamicCast<Implementation10>());
        QVERIFY(impl->iface11().dynamicCast<Implementation11>());
        QVERIFY(impl->iface12().dynamicCast<Implementation12>());
        QVERIFY(impl->iface13().dynamicCast<Implementation13>());
        QVERIFY(impl->iface14().dynamicCast<Implementation14>());
    }
};

bool DependencyManagerTest::s_firstImplFactoryCalled = false;
DependencyManager *DependencyManagerTest::s_manager = nullptr;

QTEST_MAIN(DependencyManagerTest)

#include "dependencymanagertest.moc"
