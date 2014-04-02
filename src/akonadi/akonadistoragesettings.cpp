/* This file is part of Zanshin Todo.

   Copyright 2012 Christian Mollekopf <chrigi_1@fastmail.fm>

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

#include "akonadistoragesettings.h"

#include <KConfig>
#include <KConfigGroup>
#include <KGlobal>

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

QSet<Entity::Id> StorageSettings::activeCollections()
{
    KConfigGroup config(KGlobal::config(), "General");
    return config.readEntry("activeCollections", QList<Collection::Id>()).toSet();
}

Collection StorageSettings::defaultNoteCollection()
{
    KConfigGroup config(KGlobal::config(), "General");
    Collection::Id id = config.readEntry("defaultNoteCollection", -1);
    return Collection(id);
}

Collection StorageSettings::defaultTaskCollection()
{
    KConfigGroup config(KGlobal::config(), "General");
    Collection::Id id = config.readEntry("defaultCollection", -1);
    return Collection(id);
}

void StorageSettings::setDefaultNoteCollection(const Collection &collection)
{
    if (defaultNoteCollection() == collection)
        return;

    KConfigGroup config(KGlobal::config(), "General");
    config.writeEntry("defaultNoteCollection", QString::number(collection.id()));
    config.sync();
    emit defaultNoteCollectionChanged(collection);
}

void StorageSettings::setDefaultTaskCollection(const Collection &collection)
{
    if (defaultTaskCollection() == collection)
        return;

    KConfigGroup config(KGlobal::config(), "General");
    config.writeEntry("defaultCollection", QString::number(collection.id()));
    config.sync();
    emit defaultTaskCollectionChanged(collection);
}

void StorageSettings::setActiveCollections(const QSet<Entity::Id> &collections)
{
    if (activeCollections() == collections)
        return;

    KConfigGroup config(KGlobal::config(), "General");
    config.writeEntry("activeCollections", collections.toList());
    config.sync();
    emit activeCollectionsChanged(collections);
}
