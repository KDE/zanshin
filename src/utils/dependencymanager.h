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
#include <QMap>
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
#else
        typedef Iface *(*FactoryType)(DependencyManager*);
#endif

        Provider()
            : m_factory(0)
        {
        }

        Provider(FactoryType factory)
            : m_factory(factory)
        {
        }

        Provider(const Provider &other)
            : m_factory(other.m_factory)
        {
        }

        Provider &operator=(const Provider &other)
        {
            Provider tmp(other);
            std::swap(m_factory, tmp.m_factory);
            return *this;
        }

        Iface *operator()(DependencyManager *deps) const
        {
            Q_ASSERT(m_factory != 0);
            return m_factory(deps);
        }

    private:
        FactoryType m_factory;
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
            return QSharedPointer<Iface>(s_providers.value(manager)(manager));
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

        static QMap< DependencyManager*, Provider<Iface> > s_providers;
    };

    template<class Iface>
    QMap< DependencyManager*, Provider<Iface> > Supplier<Iface>::s_providers;
}

class DependencyManager
{
public:
    static DependencyManager &globalInstance();

    DependencyManager();
    ~DependencyManager();

    template<class Iface>
    void add(const typename Internal::Provider<Iface>::FactoryType &factory)
    {
        Internal::Provider<Iface> provider(factory);
        Internal::Supplier<Iface>::setProvider(this, provider);
        m_cleanupFunctions << Internal::Supplier<Iface>::removeProvider;
    }

    template<class Iface, class Signature>
    void add()
    {
        add<Iface>(DependencyManager::FactoryHelper<Signature>::create);
    }

    template<class Iface>
    QSharedPointer<Iface> create()
    {
        return Internal::Supplier<Iface>::create(this);
    }

private:
    QList<void (*)(DependencyManager*)> m_cleanupFunctions;

    template<class Impl>
    class FactoryHelper
    {
    public:
        static Impl *create(DependencyManager*)
        {
            return new Impl;
        }
    };

#ifdef Q_COMPILER_VARIADIC_TEMPLATES
    template<class Impl, class... Args>
    class FactoryHelper<Impl(Args*...)>
    {
    public:
        static Impl *create(DependencyManager *manager)
        {
            return new Impl((manager->create<Args>())...);
        }
    };
#else
    template<class Impl, class Arg0>
    class FactoryHelper<Impl(Arg0*)>
    {
    public:
        static Impl *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>());
        }
    };

    template<class Impl, class Arg0, class Arg1>
    class FactoryHelper<Impl(Arg0*, Arg1*)>
    {
    public:
        static Impl *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>());
        }
    };

    template<class Impl, class Arg0, class Arg1, class Arg2>
    class FactoryHelper<Impl(Arg0*, Arg1*, Arg2*)>
    {
    public:
        static Impl *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>());
        }
    };

    template<class Impl, class Arg0, class Arg1, class Arg2, class Arg3>
    class FactoryHelper<Impl(Arg0*, Arg1*, Arg2*, Arg3*)>
    {
    public:
        static Impl *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>(),
                            manager->create<Arg3>());
        }
    };

    template<class Impl, class Arg0, class Arg1, class Arg2, class Arg3,
             class Arg4>
    class FactoryHelper<Impl(Arg0*, Arg1*, Arg2*, Arg3*,
                             Arg4*)>
    {
    public:
        static Impl *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>(),
                            manager->create<Arg3>(),
                            manager->create<Arg4>());
        }
    };

    template<class Impl, class Arg0, class Arg1, class Arg2, class Arg3,
             class Arg4, class Arg5>
    class FactoryHelper<Impl(Arg0*, Arg1*, Arg2*, Arg3*,
                             Arg4*, Arg5*)>
    {
    public:
        static Impl *create(DependencyManager *manager)
        {
            return new Impl(manager->create<Arg0>(),
                            manager->create<Arg1>(),
                            manager->create<Arg2>(),
                            manager->create<Arg3>(),
                            manager->create<Arg4>(),
                            manager->create<Arg5>());
        }
    };

    template<class Impl, class Arg0, class Arg1, class Arg2, class Arg3,
             class Arg4, class Arg5, class Arg6>
    class FactoryHelper<Impl(Arg0*, Arg1*, Arg2*, Arg3*,
                             Arg4*, Arg5*, Arg6*)>
    {
    public:
        static Impl *create(DependencyManager *manager)
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

    template<class Impl, class Arg0, class Arg1, class Arg2, class Arg3,
             class Arg4, class Arg5, class Arg6, class Arg7>
    class FactoryHelper<Impl(Arg0*, Arg1*, Arg2*, Arg3*,
                             Arg4*, Arg5*, Arg6*, Arg7*)>
    {
    public:
        static Impl *create(DependencyManager *manager)
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

    template<class Impl, class Arg0, class Arg1, class Arg2, class Arg3,
             class Arg4, class Arg5, class Arg6, class Arg7, class Arg8>
    class FactoryHelper<Impl(Arg0*, Arg1*, Arg2*, Arg3*,
                             Arg4*, Arg5*, Arg6*, Arg7*,
                             Arg8*)>
    {
    public:
        static Impl *create(DependencyManager *manager)
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

    template<class Impl, class Arg0, class Arg1, class Arg2, class Arg3,
             class Arg4, class Arg5, class Arg6, class Arg7, class Arg8,
             class Arg9>
    class FactoryHelper<Impl(Arg0*, Arg1*, Arg2*, Arg3*,
                             Arg4*, Arg5*, Arg6*, Arg7*,
                             Arg8*, Arg9*)>
    {
    public:
        static Impl *create(DependencyManager *manager)
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
#endif
};

}

#endif // UTILS_DEPENDENCYMANAGER_H
