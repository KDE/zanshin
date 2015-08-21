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


#include "akonadicollectionfetchjobinterface.h"
#include "akonadiitemfetchjobinterface.h"
#include "akonaditagfetchjobinterface.h"

#include "utils/jobhandler.h"

#include <functional>

using namespace std::placeholders;

using namespace Akonadi;

TagQueries::TagQueries(const StorageInterface::Ptr &storage, const SerializerInterface::Ptr &serializer, const MonitorInterface::Ptr &monitor)
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
        m_findAll->setPredicateFunction([this] (const Akonadi::Tag &akonadiTag) {
            return akonadiTag.type() == Akonadi::Tag::PLAIN;
        });
    }

    return m_findAll->result();
}

TagQueries::ArtifactResult::Ptr TagQueries::findTopLevelArtifacts(Domain::Tag::Ptr tag) const
{
    Akonadi::Tag akonadiTag = m_serializer->createAkonadiTagFromTag(tag);
    if (!m_findTopLevel.contains(akonadiTag.id())) {
        ArtifactQuery::Ptr query;
        {
            TagQueries *self = const_cast<TagQueries*>(this);
            query = self->createArtifactQuery();
            self->m_findTopLevel.insert(akonadiTag.id(), query);
        }

        query->setFetchFunction([this, akonadiTag] (const ArtifactQuery::AddFunction &add) {
            CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                           StorageInterface::Recursive,
                                                                           StorageInterface::Tasks | StorageInterface::Notes);
            Utils::JobHandler::install(job->kjob(), [this, job, add] {
                if (job->kjob()->error() != KJob::NoError)
                    return;

                for (auto collection : job->collections()) {
                    ItemFetchJobInterface *job = m_storage->fetchItems(collection);
                    Utils::JobHandler::install(job->kjob(), [this, job, add] {
                        if (job->kjob()->error() != KJob::NoError)
                            return;

                        for (auto item : job->items())
                            add(item);
                    });
                }
            });
        });
        query->setPredicateFunction([this, tag] (const Akonadi::Item &item) {
            return m_serializer->isTagChild(tag, item);
        });
    }

    return m_findTopLevel.value(akonadiTag.id())->result();
}

void TagQueries::onTagAdded(const Tag &tag)
{
    foreach (const TagQuery::Ptr &query, m_tagQueries)
        query->onAdded(tag);
}

void TagQueries::onTagRemoved(const Tag &tag)
{
    foreach (const TagQuery::Ptr &query, m_tagQueries)
        query->onRemoved(tag);
}

void TagQueries::onTagChanged(const Tag &tag)
{
    foreach (const TagQuery::Ptr &query, m_tagQueries)
        query->onChanged(tag);
}

void TagQueries::onItemAdded(const Item &item)
{
    foreach (const ArtifactQuery::Ptr &query, m_artifactQueries)
        query->onAdded(item);
}

void TagQueries::onItemRemoved(const Item &item)
{
    foreach (const ArtifactQuery::Ptr &query, m_artifactQueries)
        query->onRemoved(item);
}

void TagQueries::onItemChanged(const Item &item)
{
    foreach (const ArtifactQuery::Ptr &query, m_artifactQueries)
        query->onChanged(item);
}

TagQueries::TagQuery::Ptr TagQueries::createTagQuery()
{
    auto query = TagQueries::TagQuery::Ptr::create();

    query->setConvertFunction(std::bind(&SerializerInterface::createTagFromAkonadiTag, m_serializer, _1));
    query->setUpdateFunction(std::bind(&SerializerInterface::updateTagFromAkonadiTag, m_serializer, _2, _1));
    query->setRepresentsFunction(std::bind(&SerializerInterface::representsAkonadiTag, m_serializer, _2, _1));

    m_tagQueries << query;
    return query;
}

TagQueries::ArtifactQuery::Ptr TagQueries::createArtifactQuery()
{
    auto query = TagQueries::ArtifactQuery::Ptr::create();

    query->setConvertFunction(std::bind(&SerializerInterface::createArtifactFromItem, m_serializer, _1));
    query->setUpdateFunction(std::bind(&SerializerInterface::updateArtifactFromItem, m_serializer, _2, _1));
    query->setRepresentsFunction(std::bind(&SerializerInterface::representsItem, m_serializer, _2, _1));

    m_artifactQueries << query;
    return query;
}
