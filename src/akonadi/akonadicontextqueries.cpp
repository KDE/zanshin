/* This file is part of Zanshin

   Copyright 2014 Franck Arrecot <franck.arrecot@gmail.com>
   Copyright 2014 Rémi Benoit <r3m1.benoit@gmail.com>

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

#include "akonadicontextqueries.h"

#include "akonadicollectionfetchjobinterface.h"
#include "akonadiitemfetchjobinterface.h"
#include "akonaditagfetchjobinterface.h"

#include "utils/jobhandler.h"
#include <QByteArray>
#include <QDebug>

#include <functional>

using namespace std::placeholders;

using namespace Akonadi;

ContextQueries::ContextQueries(const StorageInterface::Ptr &storage,
                               const SerializerInterface::Ptr &serializer,
                               const MonitorInterface::Ptr &monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_monitor(monitor)
{
    connect(m_monitor.data(), SIGNAL(tagAdded(Akonadi::Tag)), this, SLOT(onTagAdded(Akonadi::Tag)));
    connect(m_monitor.data(), SIGNAL(tagRemoved(Akonadi::Tag)), this, SLOT(onTagRemoved(Akonadi::Tag)));
    connect(m_monitor.data(), SIGNAL(tagChanged(Akonadi::Tag)), this, SLOT(onTagChanged(Akonadi::Tag)));

    connect(m_monitor.data(), SIGNAL(itemAdded(Akonadi::Item)), this, SLOT(onItemAdded(Akonadi::Item)));
    connect(m_monitor.data(), SIGNAL(itemRemoved(Akonadi::Item)), this, SLOT(onItemRemoved(Akonadi::Item)));
    connect(m_monitor.data(), SIGNAL(itemChanged(Akonadi::Item)), this, SLOT(onItemChanged(Akonadi::Item)));
}

ContextQueries::ContextResult::Ptr ContextQueries::findAll() const
{
    if (!m_findAll) {
        {
            ContextQueries *self = const_cast<ContextQueries*>(this);
            self->m_findAll = self->createContextQuery();
        }

        m_findAll->setFetchFunction([this] (const ContextQuery::AddFunction &add) {
            TagFetchJobInterface *job = m_storage->fetchTags();
            Utils::JobHandler::install(job->kjob(), [this, job, add] {
                for (Akonadi::Tag tag : job->tags())
                    add(tag);
            });
        });
        m_findAll->setPredicateFunction([this] (const Akonadi::Tag &tag) {
            return tag.type() == Akonadi::SerializerInterface::contextTagType();
        });
    }

    return m_findAll->result();
}

ContextQueries::TaskResult::Ptr ContextQueries::findTopLevelTasks(Domain::Context::Ptr context) const
{
    Akonadi::Tag tag = m_serializer->createTagFromContext(context);
    if (!m_findToplevel.contains(tag.id())) {
        TaskQuery::Ptr query;

        {
            ContextQueries *self = const_cast<ContextQueries*>(this);
            query = self->createTaskQuery();
            self->m_findToplevel.insert(tag.id(), query);
        }

        query->setFetchFunction([this, tag] (const TaskQuery::AddFunction &add) {
            ItemFetchJobInterface *job = m_storage->fetchTagItems(tag);
            Utils::JobHandler::install(job->kjob(), [this, job, add] {
                if (job->kjob()->error() != KJob::NoError)
                    return;

                for (auto item : job->items())
                    add(item);
            });

        });
        query->setPredicateFunction([this, context] (const Akonadi::Item &item) {
            return m_serializer->isContextChild(context, item);
        });
    }

    return m_findToplevel.value(tag.id())->result();
}

void ContextQueries::onTagAdded(const Tag &tag)
{
    foreach (const auto &query, m_tagInputQueries)
        query->onAdded(tag);
}

void ContextQueries::onTagRemoved(const Tag &tag)
{
    foreach (const auto &query, m_tagInputQueries)
        query->onRemoved(tag);
}

void ContextQueries::onTagChanged(const Tag &tag)
{
    foreach (const auto &query, m_tagInputQueries)
        query->onChanged(tag);
}

void ContextQueries::onItemAdded(const Item &item)
{
    foreach (const auto &query, m_itemInputQueries)
        query->onAdded(item);
}

void ContextQueries::onItemRemoved(const Item &item)
{
    foreach (const auto &query, m_itemInputQueries)
        query->onRemoved(item);
}

void ContextQueries::onItemChanged(const Item &item)
{
    foreach (const auto &query, m_itemInputQueries)
        query->onChanged(item);
}

ContextQueries::ContextQuery::Ptr ContextQueries::createContextQuery()
{
    auto query = ContextQuery::Ptr::create();

    query->setConvertFunction(std::bind(&SerializerInterface::createContextFromTag, m_serializer, _1));
    query->setUpdateFunction(std::bind(&SerializerInterface::updateContextFromTag, m_serializer, _2, _1));
    query->setRepresentsFunction(std::bind(&SerializerInterface::isContextTag, m_serializer, _2, _1));

    m_tagInputQueries << query;
    return query;
}

ContextQueries::TaskQuery::Ptr ContextQueries::createTaskQuery()
{
    auto query = TaskQuery::Ptr::create();

    query->setConvertFunction(std::bind(&SerializerInterface::createTaskFromItem, m_serializer, _1));
    query->setUpdateFunction(std::bind(&SerializerInterface::updateTaskFromItem, m_serializer, _2, _1));
    query->setRepresentsFunction(std::bind(&SerializerInterface::representsItem, m_serializer, _2, _1));

    m_itemInputQueries << query;
    return query;
}
