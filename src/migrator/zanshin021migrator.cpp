/* This file is part of Zanshin

   Copyright 2014 David Faure <faure@kde.org>

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

#include "zanshin021migrator.h"
#include <akonadi/akonadicollectionfetchjobinterface.h>
#include <akonadi/akonadiitemfetchjobinterface.h>

#include <Akonadi/TransactionSequence>
#include <Akonadi/ItemModifyJob>

#include <QStringList>
#include <KCalCore/Todo>

Zanshin021Migrator::Zanshin021Migrator()
{

}

bool Zanshin021Migrator::isProject(const Akonadi::Item& item)
{
    // same as Serializer::isProject, but we don't need the serializer here...
    return item.hasPayload<KCalCore::Todo::Ptr>() && !item.payload<KCalCore::Todo::Ptr>()->customProperty("Zanshin", "Project").isEmpty();
}


Zanshin021Migrator::SeenItemHash Zanshin021Migrator::fetchAllItems()
{
    SeenItemHash hash;

    auto collectionsJob = m_storage.fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, Akonadi::StorageInterface::Tasks);
    collectionsJob->kjob()->exec();

    auto collections = collectionsJob->collections();
    for (const auto &collection : collections) {
        auto job = m_storage.fetchItems(collection);
        job->kjob()->exec();
        auto items = job->items();
        for (const Akonadi::Item &item : items) {
            if (item.hasPayload<KCalCore::Todo::Ptr>()) {
                auto todo = item.payload<KCalCore::Todo::Ptr>();
                hash.insert(todo->uid(), SeenItem(item));
            }
        }
    }

    return hash;
}

void Zanshin021Migrator::markAsProject(SeenItem& seenItem, Akonadi::TransactionSequence* sequence)
{
    Akonadi::Item &item = seenItem.item();
    if (!isProject(item)) {
        auto todo = item.payload<KCalCore::Todo::Ptr>();
        todo->setCustomProperty("Zanshin", "Project", "1");
        item.setPayload(todo);
        seenItem.setDirty();
        qDebug() << "Marking as project:" << item.id() << item.remoteId() << todo->summary();
        new Akonadi::ItemModifyJob(item, sequence);
    }
}

void Zanshin021Migrator::migrateProjectComments(Zanshin021Migrator::SeenItemHash& items, Akonadi::TransactionSequence* sequence)
{
    for (SeenItemHash::iterator it = items.begin(); it != items.end(); ++it) {
        SeenItem &seenItem = it.value();
        Akonadi::Item &item = seenItem.item();
        auto todo = item.payload<KCalCore::Todo::Ptr>();
        if (todo->comments().contains("X-Zanshin-Project"))
            markAsProject(seenItem, sequence);
    }
}

void Zanshin021Migrator::migrateProjectWithChildren(Zanshin021Migrator::SeenItemHash& items, Akonadi::TransactionSequence* sequence)
{
    for (SeenItemHash::iterator it = items.begin(); it != items.end(); ++it) {
        const SeenItem &seenItem = it.value();
        const auto todo = seenItem.item().payload<KCalCore::Todo::Ptr>();
        const QString parentUid = todo->relatedTo();
        if (!parentUid.isEmpty()) {
            auto parentIt = items.find(parentUid);
            if (parentIt != items.end())
                markAsProject(*parentIt, sequence);
        }
    }
}

bool Zanshin021Migrator::migrateProjects()
{
    SeenItemHash items = fetchAllItems();
    auto sequence = new Akonadi::TransactionSequence;
    migrateProjectComments(items, sequence);
    migrateProjectWithChildren(items, sequence);
    return sequence->exec();
}

