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

class DataSourceQueries : public QObject, public Domain::DataSourceQueries
{
    Q_OBJECT
public:
    typedef QSharedPointer<DataSourceQueries> Ptr;

    typedef Domain::LiveQueryInput<Akonadi::Collection> CollectionInputQuery;
    typedef Domain::LiveQueryOutput<Domain::DataSource::Ptr> DataSourceQueryOutput;
    typedef Domain::QueryResult<Domain::DataSource::Ptr> DataSourceResult;

    typedef Domain::LiveQueryInput<Akonadi::Item> ItemInputQuery;
    typedef Domain::LiveQueryOutput<Domain::Project::Ptr> ProjectQueryOutput;
    typedef Domain::QueryResult<Domain::Project::Ptr> ProjectResult;

    DataSourceQueries(const StorageInterface::Ptr &storage,
                      const SerializerInterface::Ptr &serializer,
                      const MonitorInterface::Ptr &monitor);

    bool isDefaultSource(Domain::DataSource::Ptr source) const override;
private:
    void changeDefaultSource(Domain::DataSource::Ptr source) override;

public:
    DataSourceResult::Ptr findTopLevel() const override;
    DataSourceResult::Ptr findChildren(Domain::DataSource::Ptr source) const override;
    DataSourceResult::Ptr findAllSelected() const override;
    ProjectResult::Ptr findProjects(Domain::DataSource::Ptr source) const override;

private:
    CollectionInputQuery::PredicateFunction createFetchPredicate(const Collection &root) const;

    SerializerInterface::Ptr m_serializer;
    LiveQueryHelpers::Ptr m_helpers;
    LiveQueryIntegrator::Ptr m_integrator;

    mutable DataSourceQueryOutput::Ptr m_findTopLevel;
    mutable QHash<Akonadi::Collection::Id, DataSourceQueryOutput::Ptr> m_findChildren;
    mutable DataSourceQueryOutput::Ptr m_findAllSelected;
    mutable QHash<Akonadi::Collection::Id, ProjectQueryOutput::Ptr> m_findProjects;
};

}

#endif // AKONADI_DATASOURCEQUERIES_H
