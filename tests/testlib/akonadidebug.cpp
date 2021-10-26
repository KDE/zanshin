/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include "akonadidebug.h"

#include <KCalCore/Todo>

#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"

void TestLib::AkonadiDebug::dumpTree(const Akonadi::StorageInterface::Ptr &storage)
{
    auto colJob = storage->fetchCollections(Akonadi::Collection::root(),
                                            Akonadi::StorageInterface::Recursive,
                                            nullptr);
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

