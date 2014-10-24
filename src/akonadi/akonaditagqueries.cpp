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

#include "akonaditagqueries.h"

#include "akonaditagfetchjobinterface.h"

#include "akonadimonitorimpl.h"
#include "akonadiserializer.h"
#include "akonadistorage.h"

#include "utils/jobhandler.h"

using namespace Akonadi;

TagQueries::TagQueries(QObject *parent)
    : QObject(parent),
      m_storage(new Storage),
      m_serializer(new Serializer),
      m_monitor(new MonitorImpl),
      m_ownInterfaces(true)
{
    connect(m_monitor, SIGNAL(itemAdded(Akonadi::Item)), this, SLOT(onItemAdded(Akonadi::Item)));
    connect(m_monitor, SIGNAL(itemRemoved(Akonadi::Item)), this, SLOT(onItemRemoved(Akonadi::Item)));
    connect(m_monitor, SIGNAL(itemChanged(Akonadi::Item)), this, SLOT(onItemChanged(Akonadi::Item)));
}

TagQueries::TagQueries(StorageInterface *storage, SerializerInterface *serializer, MonitorInterface *monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_monitor(monitor),
      m_ownInterfaces(false)
{
    connect(monitor, SIGNAL(itemAdded(Akonadi::Item)), this, SLOT(onItemAdded(Akonadi::Item)));
    connect(monitor, SIGNAL(itemRemoved(Akonadi::Item)), this, SLOT(onItemRemoved(Akonadi::Item)));
    connect(monitor, SIGNAL(itemChanged(Akonadi::Item)), this, SLOT(onItemChanged(Akonadi::Item)));
}

TagQueries::~TagQueries()
{
    if (m_ownInterfaces) {
        delete m_storage;
        delete m_serializer;
        delete m_monitor;
    }
}

TagQueries::TagResult::Ptr TagQueries::findAll() const
{
    if (!m_findAll) {
        {
            TagQueries *self = const_cast<TagQueries*>(this);
            self->m_findAll = self->createTagQuery();
        }

        m_findAll->setFetchFunction([this] (const TagQuery::AddFunction &add) {
            TagFetchJobInterface *job = m_storage->fetchTags();
            Utils::JobHandler::install(job->kjob(), [this, job, add] {
                for (Akonadi::Tag tag : job->tags())
                    add(tag);
            });
        });

        m_findAll->setConvertFunction([this] (const Akonadi::Tag &tag) {
            return m_serializer->createTagFromAkonadiTag(tag);
        });

        m_findAll->setUpdateFunction([this] (const Akonadi::Tag &akonadiTag, Domain::Tag::Ptr &tag) {
            m_serializer->updateTagFromAkonadiTag(tag, akonadiTag);
        });
        m_findAll->setPredicateFunction([this] (const Akonadi::Tag &akonadiTag) {
            return akonadiTag.type() == Akonadi::Tag::PLAIN;
        });
        m_findAll->setRepresentsFunction([this] (const Akonadi::Tag &akonadiTag, const Domain::Tag::Ptr &tag) {
            return m_serializer->representsAkonadiTag(tag, akonadiTag);
        });
    }

    return m_findAll->result();
}

TagQueries::ArtifactResult::Ptr TagQueries::findTopLevelArtifacts(Domain::Tag::Ptr tag) const
{
    Q_UNUSED(tag);
    qFatal("TagQueries find TopLevelArtifacts not supported yet !");
    return ArtifactResult::Ptr();
}

void TagQueries::onTagAdded(const Tag &tag)
{
    Q_UNUSED(tag);
}

void TagQueries::onTagRemoved(const Tag &tag)
{
    Q_UNUSED(tag);
}

void TagQueries::onTagChanged(const Tag &tag)
{
    Q_UNUSED(tag);
}

void TagQueries::onItemAdded(const Item &item)
{
    Q_UNUSED(item);
}

void TagQueries::onItemRemoved(const Item &item)
{
    Q_UNUSED(item);
}

void TagQueries::onItemChanged(const Item &item)
{
    Q_UNUSED(item);
}

TagQueries::TagQuery::Ptr TagQueries::createTagQuery()
{
    auto query = TagQueries::TagQuery::Ptr::create();
    m_tagQueries << query;
    return query;
}

TagQueries::ArtifactQuery::Ptr TagQueries::createArtifactQuery()
{
    auto query = TagQueries::ArtifactQuery::Ptr::create();
    m_artifactQueries << query;
    return query;
}
