/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "akonadiprojectrepository.h"

#include "akonadiitemfetchjobinterface.h"

#include "utils/compositejob.h"

using namespace Akonadi;

ProjectRepository::ProjectRepository(const StorageInterface::Ptr &storage,
                                     const SerializerInterface::Ptr &serializer)
    : m_storage(storage),
      m_serializer(serializer)
{
}

KJob *ProjectRepository::create(Domain::Project::Ptr project, Domain::DataSource::Ptr source)
{
    auto item = m_serializer->createItemFromProject(project);
    Q_ASSERT(!item.isValid());
    auto collection = m_serializer->createCollectionFromDataSource(source);
    Q_ASSERT(collection.isValid());
    return m_storage->createItem(item, collection);
}

KJob *ProjectRepository::update(Domain::Project::Ptr project)
{
    auto item = m_serializer->createItemFromProject(project);
    Q_ASSERT(item.isValid());
    return m_storage->updateItem(item, this);
}

KJob *ProjectRepository::remove(Domain::Project::Ptr project)
{
    auto item = m_serializer->createItemFromProject(project);
    Q_ASSERT(item.isValid());
    return m_storage->removeItem(item, this);
}

KJob *ProjectRepository::associate(Domain::Project::Ptr parent, Domain::Task::Ptr child)
{
    Item childItem = m_serializer->createItemFromTask(child);
    Q_ASSERT(childItem.isValid());

    auto job = new Utils::CompositeJob();
    ItemFetchJobInterface *fetchItemJob = m_storage->fetchItem(childItem, this);
    job->install(fetchItemJob->kjob(), [fetchItemJob, parent, child, job, this] {
        if (fetchItemJob->kjob()->error() != KJob::NoError)
           return;

        Q_ASSERT(fetchItemJob->items().size() == 1);
        auto childItem = fetchItemJob->items().at(0);
        m_serializer->updateItemProject(childItem, parent);

        // Check collections to know if we need to move child
        auto parentItem = m_serializer->createItemFromProject(parent);
        ItemFetchJobInterface *fetchParentItemJob = m_storage->fetchItem(parentItem, this);
        job->install(fetchParentItemJob->kjob(), [fetchParentItemJob, child, childItem, job, this] {
            if (fetchParentItemJob->kjob()->error() != KJob::NoError)
                return;

            Q_ASSERT(fetchParentItemJob->items().size() == 1);
            auto parentItem = fetchParentItemJob->items().at(0);

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

KJob *ProjectRepository::dissociate(Domain::Task::Ptr child)
{
    auto job = new Utils::CompositeJob();
    const auto childItem = m_serializer->createItemFromTask(child);
    Q_ASSERT(childItem.isValid());

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

#include "moc_akonadiprojectrepository.cpp"
