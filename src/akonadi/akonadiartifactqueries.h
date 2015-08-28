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

#ifndef AKONADI_ARTIFACTQUERIES_H
#define AKONADI_ARTIFACTQUERIES_H

#include "domain/artifactqueries.h"

#include "akonadi/akonadilivequeryintegrator.h"
#include "akonadi/akonadistorageinterface.h"

namespace Akonadi {

class ArtifactQueries : public Domain::ArtifactQueries
{
public:
    typedef QSharedPointer<ArtifactQueries> Ptr;

    typedef Domain::LiveQueryInput<Akonadi::Item> ItemInputQuery;
    typedef Domain::LiveQueryOutput<Domain::Artifact::Ptr> ArtifactQueryOutput;
    typedef Domain::QueryResultProvider<Domain::Artifact::Ptr> ArtifactProvider;
    typedef Domain::QueryResult<Domain::Artifact::Ptr> ArtifactResult;

    typedef Domain::QueryResultProvider<Domain::Tag::Ptr> TagProvider;
    typedef Domain::QueryResult<Domain::Tag::Ptr> TagResult;

    ArtifactQueries(const StorageInterface::Ptr &storage,
                    const SerializerInterface::Ptr &serializer,
                    const MonitorInterface::Ptr &monitor);

    ArtifactResult::Ptr findInboxTopLevel() const Q_DECL_OVERRIDE;
    TagResult::Ptr findTags(Domain::Artifact::Ptr artifact) const Q_DECL_OVERRIDE;

private:
    StorageInterface::Ptr m_storage;
    SerializerInterface::Ptr m_serializer;
    LiveQueryIntegrator::Ptr m_integrator;

    mutable ArtifactQueryOutput::Ptr m_findInbox;
};

}

#endif // AKONADI_ARTIFACTQUERIES_H
