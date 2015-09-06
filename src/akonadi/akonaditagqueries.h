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

#include "akonadi/akonadilivequeryhelpers.h"
#include "akonadi/akonadilivequeryintegrator.h"

namespace Akonadi {

class TagQueries : public Domain::TagQueries
{
public:
    typedef QSharedPointer<TagQueries> Ptr;

    typedef Domain::LiveQueryInput<Akonadi::Tag> TagInputQuery;
    typedef Domain::LiveQueryOutput<Domain::Tag::Ptr> TagQueryOutput;
    typedef Domain::QueryResult<Domain::Tag::Ptr> TagResult;
    typedef Domain::QueryResultProvider<Domain::Tag::Ptr> TagProvider;

    typedef Domain::LiveQueryInput<Akonadi::Item> ItemInputQuery;
    typedef Domain::LiveQueryOutput<Domain::Artifact::Ptr> ArtifactQueryOutput;
    typedef Domain::QueryResultProvider<Domain::Artifact::Ptr> ArtifactProvider;
    typedef Domain::QueryResult<Domain::Artifact::Ptr> ArtifactResult;

    TagQueries(const StorageInterface::Ptr &storage,
               const SerializerInterface::Ptr &serializer,
               const MonitorInterface::Ptr &monitor);

    TagResult::Ptr findAll() const Q_DECL_OVERRIDE;
    ArtifactResult::Ptr findTopLevelArtifacts(Domain::Tag::Ptr tag) const Q_DECL_OVERRIDE;

private:
    SerializerInterface::Ptr m_serializer;
    LiveQueryHelpers::Ptr m_helpers;
    LiveQueryIntegrator::Ptr m_integrator;

    mutable TagQueryOutput::Ptr m_findAll;
    mutable QHash<Akonadi::Tag::Id, ArtifactQueryOutput::Ptr> m_findTopLevel;
};

} // akonadi namespace

#endif // AKONADI_TAGQUERIES_H
