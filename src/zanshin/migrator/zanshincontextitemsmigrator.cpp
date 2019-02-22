/* This file is part of Zanshin

   Copyright 2019 David Faure <faure@kde.org>

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

#include "zanshincontextitemsmigrator.h"
#include <akonadi/akonadicollectionfetchjobinterface.h>
#include <akonadi/akonadiitemfetchjobinterface.h>

#include <AkonadiCore/ItemCreateJob>
#include <AkonadiCore/ItemModifyJob>
#include <AkonadiCore/TagFetchJob>

#include <QStringList>
#include <KCalCore/Todo>

static const char s_contextTagType[] = "Zanshin-Context";

ZanshinContextItemsMigrator::ZanshinContextItemsMigrator()
{
}

ZanshinContextItemsMigrator::FetchResult ZanshinContextItemsMigrator::fetchAllItems(WhichItems which)
{
    FetchResult result;

    auto collectionsJob = m_storage.fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive);
    collectionsJob->kjob()->exec();

    auto collections = collectionsJob->collections();
    foreach (const auto &collection, collections) {
        auto job = m_storage.fetchItemsWithTags(collection);
        job->kjob()->exec();
        bool hasTaskToConvert = false;
        auto items = job->items();
        Akonadi::Item::List selectedItems;
        foreach (const Akonadi::Item &item, items) {
            if (item.hasPayload<KCalCore::Todo::Ptr>()) {
                auto todo = item.payload<KCalCore::Todo::Ptr>();
                if (which == WhichItems::TasksToConvert && !todo->customProperty("Zanshin", "ContextList").isEmpty()) {
                    // This folder was already migrated, skip it
                    hasTaskToConvert = false;
                    qDebug() << "Detected an already converted task" << todo->uid();
                    break;
                }
                const bool isContext = !todo->customProperty("Zanshin", "Context").isEmpty();
                if ((isContext && which == WhichItems::OnlyContexts) ||
                        (!isContext && which == WhichItems::TasksToConvert) ||
                        (!isContext && which == WhichItems::AllTasks)) {
                    selectedItems.push_back(item);
                    hasTaskToConvert = true;
                }
            }
        }
        if (hasTaskToConvert) {
            result.pickedCollection = collection;
            result.items += selectedItems;
        }
    }
    if (!result.pickedCollection.isValid())
        result.pickedCollection = collections.first();

    return result;
}

Akonadi::Tag::List ZanshinContextItemsMigrator::fetchAllTags()
{
    Akonadi::Tag::List tags;
    Akonadi::TagFetchJob job;
    job.exec();
    const auto allTags = job.tags();
    std::copy_if(allTags.constBegin(), allTags.constEnd(), std::back_inserter(tags),
                 [](const Akonadi::Tag &tag) { return tag.type() == s_contextTagType; });
    return tags;
}

void ZanshinContextItemsMigrator::createContexts(const Akonadi::Tag::List &contextTags, const Akonadi::Collection &collection)
{
    int count = 0;
    for (const auto &tag : contextTags) {

        auto context = Domain::Context::Ptr::create();
        context->setName(tag.name());
        Akonadi::Item item = m_serializer.createItemFromContext(context);
        item.setParentCollection(collection);
        auto job = new Akonadi::ItemCreateJob(item, collection);
        if (job->exec()) {
            ++count;
            m_tagUids.insert(tag.id(), job->item().payload<KCalCore::Todo::Ptr>()->uid());
        } else {
            qWarning() << "Failure to create context:" << job->errorString();
        }
    }
    qDebug() << "Created" << count << "contexts in collection" << collection.name();
}

void ZanshinContextItemsMigrator::associateContexts(Akonadi::Item::List& items)
{
    int count = 0;
    for (auto &item : items) {
        const auto allTags = item.tags();
        for (const auto &tag : allTags) {
            if (tag.type() == s_contextTagType) {
                auto context = Domain::Context::Ptr::create();
                context->setName(tag.name());
                const auto tagUid = m_tagUids.value(tag.id());
                if (tagUid.isEmpty())
                    qWarning() << "Item" << item.id() << "uses unknown tag" << tag.id() << tag.name();
                context->setProperty("todoUid", tagUid);
                m_serializer.addContextToTask(context, item);
                auto job = new Akonadi::ItemModifyJob(item);
                if (job->exec()) {
                    item = job->item();
                    ++count;
                } else {
                    qWarning() << "Failure to associate context" << tag.name() << "to task:" << job->errorString();
                }
            }
        }
    }
    qDebug() << "Associated contexts to" << count << "items";
}

bool ZanshinContextItemsMigrator::migrateTags()
{
    auto result = fetchAllItems(WhichItems::TasksToConvert);
    auto tags = fetchAllTags();
    createContexts(tags, result.pickedCollection);
    associateContexts(result.items);
    return true;
}

