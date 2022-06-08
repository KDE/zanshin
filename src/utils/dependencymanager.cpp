/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "dependencymanager.h"

using namespace Utils;

Q_GLOBAL_STATIC(DependencyManager, s_globalInstance)

DependencyManager &DependencyManager::globalInstance()
{
    return *s_globalInstance();
}

DependencyManager::DependencyManager()
{
}

DependencyManager::DependencyManager(const DependencyManager &other)
    : m_cleanupFunctions(other.m_cleanupFunctions)
{
}

DependencyManager::~DependencyManager()
{
    for (const auto &cleanupFunction : std::as_const(m_cleanupFunctions)) {
        cleanupFunction(this);
    }
}

DependencyManager &DependencyManager::operator=(const DependencyManager &other)
{
    m_cleanupFunctions = other.m_cleanupFunctions;
    return *this;
}
