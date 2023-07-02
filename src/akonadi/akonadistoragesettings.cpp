/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <chrigi_1@fastmail.fm>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "akonadistoragesettings.h"

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

using namespace Akonadi;

StorageSettings::StorageSettings()
    : QObject()
{
}

StorageSettings &StorageSettings::instance()
{
    static StorageSettings i;
    return i;
}

Collection StorageSettings::defaultCollection()
{
    KConfigGroup config(KSharedConfig::openConfig(), "General");
    Collection::Id id = config.readEntry("defaultCollection", -1);
    return Collection(id);
}

void StorageSettings::setDefaultCollection(const Collection &collection)
{
    if (defaultCollection() == collection)
        return;

    KConfigGroup config(KSharedConfig::openConfig(), "General");
    config.writeEntry("defaultCollection", QString::number(collection.id()));
    config.sync();
    emit defaultCollectionChanged(collection);
}

#include "moc_akonadistoragesettings.cpp"
