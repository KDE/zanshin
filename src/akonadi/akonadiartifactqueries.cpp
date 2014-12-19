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

#include "utils/jobhandler.h"

using namespace Akonadi;

ArtifactQueries::ArtifactQueries(const StorageInterface::Ptr &storage,
                                 const SerializerInterface::Ptr &serializer,
                                 const MonitorInterface::Ptr &monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_monitor(monitor)
{
    connect(m_monitor.data(), SIGNAL(itemAdded(Akonadi::Item)), this, SLOT(onItemAdded(Akonadi::Item)));
    connect(m_monitor.data(), SIGNAL(itemRemoved(Akonadi::Item)), this, SLOT(onItemRemoved(Akonadi::Item)));
    connect(m_monitor.data(), SIGNAL(itemChanged(Akonadi::Item)), this, SLOT(onItemChanged(Akonadi::Item)));
    connect(m_monitor.data(), SIGNAL(collectionSelectionChanged(Akonadi::Collection)), this, SLOT(onCollectionSelectionChanged()));
}

ArtifactQueries::ArtifactResult::Ptr ArtifactQueries::findInboxTopLevel() const
{
    if (!m_findInbox) {
        {
            ArtifactQueries *self = const_cast<ArtifactQueries*>(this);
            self->m_findInbox = self->createArtifactQuery();
        }

        m_findInbox->setFetchFunction([this] (const ArtifactQuery::AddFunction &add) {
            CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                           StorageInterface::Recursive,
                                                                           StorageInterface::Tasks|StorageInterface::Notes);
            Utils::JobHandler::install(job->kjob(), [this, job, add] {
                if (job->kjob()->error() != KJob::NoError)
                    return;

                for (auto collection : job->collections()) {
                    if (!m_serializer->isSelectedCollection(collection))
                        continue;

                    ItemFetchJobInterface *job = m_storage->fetchItems(collection);
                    Utils::JobHandler::install(job->kjob(), [this, job, add] {
                        if (job->kjob()->error() != KJob::NoError)
                            return;

                        for (auto item : job->items()) {
                            add(item);
                        }
                    });
                }
            });
        });

        m_findInbox->setConvertFunction([this] (const Akonadi::Item &item) {
            if (m_serializer->isTaskItem(item)) {
                auto task = m_serializer->createTaskFromItem(item);
                return Domain::Artifact::Ptr(task);

            } else if (m_serializer->isNoteItem(item)) {
                auto note = m_serializer->createNoteFromItem(item);
                return Domain::Artifact::Ptr(note);

            } else {
                return Domain::Artifact::Ptr();
            }
        });

        m_findInbox->setUpdateFunction([this] (const Akonadi::Item &item, Domain::Artifact::Ptr &artifact) {
            if (auto task = artifact.dynamicCast<Domain::Task>()) {
                m_serializer->updateTaskFromItem(task, item);
            } else if (auto note = artifact.dynamicCast<Domain::Note>()) {
                m_serializer->updateNoteFromItem(note, item);
            }
        });

        m_findInbox->setPredicateFunction([this] (const Akonadi::Item &item) {
            const bool excluded = !m_serializer->relatedUidFromItem(item).isEmpty()
                               || (!m_serializer->isTaskItem(item) && !m_serializer->isNoteItem(item))
                               || (m_serializer->isTaskItem(item) && m_serializer->hasContextTags(item))
                               || m_serializer->hasAkonadiTags(item);

            return !excluded;
        });

        m_findInbox->setRepresentsFunction([this] (const Akonadi::Item &item, const Domain::Artifact::Ptr &artifact) {
            return m_serializer->representsItem(artifact, item);
        });
    }

    return m_findInbox->result();
}

ArtifactQueries::TagResult::Ptr ArtifactQueries::findTags(Domain::Artifact::Ptr artifact) const
{
    Q_UNUSED(artifact);
    qFatal("Not implemented yet");
    return TagResult::Ptr();
}

void ArtifactQueries::onItemAdded(const Item &item)
{
    foreach (const ArtifactQuery::Ptr &query, m_artifactQueries)
        query->onAdded(item);
}

void ArtifactQueries::onItemRemoved(const Item &item)
{
    foreach (const ArtifactQuery::Ptr &query, m_artifactQueries)
        query->onRemoved(item);
}

void ArtifactQueries::onItemChanged(const Item &item)
{
    foreach (const ArtifactQuery::Ptr &query, m_artifactQueries)
        query->onChanged(item);
}

void ArtifactQueries::onCollectionSelectionChanged()
{
    foreach (const ArtifactQuery::Ptr &query, m_artifactQueries)
        query->reset();
}

ArtifactQueries::ArtifactQuery::Ptr ArtifactQueries::createArtifactQuery()
{
    auto query = ArtifactQuery::Ptr::create();
    m_artifactQueries << query;
    return query;
}
