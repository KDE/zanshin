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


#ifndef UTILS_DEPENDENCYMANAGER_H
#define UTILS_DEPENDENCYMANAGER_H

#include <QList>
#include <QHash>
#include <QSharedPointer>

#include <utility>

#ifdef Q_COMPILER_LAMBDA
#include <functional>
#endif

namespace Utils {

class DependencyManager;

namespace Internal {
    template<class Iface>
    class Provider
    {
    public:
#ifdef Q_COMPILER_LAMBDA
        typedef std::function<Iface*(DependencyManager*)> FactoryType;
        typedef std::function<QSharedPointer<Iface>(FactoryType, DependencyManager*)> PolicyType;
#else
        typedef Iface *(*FactoryType)(DependencyManager*);
        typedef QSharedPointer<Iface> (*PolicyType)(FactoryType, DependencyManager*);
#endif

        Provider()
            : m_factory(nullptr),
              m_policy(nullptr)
        {
        }

        Provider(FactoryType factory, PolicyType policy)
            : m_factory(factory),
              m_policy(policy)
        {
        }

        Provider(const Provider &other)
            : m_factory(other.m_factory),
              m_policy(other.m_policy)
        {
        }

        ~Provider()
        {
        }

        Provider &operator=(const Provider &other)
        {
            Provider tmp(other);
            std::swap(m_factory, tmp.m_factory);
            std::swap(m_policy, tmp.m_policy);
            return *this;
        }

        QSharedPointer<Iface> operator()(DependencyManager *deps) const
        {
            Q_ASSERT(m_factory != nullptr);
            return m_policy(m_factory, deps);
        }

    private:
        FactoryType m_factory;
        PolicyType m_policy;
    };

    template<class Iface>
    class Supplier
    {
    public:
        static void setProvider(DependencyManager *manager, const Provider<Iface> &provider)
        {
            s_providers.insert(manager, provider);
        }

        static QSharedPointer<Iface> create(DependencyManager *manager)
        {
            return s_providers.value(manager)(manager);
        }

        static int providersCount()
        {
            return s_providers.size();
        }

        static void removeProvider(DependencyManager *manager)
        {
            s_providers.remove(manager);
        }

    private:
        Supplier();

        static QHash< DependencyManager*, Provider<Iface> > s_providers;
    };

    template<class Iface>
    QHash< DependencyManager*, Provider<Iface> > Supplier<Iface>::s_providers;

    struct MultipleInstancesPolicy
    {
        template<class Iface>
        static typename Provider<Iface>::PolicyType function(Iface *)
        {
            return create<Iface>;
        }

        template<class Iface>
        static QSharedPointer<Iface> create(typename Provider<Iface>::FactoryType factory,
                                            DependencyManager *deps)
        {
            return QSharedPointer<Iface>(factory(deps));
        }
    };

    struct UniqueInstancePolicy
    {
        template<class Iface>
        static typename Provider<Iface>::PolicyType function(Iface *)
        {
            return create<Iface>;
        }

        template<class Iface>
        static QSharedPointer<Iface> create(typename Provider<Iface>::FactoryType factory,
                                            DependencyManager *deps)
        {
            static QWeakPointer<Iface> weakRef;

            QSharedPointer<Iface> instance = weakRef.toStrongRef();
            if (!instance) {
                instance = QSharedPointer<Iface>(factory(deps));
                weakRef = instance;
            }
            return instance;
        }
    };
}

class DependencyManager
{
public:
    typedef Internal::MultipleInstancesPolicy MultipleInstances;
    typedef Internal::UniqueInstancePolicy UniqueInstance;

    static DependencyManager &globalInstance();

    DependencyManager();
    DependencyManager(const DependencyManager &other);
    ~DependencyManager();

    DependencyManager &operator=(const DependencyManager &other);

    template<class Iface, class InstancePolicy>
    void add(const typename Internal::Provider<Iface>::FactoryType &factory)
    {
        Iface * const val = nullptr;
        Internal::Provider<Iface> provider(factory, InstancePolicy::function(val));
        Internal::Supplier<Iface>::setProvider(this, provider);
        m_cleanupFunctions << Internal::Supplier<Iface>::removeProvider;
    }

    template<class Iface>
    void add(const typename Internal::Provider<Iface>::FactoryType &factory)
    {
        add<Iface, MultipleInstances>(factory);
    }

    template<class Iface, class Signature, class InstancePolicy>
    void add()
    {
        add<Iface, InstancePolicy>(DependencyManager::FactoryHelper<Iface, Signature>::create);
    }

    template<class Iface, class Signature>
    void add()
    {
        add<Iface, Signature, MultipleInstances>();
    }

