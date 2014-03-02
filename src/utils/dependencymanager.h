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

#include <utility>

namespace Utils {

class DependencyManager;

namespace Internal {
    template<class Iface, class Impl>
    Iface *standardFactory()
    {
        return new Impl;
    }

    template<class Iface>
    class Provider
    {
    public:
        typedef Iface *(*FactoryType)();

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

        Iface *operator()() const
        {
            Q_ASSERT(m_factory != 0);
            return m_factory();
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

        static Iface *create(DependencyManager *manager)
        {
            return s_providers.value(manager)();
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

    template<class Iface, class Impl>
    void add()
    {
        Internal::Provider<Iface> provider(Internal::standardFactory<Iface, Impl>);
        Internal::Supplier<Iface>::setProvider(this, provider);
        m_cleanupFunctions << Internal::Supplier<Iface>::removeProvider;
    }

    template<class Iface>
    Iface *create()
    {
        return Internal::Supplier<Iface>::create(this);
    }

private:
    QList<void (*)(DependencyManager*)> m_cleanupFunctions;
};

}

#endif // UTILS_DEPENDENCYMANAGER_H
