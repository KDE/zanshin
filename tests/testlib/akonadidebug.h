/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADIDEBUG_H
#define AKONADIDEBUG_H

#include "akonadi/akonadistorageinterface.h"

namespace TestLib {
    namespace AkonadiDebug {
        void dumpTree(const Akonadi::StorageInterface::Ptr &storage);
    }
}

#endif // AKONADIDEBUG_H
