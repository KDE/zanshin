/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
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

#ifndef AKONADI_TAGQUERIES_H
#define AKONADI_TAGQUERIES_H

#include "domain/tagqueries.h"

#include <Akonadi/Item>

#include "akonadi/akonadimonitorinterface.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

#include "domain/livequery.h"

namespace Akonadi {

class MonitorInterface;
class SerializerInterface;
class StorageInterface;

class TagQueries : public QObject, public Domain::TagQueries
{
    Q_OBJECT
public:
    typedef QSharedPointer<TagQueries> Ptr;

    typedef Domain::LiveQuery<Akonadi::Tag, Domain::Tag::Ptr> TagQuery;
    typedef Domain::QueryResult<Domain::Tag::Ptr> TagResult;
    typedef Domain::QueryResultProvider<Domain::Tag::Ptr> TagProvider;

    typedef Domain::LiveQuery<Akonadi::Item, Domain::Artifact::Ptr> ArtifactQuery;
    typedef Domain::QueryResultProvider<Domain::Artifact::Ptr> ArtifactProvider;
    typedef Domain::QueryResult<Domain::Artifact::Ptr> ArtifactResult;

    TagQueries(const StorageInterface::Ptr &storage,
               const SerializerInterface::Ptr &serializer,
               const MonitorInterface::Ptr &monitor);

    TagResult::Ptr findAll() const;
    ArtifactResult::Ptr findTopLevelArtifacts(Domain::Tag::Ptr tag) const;

private slots:
    void onTagAdded(const Akonadi::Tag &tag);
    void onTagRemoved(const Akonadi::Tag &tag);
    void onTagChanged(const Akonadi::Tag &tag);

    void onItemAdded(const Akonadi::Item &item);
    void onItemRemoved(const Akonadi::Item &item);
    void onItemChanged(const Akonadi::Item &item);

private:
    TagQuery::Ptr createTagQuery();
    ArtifactQuery::Ptr createArtifactQuery();

    StorageInterface::Ptr m_storage;
    SerializerInterface::Ptr m_serializer;
    MonitorInterface::Ptr m_monitor;

    TagQuery::Ptr m_findAll;
    TagQuery::List m_tagQueries;

    QHash<Akonadi::Tag::Id, ArtifactQuery::Ptr> m_findTopLevel;
    ArtifactQuery::List m_artifactQueries;
};

} // akonadi namespace

#endif // AKONADI_TAGQUERIES_H
