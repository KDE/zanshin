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


#include "akonaditaskrepository.h"

#include <KLocalizedString>

#include <AkonadiCore/Akonadi/Item>

#include "akonadicollectionfetchjobinterface.h"
#include "akonadiitemfetchjobinterface.h"

#include "utils/compositejob.h"

using namespace Akonadi;
using namespace Utils;

TaskRepository::TaskRepository(const StorageInterface::Ptr &storage,
                               const SerializerInterface::Ptr &serializer)
    : m_storage(storage),
      m_serializer(serializer)
{
}

KJob *TaskRepository::createItem(const Item &item)
{
    const Akonadi::Collection defaultCollection = m_storage->defaultCollection();
    if (defaultCollection.isValid()) {
        return m_storage->createItem(item, defaultCollection);
    } else {
        auto job = new CompositeJob();
        CollectionFetchJobInterface *fetchCollectionJob = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                                      StorageInterface::Recursive,
                                                                                      this);
        job->install(fetchCollectionJob->kjob(), [fetchCollectionJob, item, job, this] {
            if (fetchCollectionJob->kjob()->error() != KJob::NoError)
                return;

            Q_ASSERT(fetchCollectionJob->collections().size() > 0);
            const Akonadi::Collection::List collections = fetchCollectionJob->collections();
            auto it = std::find_if(collections.constBegin(), collections.constEnd(),
                                   [] (const Akonadi::Collection &c) {
                return (c.rights() & Akonadi::Collection::CanCreateItem)
                    && (c.rights() & Akonadi::Collection::CanChangeItem)
                    && (c.rights() & Akonadi::Collection::CanDeleteItem);
            });
            if (it == collections.constEnd()) {
                job->emitError(i18n("Could not find a collection to store the task into!"));
            } else {
                auto col = *it;
                Q_ASSERT(col.isValid());
                auto createJob = m_storage->createItem(item, col);
                job->addSubjob(createJob);
                createJob->start();
            }
        });
        return job;
    }
}

KJob *TaskRepository::create(Domain::Task::Ptr task)
{
    auto item = m_serializer->createItemFromTask(task);
    Q_ASSERT(!item.isValid());
    return createItem(item);
}

KJob *TaskRepository::createChild(Domain::Task::Ptr task, Domain::Task::Ptr parent)
{
    Item taskItem = m_serializer->createItemFromTask(task);
    Q_ASSERT(!taskItem.isValid());

    Item parentItem = m_serializer->createItemFromTask(parent);
    Q_ASSERT(parentItem.isValid());
    Q_ASSERT(parentItem.parentCollection().isValid());

    m_serializer->updateItemParent(taskItem, parent);

    return m_storage->createItem(taskItem, parentItem.parentCollection());
}

KJob *TaskRepository::createInProject(Domain::Task::Ptr task, Domain::Project::Ptr project)
{
    Item taskItem = m_serializer->createItemFromTask(task);
    Q_ASSERT(!taskItem.isValid());

    Item projectItem = m_serializer->createItemFromProject(project);
    Q_ASSERT(projectItem.isValid());
    Q_ASSERT(projectItem.parentCollection().isValid());

    m_serializer->updateItemProject(taskItem, project);

    return m_storage->createItem(taskItem, projectItem.parentCollection());
}

KJob *TaskRepository::createInContext(Domain::Task::Ptr task, Domain::Context::Ptr context)
{
    Item item = m_serializer->createItemFromTask(task);
    Q_ASSERT(!item.isValid());

    m_serializer->addContextToTask(context, item);

    return createItem(item);
}

KJob *TaskRepository::update(Domain::Task::Ptr task)
{
    auto item = m_serializer->createItemFromTask(task);
    Q_ASSERT(item.isValid());
    return m_storage->updateItem(item, this);
}

KJob *TaskRepository::remove(Domain::Task::Ptr task)
{
    auto item = m_serializer->createItemFromTask(task);
    Q_ASSERT(item.isValid());

    auto compositeJob = new CompositeJob();
    ItemFetchJobInterface *fetchItemJob = m_storage->fetchItem(item, this);
    compositeJob->install(fetchItemJob->kjob(), [fetchItemJob, compositeJob, this] {
        if (fetchItemJob->kjob()->error() != KJob::NoError)
           return;

        Q_ASSERT(fetchItemJob->items().size() == 1);
        auto item = fetchItemJob->items().at(0);

        ItemFetchJobInterface *fetchCollectionItemsJob = m_storage->fetchItems(item.parentCollection(), this);
        compositeJob->install(fetchCollectionItemsJob->kjob(), [fetchCollectionItemsJob, item, compositeJob, this] {
            if (fetchCollectionItemsJob->kjob()->error() != KJob::NoError)
                return;

            Item::List childItems = m_serializer->filterDescendantItems(fetchCollectionItemsJob->items(), item);
            childItems << item;

            auto removeJob = m_storage->removeItems(childItems, this);
            compositeJob->addSubjob(removeJob);
            removeJob->start();
        });
    });

    return compositeJob;
}

