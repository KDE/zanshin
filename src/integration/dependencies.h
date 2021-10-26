/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef INTEGRATION_DEPENDENCIES_H
#define INTEGRATION_DEPENDENCIES_H

namespace Utils
{
    class DependencyManager;
}

namespace Integration
{
    void initializeGlobalAppDependencies();

    void initializeDefaultAkonadiDependencies(Utils::DependencyManager &deps);
    void initializeDefaultDomainDependencies(Utils::DependencyManager &deps);
    void initializeDefaultPresentationDependencies(Utils::DependencyManager &deps);
}

#endif

