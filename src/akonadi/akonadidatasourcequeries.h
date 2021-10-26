/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