    template<class Iface>
    QSharedPointer<Iface> create()
    {
        return Internal::Supplier<Iface>::create(this);
    }

private:
    QList<void (*)(DependencyManager*)> m_cleanupFunctions;

    template<class Iface, class Impl>
    class FactoryHelper
    {
    public:
        static Iface *create(DependencyManager *)
        {
            return new Impl;
        }
    };

#ifdef Q_COMPILER_VARIADIC_TEMPLATES
    template<class Iface, class Impl, class... Args>
    class FactoryHelper<Iface, Impl(Args*...)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl((manager->create<Args>())...);
        }
    };
#else
    template<class Iface, class Impl, class Arg0>
    class FactoryHelper<Iface, Impl(Arg0*)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>());
        }
    };

    template<class Iface, class Impl, class Arg0, class Arg1>
    class FactoryHelper<Iface, Impl(Arg0*, Arg1*)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>());
        }
    };

    template<class Iface, class Impl, class Arg0, class Arg1, class Arg2>
    class FactoryHelper<Iface, Impl(Arg0*, Arg1*, Arg2*)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>());
        }
    };

    template<class Iface, class Impl, class Arg0, class Arg1, class Arg2,
                                      class Arg3>
    class FactoryHelper<Iface, Impl(Arg0*, Arg1*, Arg2*,
                                    Arg3*)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>(),
                            manager->create<Arg3>());
        }
    };

    template<class Iface, class Impl, class Arg0, class Arg1, class Arg2,
                                      class Arg3, class Arg4>
    class FactoryHelper<Iface, Impl(Arg0*, Arg1*, Arg2*,
                                    Arg3*, Arg4*)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>(),
                            manager->create<Arg3>(),
                            manager->create<Arg4>());
        }
    };

    template<class Iface, class Impl, class Arg0, class Arg1, class Arg2,
                                      class Arg3, class Arg4, class Arg5>
    class FactoryHelper<Iface, Impl(Arg0*, Arg1*, Arg2*,
                                    Arg3*, Arg4*, Arg5*)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>(),
                            manager->create<Arg3>(),
                            manager->create<Arg4>(),
                            manager->create<Arg5>());
        }
    };

    template<class Iface, class Impl, class Arg0, class Arg1, class Arg2,
                                      class Arg3, class Arg4, class Arg5,
                                      class Arg6>
    class FactoryHelper<Iface, Impl(Arg0*, Arg1*, Arg2*,
                                    Arg3*, Arg4*, Arg5*,
                                    Arg6*)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>(),
                            manager->create<Arg3>(),
                            manager->create<Arg4>(),
                            manager->create<Arg5>(),
                            manager->create<Arg6>());
        }
    };

    template<class Iface, class Impl, class Arg0, class Arg1, class Arg2,
                                      class Arg3, class Arg4, class Arg5,
                                      class Arg6, class Arg7>
    class FactoryHelper<Iface, Impl(Arg0*, Arg1*, Arg2*,
                                    Arg3*, Arg4*, Arg5*,
                                    Arg6*, Arg7*)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>(),
                            manager->create<Arg3>(),
                            manager->create<Arg4>(),
                            manager->create<Arg5>(),
                            manager->create<Arg6>(),
                            manager->create<Arg7>());
        }
    };

    template<class Iface, class Impl, class Arg0, class Arg1, class Arg2,
                                      class Arg3, class Arg4, class Arg5,
                                      class Arg6, class Arg7, class Arg8>
    class FactoryHelper<Iface, Impl(Arg0*, Arg1*, Arg2*,
                                    Arg3*, Arg4*, Arg5*,
                                    Arg6*, Arg7*, Arg8*)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>(),
                            manager->create<Arg3>(),
                            manager->create<Arg4>(),
                            manager->create<Arg5>(),
                            manager->create<Arg6>(),
                            manager->create<Arg7>(),
                            manager->create<Arg8>());
        }
    };

    template<class Iface, class Impl, class Arg0, class Arg1, class Arg2,
                                      class Arg3, class Arg4, class Arg5,
                                      class Arg6, class Arg7, class Arg8,
                                      class Arg9>
    class FactoryHelper<Iface, Impl(Arg0*, Arg1*, Arg2*,
                                    Arg3*, Arg4*, Arg5*,
                                    Arg6*, Arg7*, Arg8*,
                                    Arg9*)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>(),
                            manager->create<Arg3>(),
                            manager->create<Arg4>(),
                            manager->create<Arg5>(),
                            manager->create<Arg6>(),
                            manager->create<Arg7>(),
                            manager->create<Arg8>(),
                            manager->create<Arg9>());
        }
    };

    template<class Iface, class Impl, class Arg0, class Arg1, class Arg2,
                                      class Arg3, class Arg4, class Arg5,
                                      class Arg6, class Arg7, class Arg8,
                                      class Arg9, class Arg10>
    class FactoryHelper<Iface, Impl(Arg0*, Arg1*, Arg2*,
                                    Arg3*, Arg4*, Arg5*,
                                    Arg6*, Arg7*, Arg8*,
                                    Arg9*, Arg10*)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>(),
                            manager->create<Arg3>(),
                            manager->create<Arg4>(),
                            manager->create<Arg5>(),
                            manager->create<Arg6>(),
                            manager->create<Arg7>(),
                            manager->create<Arg8>(),
                            manager->create<Arg9>(),
                            manager->create<Arg10>());
        }
    };

    template<class Iface, class Impl, class Arg0, class Arg1, class Arg2,
                                      class Arg3, class Arg4, class Arg5,
                                      class Arg6, class Arg7, class Arg8,
                                      class Arg9, class Arg10, class Arg11>
    class FactoryHelper<Iface, Impl(Arg0*, Arg1*, Arg2*,
                                    Arg3*, Arg4*, Arg5*,
                                    Arg6*, Arg7*, Arg8*,
                                    Arg9*, Arg10*, Arg11*)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>(),
                            manager->create<Arg3>(),
                            manager->create<Arg4>(),
                            manager->create<Arg5>(),
                            manager->create<Arg6>(),
                            manager->create<Arg7>(),
                            manager->create<Arg8>(),
                            manager->create<Arg9>(),
                            manager->create<Arg10>(),
                            manager->create<Arg11>());
        }
    };

    template<class Iface, class Impl, class Arg0, class Arg1, class Arg2,
                                      class Arg3, class Arg4, class Arg5,
                                      class Arg6, class Arg7, class Arg8,
                                      class Arg9, class Arg10, class Arg11,
                                      class Arg12>
    class FactoryHelper<Iface, Impl(Arg0*, Arg1*, Arg2*,
                                    Arg3*, Arg4*, Arg5*,
                                    Arg6*, Arg7*, Arg8*,
                                    Arg9*, Arg10*, Arg11*,
                                    Arg12*)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>(),
                            manager->create<Arg3>(),
                            manager->create<Arg4>(),
                            manager->create<Arg5>(),
                            manager->create<Arg6>(),
                            manager->create<Arg7>(),
                            manager->create<Arg8>(),
                            manager->create<Arg9>(),
                            manager->create<Arg10>(),
                            manager->create<Arg11>(),
                            manager->create<Arg12>());
        }
    };

    template<class Iface, class Impl, class Arg0, class Arg1, class Arg2,
                                      class Arg3, class Arg4, class Arg5,
                                      class Arg6, class Arg7, class Arg8,
                                      class Arg9, class Arg10, class Arg11,
                                      class Arg12, class Arg13>
    class FactoryHelper<Iface, Impl(Arg0*, Arg1*, Arg2*,
                                    Arg3*, Arg4*, Arg5*,
                                    Arg6*, Arg7*, Arg8*,
                                    Arg9*, Arg10*, Arg11*,
                                    Arg12*, Arg13*)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>(),
                            manager->create<Arg3>(),
                            manager->create<Arg4>(),
                            manager->create<Arg5>(),
                            manager->create<Arg6>(),
                            manager->create<Arg7>(),
                            manager->create<Arg8>(),
                            manager->create<Arg9>(),
                            manager->create<Arg10>(),
                            manager->create<Arg11>(),
                            manager->create<Arg12>(),
                            manager->create<Arg13>());
        }
    };

    template<class Iface, class Impl, class Arg0, class Arg1, class Arg2,
                                      class Arg3, class Arg4, class Arg5,
                                      class Arg6, class Arg7, class Arg8,
                                      class Arg9, class Arg10, class Arg11,
                                      class Arg12, class Arg13, class Arg14>
    class FactoryHelper<Iface, Impl(Arg0*, Arg1*, Arg2*,
                                    Arg3*, Arg4*, Arg5*,
                                    Arg6*, Arg7*, Arg8*,
                                    Arg9*, Arg10*, Arg11*,
                                    Arg12*, Arg13*, Arg14*)>
    {
    public:
        static Iface *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>(),
                            manager->create<Arg3>(),
                            manager->create<Arg4>(),
                            manager->create<Arg5>(),
                            manager->create<Arg6>(),
                            manager->create<Arg7>(),
                            manager->create<Arg8>(),
                            manager->create<Arg9>(),
                            manager->create<Arg10>(),
                            manager->create<Arg11>(),
                            manager->create<Arg12>(),
                            manager->create<Arg13>(),
                            manager->create<Arg14>());
        }
    };
#endif
};

}

#endif // UTILS_DEPENDENCYMANAGER_H
