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
#include "akonadiitemfetchjobinterface.h"
#include "akonadimonitorimpl.h"
#include "akonadiserializer.h"
#include "akonadistorage.h"

#include "utils/jobhandler.h"

using namespace Akonadi;

DataSourceQueries::DataSourceQueries()
    : m_storage(new Storage),
      m_serializer(new Serializer),
      m_monitor(new MonitorImpl),
      m_ownInterfaces(true)
{
    connect(m_monitor, SIGNAL(collectionAdded(Akonadi::Collection)), this, SLOT(onCollectionAdded(Akonadi::Collection)));
    connect(m_monitor, SIGNAL(collectionRemoved(Akonadi::Collection)), this, SLOT(onCollectionRemoved(Akonadi::Collection)));
    connect(m_monitor, SIGNAL(collectionChanged(Akonadi::Collection)), this, SLOT(onCollectionChanged(Akonadi::Collection)));
}

DataSourceQueries::DataSourceQueries(StorageInterface *storage, SerializerInterface *serializer, MonitorInterface *monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_monitor(monitor),
      m_ownInterfaces(false)
{
    connect(monitor, SIGNAL(collectionAdded(Akonadi::Collection)), this, SLOT(onCollectionAdded(Akonadi::Collection)));
    connect(monitor, SIGNAL(collectionRemoved(Akonadi::Collection)), this, SLOT(onCollectionRemoved(Akonadi::Collection)));
    connect(monitor, SIGNAL(collectionChanged(Akonadi::Collection)), this, SLOT(onCollectionChanged(Akonadi::Collection)));
}

DataSourceQueries::~DataSourceQueries()
{
    if (m_ownInterfaces) {
        delete m_storage;
        delete m_serializer;
        delete m_monitor;
    }
}

DataSourceQueries::DataSourceResult::Ptr DataSourceQueries::findAll() const
{
    DataSourceProvider::Ptr provider(m_dataSourceProvider.toStrongRef());

    if (!provider) {
        provider = DataSourceProvider::Ptr(new DataSourceProvider);
        m_dataSourceProvider = provider.toWeakRef();
    }

    DataSourceQueries::DataSourceResult::Ptr result = DataSourceProvider::createResult(provider);

    CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(), StorageInterface::Recursive);
    Utils::JobHandler::install(job->kjob(), [provider, job, this] {
        for (auto collection : job->collections()) {
            auto dataSource = deserializeDataSource(collection);
            if (dataSource)
                provider->append(dataSource);
        }
    });

    return result;
}

void DataSourceQueries::onCollectionAdded(const Collection &collection)
{
    DataSourceProvider::Ptr provider(m_dataSourceProvider.toStrongRef());
    auto dataSource = deserializeDataSource(collection);

    if (!dataSource)
        return;

    if (provider) {
        provider->append(dataSource);
    }
}

void DataSourceQueries::onCollectionRemoved(const Collection &collection)
{
    DataSourceProvider::Ptr provider(m_dataSourceProvider.toStrongRef());

    if (provider) {
        for (int i = 0; i < provider->data().size(); i++) {
            auto dataSource = provider->data().at(i);
            if (isDataSourceCollection(dataSource, collection)) {
                provider->removeAt(i);
                i--;
            }
        }
    }
}

void DataSourceQueries::onCollectionChanged(const Collection &collection)
{
    DataSourceProvider::Ptr provider(m_dataSourceProvider.toStrongRef());

    if (provider) {
        for (int i = 0; i < provider->data().size(); i++) {
            auto dataSource = provider->data().at(i);
            if (isDataSourceCollection(dataSource, collection)) {
                m_serializer->updateDataSourceFromCollection(dataSource, collection);
                provider->replace(i, dataSource);
            }
        }
    }
}

bool DataSourceQueries::isDataSourceCollection(const Domain::DataSource::Ptr &dataSource, const Collection &collection) const
{
    return dataSource->property("collectionId").toLongLong() == collection.id();
}

Domain::DataSource::Ptr DataSourceQueries::deserializeDataSource(const Collection &collection) const
{
    auto dataSource = m_serializer->createDataSourceFromCollection(collection);
    if (dataSource) {
        dataSource->setProperty("collectionId", collection.id());
    }

    return dataSource;
}
