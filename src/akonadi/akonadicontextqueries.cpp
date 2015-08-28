/* This file is part of Zanshin

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

#include "akonadicontextqueries.h"

#include "akonadicollectionfetchjobinterface.h"
#include "akonadiitemfetchjobinterface.h"
#include "akonaditagfetchjobinterface.h"

#include "utils/jobhandler.h"

using namespace Akonadi;

ContextQueries::ContextQueries(const StorageInterface::Ptr &storage,
                               const SerializerInterface::Ptr &serializer,
                               const MonitorInterface::Ptr &monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_integrator(new LiveQueryIntegrator(serializer, monitor))
{
    m_integrator->addRemoveHandler([this] (const Tag &tag) {
        m_findToplevel.remove(tag.id());
    });
}

ContextQueries::ContextResult::Ptr ContextQueries::findAll() const
{
    m_integrator->bind(m_findAll,
                  [this] (const TagInputQuery::AddFunction &add) {
                      TagFetchJobInterface *job = m_storage->fetchTags();
                      Utils::JobHandler::install(job->kjob(), [this, job, add] {
                          for (Akonadi::Tag tag : job->tags())
                              add(tag);
                      });
                  },
                  [this] (const Akonadi::Tag &tag) {
                      return tag.type() == Akonadi::SerializerInterface::contextTagType();
                  });

    return m_findAll->result();
}

ContextQueries::TaskResult::Ptr ContextQueries::findTopLevelTasks(Domain::Context::Ptr context) const
{
    Akonadi::Tag tag = m_serializer->createTagFromContext(context);
    auto &query = m_findToplevel[tag.id()];
    m_integrator->bind(query,
                  [this, tag] (const ItemInputQuery::AddFunction &add) {
                      ItemFetchJobInterface *job = m_storage->fetchTagItems(tag);
                      Utils::JobHandler::install(job->kjob(), [this, job, add] {
                          if (job->kjob()->error() != KJob::NoError)
                              return;

                          for (auto item : job->items())
                              add(item);
                      });
                  },
                  [this, context] (const Akonadi::Item &item) {
                      return m_serializer->isContextChild(context, item);
                  });

    return query->result();
}
