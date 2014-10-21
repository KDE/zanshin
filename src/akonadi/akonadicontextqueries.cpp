/* This file is part of Zanshin

   Copyright 2014 Franck Arrecot <franck.arrecot@gmail.com>

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
#include "akonadimonitorimpl.h"
#include "akonadiserializer.h"
#include "akonadistorage.h"

#include "utils/jobhandler.h"
#include <QByteArray>
#include <QDebug>

using namespace Akonadi;

ContextQueries::ContextQueries(QObject *parent)
    : QObject(parent),
      m_storage(new Storage),
      m_serializer(new Serializer),
      m_monitor(new MonitorImpl),
      m_ownInterfaces(true)
{
    connect(m_monitor, SIGNAL(tagAdded(Akonadi::Tag)), this, SLOT(onTagAdded(Akonadi::Tag)));
    connect(m_monitor, SIGNAL(tagRemoved(Akonadi::Tag)), this, SLOT(onTagRemoved(Akonadi::Tag)));
    connect(m_monitor, SIGNAL(tagChanged(Akonadi::Tag)), this, SLOT(onTagChanged(Akonadi::Tag)));
}

ContextQueries::ContextQueries(StorageInterface *storage, SerializerInterface *serializer, MonitorInterface *monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_monitor(monitor),
      m_ownInterfaces(false)
{
    connect(m_monitor, SIGNAL(tagAdded(Akonadi::Tag)), this, SLOT(onTagAdded(Akonadi::Tag)));
    connect(m_monitor, SIGNAL(tagRemoved(Akonadi::Tag)), this, SLOT(onTagRemoved(Akonadi::Tag)));
    connect(m_monitor, SIGNAL(tagChanged(Akonadi::Tag)), this, SLOT(onTagChanged(Akonadi::Tag)));
}

ContextQueries::~ContextQueries()
{
    if (m_ownInterfaces) {
        delete m_storage;
        delete m_serializer;
        delete m_monitor;
    }
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

        m_findAll->setConvertFunction([this] (const Akonadi::Tag &tag) {
            return m_serializer->createContextFromTag(tag);
        });

        m_findAll->setUpdateFunction([this] (const Akonadi::Tag &tag, Domain::Context::Ptr &context) {
            m_serializer->updateContextFromTag(context, tag);
        });
        m_findAll->setPredicateFunction([this] (const Akonadi::Tag &tag) {
            return tag.type() == Akonadi::Serializer::contextTagType();
        });
        m_findAll->setRepresentsFunction([this] (const Akonadi::Tag &tag, const Domain::Context::Ptr &context) {
            return m_serializer->isContextTag(context, tag);
        });
    }

    return m_findAll->result();
}

ContextQueries::TaskResult::Ptr ContextQueries::findTasks(Domain::Context::Ptr context) const
{
    qFatal("Not implemented yet");
    Q_UNUSED(context);
    return TaskResult::Ptr();
}

void ContextQueries::onTagAdded(const Tag &tag)
{
    foreach (const ContextQuery::Ptr &query, m_contextQueries)
        query->onAdded(tag);
}

void ContextQueries::onTagRemoved(const Tag &tag)
{
    foreach (const ContextQuery::Ptr &query, m_contextQueries)
        query->onRemoved(tag);
}

void ContextQueries::onTagChanged(const Tag &tag)
{
    foreach (const ContextQuery::Ptr &query, m_contextQueries)
        query->onChanged(tag);
}

ContextQueries::ContextQuery::Ptr ContextQueries::createContextQuery()
{
    auto query = ContextQuery::Ptr::create();
    m_contextQueries << query;
    return query;
}
