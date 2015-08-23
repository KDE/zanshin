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

#include <functional>

using namespace std::placeholders;

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
        m_findInbox->setPredicateFunction([this] (const Akonadi::Item &item) {
            const bool excluded = !m_serializer->relatedUidFromItem(item).isEmpty()
                               || (!m_serializer->isTaskItem(item) && !m_serializer->isNoteItem(item))
                               || (m_serializer->isTaskItem(item) && m_serializer->hasContextTags(item))
                               || m_serializer->hasAkonadiTags(item);

            return !excluded;
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
    foreach (const auto &query, m_itemInputQueries)
        query->onAdded(item);
}

void ArtifactQueries::onItemRemoved(const Item &item)
{
    foreach (const auto &query, m_itemInputQueries)
        query->onRemoved(item);
}

void ArtifactQueries::onItemChanged(const Item &item)
{
    foreach (const auto &query, m_itemInputQueries)
        query->onChanged(item);
}

void ArtifactQueries::onCollectionSelectionChanged()
{
    foreach (const auto &query, m_itemInputQueries)
        query->reset();
}

ArtifactQueries::ArtifactQuery::Ptr ArtifactQueries::createArtifactQuery()
{
    auto query = ArtifactQuery::Ptr::create();

    query->setConvertFunction(std::bind(&SerializerInterface::createArtifactFromItem, m_serializer, _1));
    query->setUpdateFunction(std::bind(&SerializerInterface::updateArtifactFromItem, m_serializer, _2, _1));
    query->setRepresentsFunction(std::bind(&SerializerInterface::representsItem, m_serializer, _2, _1));

    m_itemInputQueries << query;
    return query;
}
