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

#ifndef AKONADI_PROJECTQUERIES_H
#define AKONADI_PROJECTQUERIES_H

#include "domain/projectqueries.h"

#include <KDE/Akonadi/Item>

class KJob;

namespace Akonadi {

class MonitorInterface;
class SerializerInterface;
class StorageInterface;

class ProjectQueries : public QObject, public Domain::ProjectQueries
{
    Q_OBJECT
public:
    typedef Domain::QueryResultProvider<Domain::Project::Ptr> ProjectProvider;
    typedef Domain::QueryResult<Domain::Project::Ptr> ProjectResult;

    typedef Domain::QueryResultProvider<Domain::Artifact::Ptr> ArtifactProvider;
    typedef Domain::QueryResult<Domain::Artifact::Ptr> ArtifactResult;

    explicit ProjectQueries(QObject *parent = 0);
    ProjectQueries(StorageInterface *storage, SerializerInterface *serializer, MonitorInterface *monitor);
    virtual ~ProjectQueries();

    ProjectResult::Ptr findAll() const;
    ArtifactResult::Ptr findTopLevelArtifacts(Domain::Project::Ptr project) const;

private slots:
    void onItemAdded(const Akonadi::Item &item);
    void onItemRemoved(const Akonadi::Item &item);
    void onItemChanged(const Akonadi::Item &item);

private:
    void addItemIdInCache(const Domain::Project::Ptr &project, Akonadi::Entity::Id id) const;
    Domain::Artifact::Ptr deserializeArtifact(const Item &item) const;
    ArtifactProvider::Ptr topLevelProviderForUid(const QString &uid) const;
    void removeItemFromTopLevelProviders(const Item &item);

    StorageInterface *m_storage;
    SerializerInterface *m_serializer;
    MonitorInterface *m_monitor;
    bool m_ownInterfaces;

    mutable ProjectProvider::WeakPtr m_projectProvider;
    mutable QHash<Akonadi::Entity::Id, ArtifactProvider::WeakPtr> m_topLevelProviders;
    mutable QHash<QString, Akonadi::Entity::Id> m_uidtoIdCache;
    mutable QHash<Akonadi::Entity::Id, QString> m_idToRelatedUidCache;
};

}

#endif // AKONADI_PROJECTQUERIES_H
