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


#include "akonaditaskqueries.h"

#include "akonadicollectionfetchjobinterface.h"
#include "akonadiitemfetchjobinterface.h"

#include "utils/datetime.h"
#include "utils/jobhandler.h"

using namespace Akonadi;

TaskQueries::TaskQueries(const StorageInterface::Ptr &storage,
                         const SerializerInterface::Ptr &serializer,
                         const MonitorInterface::Ptr &monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_integrator(new LiveQueryIntegrator(serializer, monitor))
{
    m_integrator->addRemoveHandler([this] (const Item &item) {
        m_findChildren.remove(item.id());
    });
}

TaskQueries::TaskResult::Ptr TaskQueries::findAll() const
{
    m_integrator->bind(m_findAll,
                  [this] (const ItemInputQuery::AddFunction &add) {
                      CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                                     StorageInterface::Recursive,
                                                                                     StorageInterface::Tasks);
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
                       return m_serializer->isTaskItem(item);
                   });

    return m_findAll->result();
}

TaskQueries::TaskResult::Ptr TaskQueries::findChildren(Domain::Task::Ptr task) const
{
    Akonadi::Item item = m_serializer->createItemFromTask(task);
    auto &query = m_findChildren[item.id()];
    m_integrator->bind(query,
                  [this, item] (const ItemInputQuery::AddFunction &add) {
                      ItemFetchJobInterface *job = m_storage->fetchItem(item);
                      Utils::JobHandler::install(job->kjob(), [this, job, add] {
                          if (job->kjob()->error() != KJob::NoError)
                              return;

                          Q_ASSERT(job->items().size() == 1);
                          auto item = job->items()[0];
                          Q_ASSERT(item.parentCollection().isValid());
                          ItemFetchJobInterface *job = m_storage->fetchItems(item.parentCollection());
                          Utils::JobHandler::install(job->kjob(), [this, job, add] {
                              if (job->kjob()->error() != KJob::NoError)
                                  return;

                              for (auto item : job->items())
                                  add(item);
                          });
                      });
                  },
                  [this, task] (const Akonadi::Item &item) {
                      return m_serializer->isTaskChild(task, item);
                  });

    return query->result();
}

TaskQueries::TaskResult::Ptr TaskQueries::findTopLevel() const
{
    m_integrator->bind(m_findTopLevel,
                  [this] (const ItemInputQuery::AddFunction &add) {
                      CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                                     StorageInterface::Recursive,
                                                                                     StorageInterface::Tasks);
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
                      return m_serializer->relatedUidFromItem(item).isEmpty() && m_serializer->isTaskItem(item);
                  });

    return m_findTopLevel->result();
}

TaskQueries::TaskResult::Ptr TaskQueries::findWorkdayTopLevel() const
{
    m_integrator->bind(m_findWorkdayTopLevel,
                  [this] (const ItemInputQuery::AddFunction &add) {
                      CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                                     StorageInterface::Recursive,
                                                                                     StorageInterface::Tasks);
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
                      if (!m_serializer->isTaskItem(item))
                          return false;

                      const Domain::Task::Ptr task = m_serializer->createTaskFromItem(item);

                      const QDate doneDate = task->doneDate().date();
                      const QDate startDate = task->startDate().date();
                      const QDate dueDate = task->dueDate().date();
                      const QDate today = Utils::DateTime::currentDateTime().date();

                      const bool pastStartDate = startDate.isValid() && startDate <= today;
                      const bool pastDueDate = dueDate.isValid() && dueDate <= today;
                      const bool todayDoneDate = doneDate == today;

                      if (task->isDone())
                          return todayDoneDate;
                      else
                          return pastStartDate || pastDueDate;
                  });

    return m_findWorkdayTopLevel->result();
}

TaskQueries::ContextResult::Ptr TaskQueries::findContexts(Domain::Task::Ptr task) const
{
    qFatal("Not implemented yet");
    Q_UNUSED(task);
    return ContextResult::Ptr();
}