KJob *TaskRepository::promoteToProject(Domain::Task::Ptr task)
{
    auto item = m_serializer->createItemFromTask(task);

    auto job = new CompositeJob();
    auto fetchJob = m_storage->fetchItem(item, this);
    job->install(fetchJob->kjob(), [fetchJob, job, this] {
        if (fetchJob->kjob()->error() != KJob::NoError)
           return;

        Q_ASSERT(fetchJob->items().size() == 1);
        auto item = fetchJob->items().at(0);
        m_serializer->promoteItemToProject(item);

        auto updateJob = m_storage->updateItem(item, this);
        job->addSubjob(updateJob);
        updateJob->start();
    });
    return job;
}

KJob *TaskRepository::associate(Domain::Task::Ptr parent, Domain::Task::Ptr child)
{
    auto childItem = m_serializer->createItemFromTask(child);

    auto job = new CompositeJob();
    ItemFetchJobInterface *fetchItemJob = m_storage->fetchItem(childItem, this);
    job->install(fetchItemJob->kjob(), [fetchItemJob, child, parent, job, this] {
        if (fetchItemJob->kjob()->error() != KJob::NoError)
           return;

        Q_ASSERT(fetchItemJob->items().size() == 1);
        auto childItem = fetchItemJob->items().at(0);
        m_serializer->updateItemParent(childItem, parent);

        // Check collections to know if we need to move child
        auto partialParentItem = m_serializer->createItemFromTask(parent);
        ItemFetchJobInterface *fetchParentItemJob = m_storage->fetchItems(partialParentItem.parentCollection(), this);
        job->install(fetchParentItemJob->kjob(), [child, parent, fetchParentItemJob, partialParentItem, childItem, job, this] {
            if (fetchParentItemJob->kjob()->error() != KJob::NoError)
                return;

            const auto items = fetchParentItemJob->items();
            const auto parentIndex = items.indexOf(partialParentItem);
            Q_ASSERT(parentIndex >= 0);
            const auto parentItem = items.at(parentIndex);

            const auto childUid = m_serializer->itemUid(childItem);
            auto relatedUid = m_serializer->relatedUidFromItem(parentItem);
            while (!relatedUid.isEmpty()) {
                if (relatedUid == childUid) {
                    job->emitError(i18n("Could not associate '%1', it is an ancestor of '%2'",
                                        child->title(),
                                        parent->title()));
                    return;
                }

                auto it = std::find_if(items.constBegin(), items.constEnd(),
                                       [relatedUid, this] (const Akonadi::Item &item) {
                    return m_serializer->itemUid(item) == relatedUid;
                });
                if (it == items.end())
                    break;

                relatedUid = m_serializer->relatedUidFromItem(*it);
            }

            const int itemCollectionId = childItem.parentCollection().id();
            const int parentCollectionId = parentItem.parentCollection().id();

            if (itemCollectionId != parentCollectionId) {
                ItemFetchJobInterface *fetchChildrenItemJob = m_storage->fetchItems(childItem.parentCollection(), this);
                job->install(fetchChildrenItemJob->kjob(), [fetchChildrenItemJob, childItem, parentItem, job, this] {
                    if (fetchChildrenItemJob->kjob()->error() != KJob::NoError)
                        return;

                    Item::List childItems = m_serializer->filterDescendantItems(fetchChildrenItemJob->items(), childItem);

                    auto transaction = m_storage->createTransaction(this);
                    m_storage->updateItem(childItem, transaction);
                    childItems.push_front(childItem);
                    m_storage->moveItems(childItems, parentItem.parentCollection(), transaction);
                    job->addSubjob(transaction);
                    transaction->start();
                });
            } else {
                auto updateJob = m_storage->updateItem(childItem, this);
                job->addSubjob(updateJob);
                updateJob->start();
            }
        });
    });

    return job;
}

KJob *TaskRepository::dissociate(Domain::Task::Ptr child)
{
    auto job = new CompositeJob();
    auto childItem = m_serializer->createItemFromTask(child);
    ItemFetchJobInterface *fetchItemJob = m_storage->fetchItem(childItem, this);
    job->install(fetchItemJob->kjob(), [fetchItemJob, job, this] {
        if (fetchItemJob->kjob()->error() != KJob::NoError)
            return;

        Q_ASSERT(fetchItemJob->items().size() == 1);
        auto childItem = fetchItemJob->items().at(0);

        m_serializer->removeItemParent(childItem);

        auto updateJob = m_storage->updateItem(childItem, this);
        job->addSubjob(updateJob);
        updateJob->start();
    });

    return job;
}

KJob *TaskRepository::dissociateAll(Domain::Task::Ptr child)
{
    auto job = new CompositeJob();
    auto childItem = m_serializer->createItemFromTask(child);
    ItemFetchJobInterface *fetchItemJob = m_storage->fetchItem(childItem, this);
    job->install(fetchItemJob->kjob(), [fetchItemJob, job, this] {
        if (fetchItemJob->kjob()->error() != KJob::NoError)
            return;

        Q_ASSERT(fetchItemJob->items().size() == 1);
        auto childItem = fetchItemJob->items().at(0);

        m_serializer->removeItemParent(childItem);
        m_serializer->clearItem(&childItem);

        auto updateJob = m_storage->updateItem(childItem, this);
        job->addSubjob(updateJob);
        updateJob->start();
    });

    return job;
}
