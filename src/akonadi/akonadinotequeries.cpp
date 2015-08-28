/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Remi Benoit <r3m1.benoit@gmail.com>

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

#include "akonadinotequeries.h"

#include "akonadicollectionfetchjobinterface.h"
#include "akonadiitemfetchjobinterface.h"

#include "utils/jobhandler.h"

using namespace Akonadi;

NoteQueries::NoteQueries(const StorageInterface::Ptr &storage,
                         const SerializerInterface::Ptr &serializer,
                         const MonitorInterface::Ptr &monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_integrator(new LiveQueryIntegrator(serializer, monitor))
{
}

NoteQueries::NoteResult::Ptr NoteQueries::findAll() const
{
    m_integrator->bind(m_findAll,
                  [this] (const ItemInputQuery::AddFunction &add) {
                      CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                                     StorageInterface::Recursive,
                                                                                     StorageInterface::Notes);
                      Utils::JobHandler::install(job->kjob(), [this, job, add] {
                          if (job->kjob()->error() != KJob::NoError)
                              return;

                          for (auto collection : job->collections()) {
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
                      return m_serializer->isNoteItem(item);
                  });

    return m_findAll->result();
}
