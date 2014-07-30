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

#include <Akonadi/Item>

#include "akonadicollectionfetchjobinterface.h"
#include "akonadiitemfetchjobinterface.h"
#include "akonadiserializer.h"
#include "akonadistorage.h"
#include "akonadistoragesettings.h"

#include "utils/compositejob.h"

using namespace Akonadi;
using namespace Utils;

TaskRepository::TaskRepository(QObject *parent)
    : QObject(parent),
      m_storage(new Storage),
      m_serializer(new Serializer),
      m_ownInterfaces(true)
{
}

TaskRepository::TaskRepository(StorageInterface *storage, SerializerInterface *serializer)
    : m_storage(storage),
      m_serializer(serializer),
      m_ownInterfaces(false)
{
}

TaskRepository::~TaskRepository()
{
    if (m_ownInterfaces) {
        delete m_storage;
        delete m_serializer;
    }
}

bool TaskRepository::isDefaultSource(Domain::DataSource::Ptr source) const
{
    auto settingsCollection = StorageSettings::instance().defaultTaskCollection();
    auto sourceCollection = m_serializer->createCollectionFromDataSource(source);
    return settingsCollection == sourceCollection;
}

void TaskRepository::setDefaultSource(Domain::DataSource::Ptr source)
{
    auto collection = m_serializer->createCollectionFromDataSource(source);
    StorageSettings::instance().setDefaultTaskCollection(collection);
}

KJob *TaskRepository::save(Domain::Task::Ptr task)
{
    auto item = m_serializer->createItemFromTask(task);

    if (task->property("itemId").isValid()) {
        item.setId(task->property("itemId").toLongLong());
        return m_storage->updateItem(item);
    } else {
        const Akonadi::Collection defaultCollection = m_storage->defaultTaskCollection();
        if (defaultCollection.isValid()) {
            return m_storage->createItem(item, defaultCollection);
        } else {
            auto job = new CompositeJob();
            CollectionFetchJobInterface *fetchCollectionJob = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                                          StorageInterface::Recursive,
                                                                                          StorageInterface::Tasks);
            job->install(fetchCollectionJob->kjob(), [fetchCollectionJob, item, job, this] {
                if (fetchCollectionJob->kjob()->error() != KJob::NoError)
                    return;

                Q_ASSERT(fetchCollectionJob->collections().size() > 0);
                const Akonadi::Collection::List collections = fetchCollectionJob->collections();
                Akonadi::Collection col = *std::find_if(collections.constBegin(), collections.constEnd(),
                                                        [] (const Akonadi::Collection &c) {
                                                            return c.rights() == Akonadi::Collection::AllRights;
                                                        });
                Q_ASSERT(col.isValid());
                auto createJob = m_storage->createItem(item, col);
                job->addSubjob(createJob);
                createJob->start();
            });
            return job;
        }
    }
}

KJob *TaskRepository::remove(Domain::Task::Ptr task)
{
    auto item = m_serializer->createItemFromTask(task);
    Q_ASSERT(item.isValid());
    return m_storage->removeItem(item);
}

KJob *TaskRepository::associate(Domain::Task::Ptr parent, Domain::Task::Ptr child)
{
    auto childItem = m_serializer->createItemFromTask(child);

    auto job = new CompositeJob();
    ItemFetchJobInterface *fetchItemJob = m_storage->fetchItem(childItem);
    job->install(fetchItemJob->kjob(), [fetchItemJob, parent, job, this] {
        if (fetchItemJob->kjob()->error() != KJob::NoError)
           return;

        Q_ASSERT(fetchItemJob->items().size() == 1);
        auto childItem = fetchItemJob->items().first();
        m_serializer->updateItemParent(childItem, parent);

        // Check collections to know if we need to move child
        auto parentItem = m_serializer->createItemFromTask(parent);
        ItemFetchJobInterface *fetchParentItemJob = m_storage->fetchItem(parentItem);
        job->install(fetchParentItemJob->kjob(), [fetchParentItemJob, childItem, job, this] {
            if (fetchParentItemJob->kjob()->error() != KJob::NoError)
                return;

            Q_ASSERT(fetchParentItemJob->items().size() == 1);
            auto parentItem = fetchParentItemJob->items().first();

            const int itemCollectionId = childItem.parentCollection().id();
            const int parentCollectionId = parentItem.parentCollection().id();

            if (itemCollectionId != parentCollectionId) {
                ItemFetchJobInterface *fetchChildrenItemJob = m_storage->fetchItems(childItem.parentCollection());
                job->install(fetchChildrenItemJob->kjob(), [fetchChildrenItemJob, childItem, parentItem, job, this] {
                    if (fetchChildrenItemJob->kjob()->error() != KJob::NoError)
                        return;

                    Item::List childItems = m_serializer->filterDescendantItems(fetchChildrenItemJob->items(), childItem);

                    auto transaction = m_storage->createTransaction();
                    m_storage->updateItem(childItem, transaction);
                    childItems.push_front(childItem);
                    m_storage->moveItems(childItems, parentItem.parentCollection(), transaction);
                    job->addSubjob(transaction);
                    transaction->start();
                });
            } else {
                auto updateJob = m_storage->updateItem(childItem);
                job->addSubjob(updateJob);
                updateJob->start();
            }
        });
    });

    return job;
}

KJob *TaskRepository::dissociate(Domain::Task::Ptr parent, Domain::Task::Ptr child)
{
    auto job = new CompositeJob();
    auto parentItem = m_serializer->createItemFromTask(parent);
    ItemFetchJobInterface *fetchParentItemJob = m_storage->fetchItem(parentItem);
    job->install(fetchParentItemJob->kjob(), [fetchParentItemJob, child, job, this] {
        if (fetchParentItemJob->kjob()->error() != KJob::NoError)
            return;

        Q_ASSERT(fetchParentItemJob->items().size() == 1);
        auto parentItem = fetchParentItemJob->items().first();

        Q_ASSERT(m_serializer->isTaskChild(child, parentItem));

        auto childItem = m_serializer->createItemFromTask(child);
        ItemFetchJobInterface *fetchItemJob = m_storage->fetchItem(childItem);
        job->install(fetchItemJob->kjob(), [fetchItemJob, job, this] {
            if (fetchItemJob->kjob()->error() != KJob::NoError)
                return;

            Q_ASSERT(fetchItemJob->items().size() == 1);
            auto childItem = fetchItemJob->items().first();

            m_serializer->removeItemParent(childItem);

            auto updateJob = m_storage->updateItem(childItem);
            job->addSubjob(updateJob);
            updateJob->start();
        });
    });

    return job;
}
