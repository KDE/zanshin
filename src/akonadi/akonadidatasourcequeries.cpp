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

DataSourceQueries::DataSourceQueries(QObject *parent)
    : QObject(parent),
      m_storage(new Storage),
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

DataSourceQueries::DataSourceResult::Ptr DataSourceQueries::findTasks() const
{
    if (!m_findTasks) {
        {
            DataSourceQueries *self = const_cast<DataSourceQueries*>(this);
            self->m_findTasks = self->createDataSourceQuery();
        }

        m_findTasks->setFetchFunction([this] (const DataSourceQuery::AddFunction &add) {
            CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(), StorageInterface::Recursive, StorageInterface::Tasks);
            Utils::JobHandler::install(job->kjob(), [this, job, add] {
                for (auto collection : job->collections())
                    add(collection);
            });
        });

        m_findTasks->setConvertFunction([this] (const Akonadi::Collection &collection) {
            return m_serializer->createDataSourceFromCollection(collection);
        });
        m_findTasks->setUpdateFunction([this] (const Akonadi::Collection &collection, Domain::DataSource::Ptr &source) {
            m_serializer->updateDataSourceFromCollection(source, collection);
        });
        m_findTasks->setPredicateFunction([this] (const Akonadi::Collection &collection) {
            return m_serializer->isTaskCollection(collection);
        });
        m_findTasks->setRepresentsFunction([this] (const Akonadi::Collection &collection, const Domain::DataSource::Ptr &source) {
            return m_serializer->representsCollection(source, collection);
        });
    }

    return m_findTasks->result();
}

DataSourceQueries::DataSourceResult::Ptr DataSourceQueries::findNotes() const
{
    if (!m_findNotes) {
        {
            DataSourceQueries *self = const_cast<DataSourceQueries*>(this);
            self->m_findNotes = self->createDataSourceQuery();
        }

        m_findNotes->setFetchFunction([this] (const DataSourceQuery::AddFunction &add) {
            CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(), StorageInterface::Recursive, StorageInterface::Notes);
            Utils::JobHandler::install(job->kjob(), [this, job, add] {
                for (auto collection : job->collections())
                    add(collection);
            });
        });

        m_findNotes->setConvertFunction([this] (const Akonadi::Collection &collection) {
            return m_serializer->createDataSourceFromCollection(collection);
        });
        m_findNotes->setUpdateFunction([this] (const Akonadi::Collection &collection, Domain::DataSource::Ptr &source) {
            m_serializer->updateDataSourceFromCollection(source, collection);
        });
        m_findNotes->setPredicateFunction([this] (const Akonadi::Collection &collection) {
            return m_serializer->isNoteCollection(collection);
        });
        m_findNotes->setRepresentsFunction([this] (const Akonadi::Collection &collection, const Domain::DataSource::Ptr &source) {
            return m_serializer->representsCollection(source, collection);
        });
    }

    return m_findNotes->result();
}

void DataSourceQueries::onCollectionAdded(const Collection &collection)
{
    foreach (const DataSourceQuery::Ptr &query, m_dataSourceQueries)
        query->onAdded(collection);
}

void DataSourceQueries::onCollectionRemoved(const Collection &collection)
{
    foreach (const DataSourceQuery::Ptr &query, m_dataSourceQueries)
        query->onRemoved(collection);
}

void DataSourceQueries::onCollectionChanged(const Collection &collection)
{
    foreach (const DataSourceQuery::Ptr &query, m_dataSourceQueries)
        query->onChanged(collection);
}

DataSourceQueries::DataSourceQuery::Ptr DataSourceQueries::createDataSourceQuery()
{
    auto query = DataSourceQuery::Ptr::create();
    m_dataSourceQueries << query;
    return query;
}
