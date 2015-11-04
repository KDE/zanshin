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

#ifndef AKONADI_DATASOURCEQUERIES_H
#define AKONADI_DATASOURCEQUERIES_H

#include "domain/datasourcequeries.h"

#include "akonadi/akonadilivequeryhelpers.h"
#include "akonadi/akonadilivequeryintegrator.h"

namespace Akonadi {

class DataSourceQueries : public Domain::DataSourceQueries
{
public:
    typedef QSharedPointer<DataSourceQueries> Ptr;

    typedef Domain::LiveQueryInput<Akonadi::Collection> CollectionInputQuery;
    typedef Domain::LiveQueryOutput<Domain::DataSource::Ptr> DataSourceQueryOutput;
    typedef Domain::QueryResultProvider<Domain::DataSource::Ptr> DataSourceProvider;
    typedef Domain::QueryResult<Domain::DataSource::Ptr> DataSourceResult;

    DataSourceQueries(const StorageInterface::FetchContentTypes &contentTypes,
                      const StorageInterface::Ptr &storage,
                      const SerializerInterface::Ptr &serializer,
                      const MonitorInterface::Ptr &monitor);

    DataSourceResult::Ptr findTasks() const Q_DECL_OVERRIDE;
    DataSourceResult::Ptr findNotes() const Q_DECL_OVERRIDE;
    DataSourceResult::Ptr findTopLevel() const Q_DECL_OVERRIDE;
    DataSourceResult::Ptr findChildren(Domain::DataSource::Ptr source) const Q_DECL_OVERRIDE;

    QString searchTerm() const Q_DECL_OVERRIDE;
    void setSearchTerm(QString term) Q_DECL_OVERRIDE;
    DataSourceResult::Ptr findSearchTopLevel() const Q_DECL_OVERRIDE;
    DataSourceResult::Ptr findSearchChildren(Domain::DataSource::Ptr source) const Q_DECL_OVERRIDE;

private:
    CollectionInputQuery::PredicateFunction createFetchPredicate(const Collection &root) const;
    CollectionInputQuery::PredicateFunction createSearchPredicate(const Collection &root) const;

    StorageInterface::FetchContentTypes m_contentTypes;
    SerializerInterface::Ptr m_serializer;
    LiveQueryHelpers::Ptr m_helpers;
    LiveQueryIntegrator::Ptr m_integrator;

    mutable DataSourceQueryOutput::Ptr m_findTasks;
    mutable DataSourceQueryOutput::Ptr m_findNotes;
    mutable DataSourceQueryOutput::Ptr m_findTopLevel;
    mutable QHash<Akonadi::Collection::Id, DataSourceQueryOutput::Ptr> m_findChildren;
    QString m_searchTerm;
    mutable DataSourceQueryOutput::Ptr m_findSearchTopLevel;
    mutable QHash<Akonadi::Collection::Id, DataSourceQueryOutput::Ptr> m_findSearchChildren;
};

}

#endif // AKONADI_DATASOURCEQUERIES_H
