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

#include <functional>

#include <QHash>
#include <Akonadi/Item>

#include "domain/datasourcequeries.h"

class KJob;

namespace Akonadi {

class Item;
class MonitorInterface;
class SerializerInterface;
class StorageInterface;

class DataSourceQueries : public QObject, public Domain::DataSourceQueries
{
    Q_OBJECT
public:
    typedef Domain::QueryResultProvider<Domain::DataSource::Ptr> DataSourceProvider;
    typedef Domain::QueryResult<Domain::DataSource::Ptr> DataSourceResult;

    explicit DataSourceQueries(QObject *parent = 0);
    DataSourceQueries(StorageInterface *storage, SerializerInterface *serializer, MonitorInterface *monitor);
    virtual ~DataSourceQueries();

    DataSourceResult::Ptr findTasks() const;
    DataSourceResult::Ptr findNotes() const;

private slots:
    void onCollectionAdded(const Akonadi::Collection &collection);
    void onCollectionRemoved(const Akonadi::Collection &collection);
    void onCollectionChanged(const Akonadi::Collection &collection);

private:
    bool isDataSourceCollection(const Domain::DataSource::Ptr &dataSource, const Collection &collection) const;
    Domain::DataSource::Ptr deserializeDataSource(const Collection &collection) const;

    StorageInterface *m_storage;
    SerializerInterface *m_serializer;
    MonitorInterface *m_monitor;
    bool m_ownInterfaces;

    mutable DataSourceProvider::WeakPtr m_taskDataSourceProvider;
    mutable DataSourceProvider::WeakPtr m_noteDataSourceProvider;
};

}

#endif // AKONADI_DATASOURCEQUERIES_H
