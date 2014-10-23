/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Franck Arrecot <franck.arrecot@gmail.com>
   Copyright 2014 Rémi Benoit <r3m1.benoit@gmail.com>

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

#include "akonadicontextrepository.h"

#include "akonadiitemfetchjobinterface.h"
#include "akonadiserializer.h"
#include "akonadistorage.h"

#include "utils/compositejob.h"

using namespace Akonadi;

ContextRepository::ContextRepository(QObject *parent) :
    QObject(parent),
    m_storage(new Storage),
    m_serializer(new Serializer),
    m_ownInterfaces(true)
{
}

ContextRepository::ContextRepository(StorageInterface *storage, SerializerInterface *serializer):
    m_storage(storage),
    m_serializer(serializer),
    m_ownInterfaces(false)
{

}

ContextRepository::~ContextRepository()
{
    if (m_ownInterfaces) {
        delete m_storage;
        delete m_serializer;
    }
}

KJob *ContextRepository::create(Domain::Context::Ptr context)
{
    auto tag = m_serializer->createTagFromContext(context);
    Q_ASSERT(!tag.isValid());
    return m_storage->createTag(tag);
}

KJob *ContextRepository::update(Domain::Context::Ptr context)
{
    auto tag = m_serializer->createTagFromContext(context);
    Q_ASSERT(tag.isValid());
    return m_storage->updateTag(tag);
}

KJob *ContextRepository::remove(Domain::Context::Ptr context)
{
    auto tag = m_serializer->createTagFromContext(context);
    Q_ASSERT(tag.isValid());
    return m_storage->removeTag(tag);
}

KJob *ContextRepository::associate(Domain::Context::Ptr parent, Domain::Task::Ptr child)
{
    Item childItem;

    childItem = m_serializer->createItemFromTask(child);
    Q_ASSERT(childItem.isValid());

    auto job = new Utils::CompositeJob();
    ItemFetchJobInterface *fetchItemJob = m_storage->fetchItem(childItem);
    job->install(fetchItemJob->kjob(), [fetchItemJob, parent, job, this] {
        if (fetchItemJob->kjob()->error() != KJob::NoError)
            return;

        Q_ASSERT(fetchItemJob->items().size() == 1);
        auto childItem = fetchItemJob->items().first();
        auto tag = m_serializer->createTagFromContext(parent);
        Q_ASSERT(tag.isValid());
        childItem.setTag(tag);

        auto updateJob = m_storage->updateItem(childItem);
        job->addSubjob(updateJob);
        updateJob->start();
    });
    return job;
}


KJob *ContextRepository::dissociate(Domain::Context::Ptr parent, Domain::Task::Ptr child)
{
    Item childItem;

    childItem = m_serializer->createItemFromTask(child);
    Q_ASSERT(childItem.isValid());
    auto job = new Utils::CompositeJob();
    ItemFetchJobInterface *fetchItemJob = m_storage->fetchItem(childItem);
    job->install(fetchItemJob->kjob(), [fetchItemJob, parent, job, this] {
        if (fetchItemJob->kjob()->error() != KJob::NoError)
            return;

        Q_ASSERT(fetchItemJob->items().size() == 1);
        auto childItem = fetchItemJob->items().first();
        auto tag = m_serializer->createTagFromContext(parent);
        Q_ASSERT(tag.isValid());
        childItem.clearTag(tag);

        auto updateJob = m_storage->updateItem(childItem);
        job->addSubjob(updateJob);
        updateJob->start();
    });

    return job;
}

KJob *ContextRepository::dissociateAll(Domain::Task::Ptr child)
{
    Item childItem;

    childItem = m_serializer->createItemFromTask(child);
    Q_ASSERT(childItem.isValid());
    auto job = new Utils::CompositeJob();
    ItemFetchJobInterface *fetchItemJob = m_storage->fetchItem(childItem);
    job->install(fetchItemJob->kjob(), [fetchItemJob, job, this] {
        if (fetchItemJob->kjob()->error() != KJob::NoError)
            return;

        Q_ASSERT(fetchItemJob->items().size() == 1);
        auto childItem = fetchItemJob->items().first();
        childItem.clearTags();

        auto updateJob = m_storage->updateItem(childItem);
        job->addSubjob(updateJob);
        updateJob->start();
    });

    return job;
}
