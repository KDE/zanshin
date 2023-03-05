/*
 * SPDX-FileCopyrightText: 2014 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "zanshin021migrator.h"
#include <akonadi/akonadicollectionfetchjobinterface.h>
#include <akonadi/akonadiitemfetchjobinterface.h>
#include <akonadi/akonadiserializer.h>

#include <Akonadi/TransactionSequence>
#include <Akonadi/ItemModifyJob>

#include <QStringList>
#include <KCalendarCore/Todo>

using Akonadi::Serializer;

Zanshin021Migrator::Zanshin021Migrator()
{

}

bool Zanshin021Migrator::isProject(const Akonadi::Item& item)
{
    // same as Serializer::isProject, but we don't need the serializer here...
    return item.hasPayload<KCalendarCore::Todo::Ptr>() && !item.payload<KCalendarCore::Todo::Ptr>()->customProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsProject()).isEmpty();
}


Zanshin021Migrator::SeenItemHash Zanshin021Migrator::fetchAllItems()
{
    SeenItemHash hash;

    auto collectionsJob = m_storage.fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, nullptr);
    collectionsJob->kjob()->exec();

    auto collections = collectionsJob->collections();
    foreach (const auto &collection, collections) {
        auto job = m_storage.fetchItems(collection, nullptr);
        job->kjob()->exec();
        auto items = job->items();
        foreach (const Akonadi::Item &item, items) {
            if (item.hasPayload<KCalendarCore::Todo::Ptr>()) {
                auto todo = item.payload<KCalendarCore::Todo::Ptr>();
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
        auto todo = item.payload<KCalendarCore::Todo::Ptr>();
        todo->setCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsProject(), QStringLiteral("1"));
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
        auto todo = item.payload<KCalendarCore::Todo::Ptr>();
        if (todo->comments().contains(QStringLiteral("X-Zanshin-Project")))
            markAsProject(seenItem, sequence);
    }
}

void Zanshin021Migrator::migrateProjectWithChildren(Zanshin021Migrator::SeenItemHash& items, Akonadi::TransactionSequence* sequence)
{
    for (SeenItemHash::iterator it = items.begin(); it != items.end(); ++it) {
        const SeenItem &seenItem = it.value();
        const auto todo = seenItem.item().payload<KCalendarCore::Todo::Ptr>();
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

