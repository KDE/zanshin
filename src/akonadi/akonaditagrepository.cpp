/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Franck Arrecot <franck.arrecot@gmail.com>

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

#include "akonaditagrepository.h"

#include "akonadiitemfetchjobinterface.h"

#include "utils/compositejob.h"

using namespace Akonadi;


TagRepository::TagRepository(const StorageInterface::Ptr &storage,
                             const SerializerInterface::Ptr &serializer)
    : m_storage(storage),
      m_serializer(serializer)
{
}

KJob *TagRepository::create(Domain::Tag::Ptr tag)
{
    auto akonadiTag = m_serializer->createAkonadiTagFromTag(tag);
    Q_ASSERT(!akonadiTag.isValid());
    return m_storage->createTag(akonadiTag);
}

KJob *TagRepository::remove(Domain::Tag::Ptr tag)
{
   auto akonadiTag = m_serializer->createAkonadiTagFromTag(tag);
   Q_ASSERT(akonadiTag.isValid());
   return m_storage->removeTag(akonadiTag);
}

KJob *TagRepository::associate(Domain::Tag::Ptr parent, Domain::Note::Ptr child)
{
    auto akonadiTag = m_serializer->createAkonadiTagFromTag(parent);
    Q_ASSERT(akonadiTag.isValid());

    Item childItem = m_serializer->createItemFromNote(child);
    Q_ASSERT(childItem.isValid());

    auto job = new Utils::CompositeJob();
    ItemFetchJobInterface *fetchItemJob = m_storage->fetchItem(childItem);
    job->install(fetchItemJob->kjob(), [akonadiTag, fetchItemJob, parent, job, this] {
        if (fetchItemJob->kjob()->error() != KJob::NoError)
            return;

        Q_ASSERT(fetchItemJob->items().size() == 1);
        auto childItem = fetchItemJob->items().first();
        childItem.setTag(akonadiTag);

        auto updateJob = m_storage->updateItem(childItem);
        job->addSubjob(updateJob);
        updateJob->start();
    });
    return job;
}

KJob *TagRepository::dissociate(Domain::Tag::Ptr parent, Domain::Note::Ptr child)
{
    Item childItem = m_serializer->createItemFromNote(child);
    Q_ASSERT(childItem.isValid());

    auto job = new Utils::CompositeJob();
    ItemFetchJobInterface *fetchItemJob = m_storage->fetchItem(childItem);
    job->install(fetchItemJob->kjob(), [fetchItemJob, parent, job, this] {
        if (fetchItemJob->kjob()->error() != KJob::NoError)
            return;

        Q_ASSERT(fetchItemJob->items().size() == 1);
        auto childItem = fetchItemJob->items().first();
        auto tag = m_serializer->createAkonadiTagFromTag(parent);
        Q_ASSERT(tag.isValid());
        childItem.clearTag(tag);

        auto updateJob = m_storage->updateItem(childItem);
        job->addSubjob(updateJob);
        updateJob->start();
    });

    return job;
}

KJob *TagRepository::dissociateAll(Domain::Note::Ptr child)
{
    Item childItem;

    childItem = m_serializer->createItemFromNote(child);
    Q_ASSERT(childItem.isValid());
    auto job = new Utils::CompositeJob();
    ItemFetchJobInterface *fetchItemJob = m_storage->fetchItem(childItem);
    job->install(fetchItemJob->kjob(), [fetchItemJob, job, this] {
        if (fetchItemJob->kjob()->error() != KJob::NoError)
            return;

        Q_ASSERT(fetchItemJob->items().size() == 1);
        auto childItem = fetchItemJob->items().first();
        foreach (const Tag &tag, childItem.tags())
            childItem.clearTag(tag);

        auto updateJob = m_storage->updateItem(childItem);
        job->addSubjob(updateJob);
        updateJob->start();
    });

    return job;
}
