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
      m_integrator(new LiveQueryIntegrator(serializer, monitor))
{
}

ArtifactQueries::ArtifactResult::Ptr ArtifactQueries::findInboxTopLevel() const
{
    m_integrator->bind(m_findInbox,
                  [this] (const ItemInputQuery::AddFunction &add) {
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
                  },
                  [this] (const Akonadi::Item &item) {
                      const bool excluded = !m_serializer->relatedUidFromItem(item).isEmpty()
                                         || (!m_serializer->isTaskItem(item) && !m_serializer->isNoteItem(item))
                                         || (m_serializer->isTaskItem(item) && m_serializer->hasContextTags(item))
                                         || m_serializer->hasAkonadiTags(item);

                      return !excluded;
                  });

    return m_findInbox->result();
}

ArtifactQueries::TagResult::Ptr ArtifactQueries::findTags(Domain::Artifact::Ptr artifact) const
{
    Q_UNUSED(artifact);
    qFatal("Not implemented yet");
    return TagResult::Ptr();
}
