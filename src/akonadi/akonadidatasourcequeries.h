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

#include <functional>

#include <QHash>

#include <Akonadi/Collection>

#include "akonadi/akonadimonitorinterface.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

#include "domain/livequery.h"

class KJob;

namespace Akonadi {

class Item;

class DataSourceQueries : public QObject, public Domain::DataSourceQueries
{
    Q_OBJECT
public:
    typedef QSharedPointer<DataSourceQueries> Ptr;

    typedef Domain::LiveQuery<Akonadi::Collection, Domain::DataSource::Ptr> DataSourceQuery;
    typedef Domain::QueryResultProvider<Domain::DataSource::Ptr> DataSourceProvider;
    typedef Domain::QueryResult<Domain::DataSource::Ptr> DataSourceResult;

    explicit DataSourceQueries(QObject *parent = 0);
    DataSourceQueries(const StorageInterface::Ptr &storage,
                      const SerializerInterface::Ptr &serializer,
                      const MonitorInterface::Ptr &monitor);

    DataSourceResult::Ptr findTasks() const;
    DataSourceResult::Ptr findNotes() const;
    DataSourceResult::Ptr findTopLevel() const;
    DataSourceResult::Ptr findChildren(Domain::DataSource::Ptr source) const;

    QString searchTerm() const;
    void setSearchTerm(QString term);
    DataSourceResult::Ptr findSearchTopLevel() const;
    DataSourceResult::Ptr findSearchChildren(Domain::DataSource::Ptr source) const;

private slots:
    void onCollectionAdded(const Akonadi::Collection &collection);
    void onCollectionRemoved(const Akonadi::Collection &collection);
    void onCollectionChanged(const Akonadi::Collection &collection);

private:
    DataSourceQuery::Ptr createDataSourceQuery();

    StorageInterface::Ptr m_storage;
    SerializerInterface::Ptr m_serializer;
    MonitorInterface::Ptr m_monitor;

    DataSourceQuery::Ptr m_findTasks;
    DataSourceQuery::Ptr m_findNotes;
    DataSourceQuery::Ptr m_findTopLevel;
    QHash<Akonadi::Entity::Id, DataSourceQuery::Ptr> m_findChildren;
    DataSourceQuery::List m_dataSourceQueries;
    QString m_searchTerm;
    DataSourceQuery::Ptr m_findSearchTopLevel;
    QHash<Akonadi::Entity::Id, DataSourceQuery::Ptr> m_findSearchChildren;
};

}

#endif // AKONADI_DATASOURCEQUERIES_H
