/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "akonadiitemfetchjobinterface.h"

#include <KJob>

using namespace Akonadi;

ItemFetchJobInterface::ItemFetchJobInterface()
{
}

ItemFetchJobInterface::~ItemFetchJobInterface()
{
}

KJob *ItemFetchJobInterface::kjob()
{
    KJob *job = dynamic_cast<KJob*>(this);
    Q_ASSERT(job);
    return job;
}
