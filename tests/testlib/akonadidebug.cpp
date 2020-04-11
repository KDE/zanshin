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

#include "akonadidebug.h"

#include <KCalCore/Todo>

#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"

void TestLib::AkonadiDebug::dumpTree(const Akonadi::StorageInterface::Ptr &storage)
{
    auto colJob = storage->fetchCollections(Akonadi::Collection::root(),
                                            Akonadi::StorageInterface::Recursive);
    colJob->kjob()->exec();
    foreach (const auto &col, colJob->collections()) {
        qDebug() << "COL:" << col.id() << col.name() << col.remoteId();
        auto itemJob = storage->fetchItems(col, nullptr);
        itemJob->kjob()->exec();
        foreach (const auto &item, itemJob->items()) {
            QString summary;
            if (item.hasPayload<KCalCore::Todo::Ptr>())
                summary = item.payload<KCalCore::Todo::Ptr>()->summary();
            qDebug() << "\tITEM:" << item.id() << item.remoteId() << summary;
        }
    }
}

