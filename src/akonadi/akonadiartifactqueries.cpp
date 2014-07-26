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


#include "akonadiartifactqueries.h"

#include "akonadicollectionfetchjobinterface.h"
#include "akonadiitemfetchjobinterface.h"
#include "akonadimonitorimpl.h"
#include "akonadiserializer.h"
#include "akonadistorage.h"

#include "utils/jobhandler.h"

using namespace Akonadi;

ArtifactQueries::ArtifactQueries()
    : m_storage(new Storage),
      m_serializer(new Serializer),
      m_monitor(new MonitorImpl),
      m_ownInterfaces(true)
{
}

ArtifactQueries::ArtifactQueries(StorageInterface *storage, SerializerInterface *serializer, MonitorInterface *monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_monitor(monitor),
      m_ownInterfaces(false)
{
}

ArtifactQueries::~ArtifactQueries()
{
    if (m_ownInterfaces) {
        delete m_storage;
        delete m_serializer;
        delete m_monitor;
    }
}

ArtifactQueries::ArtifactResult::Ptr ArtifactQueries::findInboxTopLevel() const
{
    ArtifactProvider::Ptr provider(m_inboxProvider.toStrongRef());

    if (!provider) {
        provider = ArtifactProvider::Ptr(new ArtifactProvider);
        m_inboxProvider = provider.toWeakRef();
    }

    ArtifactQueries::ArtifactResult::Ptr result = ArtifactProvider::createResult(provider);

    CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                   StorageInterface::Recursive,
                                                                   StorageInterface::Tasks|StorageInterface::Notes);
    Utils::JobHandler::install(job->kjob(), [provider, job, this] {
        if (job->kjob()->error() != KJob::NoError)
            return;

        for (auto collection : job->collections()) {
            ItemFetchJobInterface *job = m_storage->fetchItems(collection);
            Utils::JobHandler::install(job->kjob(), [provider, job, this] {
                if (job->kjob()->error() != KJob::NoError)
                    return;

                for (auto item : job->items()) {
                    if (!m_serializer->relatedUidFromItem(item).isEmpty())
                        continue;

                    auto task = m_serializer->createTaskFromItem(item);
                    if (task) {
                        provider->append(task);
                        continue;
                    }

                    auto note = m_serializer->createNoteFromItem(item);
                    if (note) {
                        provider->append(note);
                        continue;
                    }
                }
            });
        }
    });

    return result;
}
