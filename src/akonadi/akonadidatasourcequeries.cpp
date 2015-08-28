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


#include "akonadidatasourcequeries.h"

#include "akonadicollectionfetchjobinterface.h"
#include "akonadicollectionsearchjobinterface.h"

#include "utils/jobhandler.h"

using namespace Akonadi;

DataSourceQueries::DataSourceQueries(const StorageInterface::Ptr &storage,
                                     const SerializerInterface::Ptr &serializer,
                                     const MonitorInterface::Ptr &monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_integrator(new LiveQueryIntegrator(serializer, monitor))
{
    m_integrator->addRemoveHandler([this] (const Collection &collection) {
        m_findChildren.remove(collection.id());
        m_findSearchChildren.remove(collection.id());
    });
}

DataSourceQueries::DataSourceResult::Ptr DataSourceQueries::findTasks() const
{
    m_integrator->bind(m_findTasks,
                  [this] (const CollectionInputQuery::AddFunction &add) {
                      CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(), StorageInterface::Recursive, StorageInterface::Tasks);
                      Utils::JobHandler::install(job->kjob(), [this, job, add] {
                          for (auto collection : job->collections())
                              add(collection);
                      });
                  },
                  [this] (const Akonadi::Collection &collection) {
                      return m_serializer->isTaskCollection(collection);
                  },
                  SerializerInterface::FullPath);

    return m_findTasks->result();
}

DataSourceQueries::DataSourceResult::Ptr DataSourceQueries::findNotes() const
{
    m_integrator->bind(m_findNotes,
                  [this] (const CollectionInputQuery::AddFunction &add) {
                      CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(), StorageInterface::Recursive, StorageInterface::Notes);
                      Utils::JobHandler::install(job->kjob(), [this, job, add] {
                          for (auto collection : job->collections())
                              add(collection);
                      });
                  },
                  [this] (const Akonadi::Collection &collection) {
                      return m_serializer->isNoteCollection(collection);
                  },
                  SerializerInterface::FullPath);

    return m_findNotes->result();
}

DataSourceQueries::DataSourceResult::Ptr DataSourceQueries::findTopLevel() const
{
    m_integrator->bind(m_findTopLevel,
                  [this] (const CollectionInputQuery::AddFunction &add) {
                      CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                                     StorageInterface::Recursive,
                                                                                     StorageInterface::Tasks | StorageInterface::Notes);
                      Utils::JobHandler::install(job->kjob(), [this, job, add] {
                          if (job->kjob()->error())
                              return;

                          QHash<Collection::Id, Collection> topLevels;
                          foreach (const auto &collection, job->collections()) {
                              auto topLevel = collection;
                              while (topLevel.parentCollection() != Collection::root())
                                  topLevel = topLevel.parentCollection();
                              if (!topLevels.contains(topLevel.id()))
                                  topLevels[topLevel.id()] = topLevel;
                          }

                          foreach (const auto &topLevel, topLevels.values())
                              add(topLevel);
                      });
                  },
                  [this] (const Akonadi::Collection &collection) {
                      return collection.isValid()
                          && collection.parentCollection() == Akonadi::Collection::root()
                          && m_serializer->isListedCollection(collection);
                  });

    return m_findTopLevel->result();
}

DataSourceQueries::DataSourceResult::Ptr DataSourceQueries::findChildren(Domain::DataSource::Ptr source) const
{
    Collection root = m_serializer->createCollectionFromDataSource(source);
    auto &query = m_findChildren[root.id()];
    m_integrator->bind(query,
                  [this, root] (const CollectionInputQuery::AddFunction &add) {
                      CollectionFetchJobInterface *job = m_storage->fetchCollections(root,
                                                                                     StorageInterface::Recursive,
                                                                                     StorageInterface::Tasks | StorageInterface::Notes);
                      Utils::JobHandler::install(job->kjob(), [this, root, job, add] {
                          if (job->kjob()->error())
                              return;

                          QHash<Collection::Id, Collection> children;
                          foreach (const auto &collection, job->collections()) {
                              auto child = collection;
                              while (child.parentCollection() != root)
                                  child = child.parentCollection();
                              if (!children.contains(child.id()))
                                  children[child.id()] = child;
                          }

                          foreach (const auto &topLevel, children.values())
                              add(topLevel);
                      });
                  },
                  [this, root] (const Akonadi::Collection &collection) {
                      return collection.isValid()
                          && collection.parentCollection() == root
                          && m_serializer->isListedCollection(collection);
                  });

    return query->result();
}

QString DataSourceQueries::searchTerm() const
{
    return m_searchTerm;
}

void DataSourceQueries::setSearchTerm(QString term)
{
    if (m_searchTerm == term)
        return;

    m_searchTerm = term;
    if (m_findSearchTopLevel) {
        m_findSearchTopLevel->reset();
    }
    foreach(auto query, m_findSearchChildren.values())
        query->reset();
}

DataSourceQueries::DataSourceResult::Ptr DataSourceQueries::findSearchTopLevel() const
{
    m_integrator->bind(m_findSearchTopLevel,
                  [this] (const CollectionInputQuery::AddFunction &add) {
                      if (m_searchTerm.isEmpty())
                          return;

                      CollectionSearchJobInterface *job = m_storage->searchCollections(m_searchTerm);
                      Utils::JobHandler::install(job->kjob(), [this, job, add] {
                          if (job->kjob()->error())
                              return;

                          QHash<Collection::Id, Collection> topLevels;
                          foreach (const auto &collection, job->collections()) {
                              auto topLevel = collection;
                              while (topLevel.parentCollection() != Collection::root())
                                  topLevel = topLevel.parentCollection();
                              if (!topLevels.contains(topLevel.id()))
                                  topLevels[topLevel.id()] = topLevel;
                          }

                          foreach (const auto &topLevel, topLevels.values())
                              add(topLevel);

                      });
                  },
                  [this] (const Akonadi::Collection &collection) {
                      return collection.isValid()
                          && collection.parentCollection() == Akonadi::Collection::root();
                  });

    return m_findSearchTopLevel->result();
}

DataSourceQueries::DataSourceResult::Ptr DataSourceQueries::findSearchChildren(Domain::DataSource::Ptr source) const
{
    Collection root = m_serializer->createCollectionFromDataSource(source);
    auto &query = m_findSearchChildren[root.id()];
    m_integrator->bind(query,
                  [this, root] (const CollectionInputQuery::AddFunction &add) {
                      if (m_searchTerm.isEmpty())
                          return;

                      CollectionSearchJobInterface *job = m_storage->searchCollections(m_searchTerm);
                      Utils::JobHandler::install(job->kjob(), [this, root, job, add] {
                          if (job->kjob()->error())
                              return;

                          QHash<Collection::Id, Collection> children;
                          foreach (const auto &collection, job->collections()) {
                              auto child = collection;
                              while (child.parentCollection() != root && child.parentCollection().isValid())
                                  child = child.parentCollection();
                              if (!children.contains(child.id()))
                                  children[child.id()] = child;
                          }

                          foreach (const auto &topLevel, children.values())
                              add(topLevel);
                      });
                  },
                  [this, root] (const Akonadi::Collection &collection) {
                      return collection.isValid() && collection.parentCollection() == root;
                  });

    return m_findSearchChildren.value(root.id())->result();
}
