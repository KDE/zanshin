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

using namespace Akonadi;

DataSourceQueries::DataSourceQueries(const StorageInterface::Ptr &storage,
                                     const SerializerInterface::Ptr &serializer,
                                     const MonitorInterface::Ptr &monitor)
    : m_serializer(serializer),
      m_helpers(new LiveQueryHelpers(serializer, storage)),
      m_integrator(new LiveQueryIntegrator(serializer, monitor))
{
    m_integrator->addRemoveHandler([this] (const Collection &collection) {
        m_findChildren.remove(collection.id());
        m_findSearchChildren.remove(collection.id());
    });
}

DataSourceQueries::DataSourceResult::Ptr DataSourceQueries::findTasks() const
{
    auto fetch = m_helpers->fetchAllCollections(StorageInterface::Tasks);
    auto predicate = [this] (const Akonadi::Collection &collection) {
        return m_serializer->isTaskCollection(collection);
    };
    m_integrator->bind(m_findTasks, fetch, predicate, SerializerInterface::FullPath);
    return m_findTasks->result();
}

DataSourceQueries::DataSourceResult::Ptr DataSourceQueries::findNotes() const
{
    auto fetch = m_helpers->fetchAllCollections(StorageInterface::Notes);
    auto predicate = [this] (const Akonadi::Collection &collection) {
        return m_serializer->isNoteCollection(collection);
    };
    m_integrator->bind(m_findNotes, fetch, predicate, SerializerInterface::FullPath);
    return m_findNotes->result();
}

DataSourceQueries::DataSourceResult::Ptr DataSourceQueries::findTopLevel() const
{
    auto fetch = m_helpers->fetchCollections(Collection::root(), StorageInterface::Tasks | StorageInterface::Notes);
    auto predicate = createFetchPredicate(Collection::root());
    m_integrator->bind(m_findTopLevel, fetch, predicate);
    return m_findTopLevel->result();
}

DataSourceQueries::DataSourceResult::Ptr DataSourceQueries::findChildren(Domain::DataSource::Ptr source) const
{
    Collection root = m_serializer->createCollectionFromDataSource(source);
    auto &query = m_findChildren[root.id()];
    auto fetch = m_helpers->fetchCollections(root, StorageInterface::Tasks | StorageInterface::Notes);
    auto predicate = createFetchPredicate(root);
    m_integrator->bind(query, fetch, predicate);
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
    auto fetch = m_helpers->searchCollections(Collection::root(), &m_searchTerm, Akonadi::StorageInterface::Tasks | Akonadi::StorageInterface::Notes);
    auto predicate = createSearchPredicate(Collection::root());
    m_integrator->bind(m_findSearchTopLevel, fetch, predicate);
    return m_findSearchTopLevel->result();
}

DataSourceQueries::DataSourceResult::Ptr DataSourceQueries::findSearchChildren(Domain::DataSource::Ptr source) const
{
    Collection root = m_serializer->createCollectionFromDataSource(source);
    auto &query = m_findSearchChildren[root.id()];
    auto fetch = m_helpers->searchCollections(root, &m_searchTerm, Akonadi::StorageInterface::Tasks | Akonadi::StorageInterface::Notes);
    auto predicate = createSearchPredicate(root);
    m_integrator->bind(query, fetch, predicate);
    return query->result();
}

DataSourceQueries::CollectionInputQuery::PredicateFunction DataSourceQueries::createFetchPredicate(const Collection &root) const
{
    return [this, root] (const Collection &collection) {
        return collection.isValid()
            && collection.parentCollection() == root
            && m_serializer->isListedCollection(collection);
    };
}

DataSourceQueries::CollectionInputQuery::PredicateFunction DataSourceQueries::createSearchPredicate(const Collection &root) const
{
    return [root] (const Collection &collection) {
        return collection.isValid()
            && collection.parentCollection() == root;
    };
}
