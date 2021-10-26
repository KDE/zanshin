/*
 * SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "zanshincontextitemsmigrator.h"
#include <akonadi/akonadicollectionfetchjobinterface.h>
#include <akonadi/akonadiitemfetchjobinterface.h>

#include <AkonadiCore/Akonadi/ItemCreateJob>
#include <AkonadiCore/Akonadi/ItemModifyJob>
#include <AkonadiCore/Akonadi/TagFetchJob>
#include <AkonadiCore/Akonadi/ItemDeleteJob>

#include <QStringList>
#include <KCalCore/Todo>

using Akonadi::Serializer;

static const char s_contextTagType[] = "Zanshin-Context";

ZanshinContextItemsMigrator::ZanshinContextItemsMigrator(bool forceMigration)
    : m_forceMigration(forceMigration)
{
}

ZanshinContextItemsMigrator::FetchResult ZanshinContextItemsMigrator::fetchAllItems(WhichItems which)
{
    FetchResult result;

    auto collectionsJob = m_storage.fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, nullptr);
    collectionsJob->kjob()->exec();

    int deletedCount = 0;
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
                if (!m_forceMigration) {
                    if (which == WhichItems::TasksToConvert && !todo->customProperty(Serializer::customPropertyAppName(), Serializer::customPropertyContextList()).isEmpty()) {
                        // This folder was already migrated, skip it
                        hasTaskToConvert = false;
                        qDebug() << "Detected an already converted task" << todo->uid();
                        break;
                    }
                }
                const bool isContext = !todo->customProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsContext()).isEmpty();
                if ((isContext && which == WhichItems::OnlyContexts) ||
                        (!isContext && which == WhichItems::TasksToConvert) ||
                        (!isContext && which == WhichItems::AllTasks)) {
                    selectedItems.push_back(item);
                    hasTaskToConvert = true;
                }
                if (m_forceMigration && isContext && which == WhichItems::TasksToConvert) {
                    ++deletedCount;
                    auto job = new Akonadi::ItemDeleteJob(item);
                    job->exec();
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

    if (deletedCount > 0) {
        qDebug() << "Deleted all" << deletedCount << "contexts, we will recreate them";
    }

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
        m_serializer.clearItem(&item);
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
        // While we're here, port from "Project" to "ISPROJECT"
        auto todo = item.payload<KCalCore::Todo::Ptr>();
        if (!todo->customProperty("Zanshin", "Project").isEmpty()) {
            todo->setCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsProject(), QStringLiteral("1"));
            auto job = new Akonadi::ItemModifyJob(item);
            job->exec();
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

