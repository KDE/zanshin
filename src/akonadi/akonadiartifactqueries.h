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

#include <QHash>
#include <Akonadi/Item>

#include "domain/artifactqueries.h"

class KJob;

namespace Akonadi {

class Item;
class MonitorInterface;
class SerializerInterface;
class StorageInterface;

class ArtifactQueries : public QObject, public Domain::ArtifactQueries
{
    Q_OBJECT
public:
    typedef Domain::QueryResultProvider<Domain::Artifact::Ptr> ArtifactProvider;
    typedef Domain::QueryResult<Domain::Artifact::Ptr> ArtifactResult;

    explicit ArtifactQueries(QObject *parent = 0);
    ArtifactQueries(StorageInterface *storage, SerializerInterface *serializer, MonitorInterface *monitor);
    virtual ~ArtifactQueries();

    ArtifactResult::Ptr findInboxTopLevel() const;

private slots:
    void onItemAdded(const Akonadi::Item &item);
    void onItemRemoved(const Akonadi::Item &item);
    void onItemChanged(const Akonadi::Item &item);

private:
    bool isInboxItem(const Item &item) const;
    bool isArtifactItem(const Domain::Artifact::Ptr &artifact, const Item &item) const;
    Domain::Artifact::Ptr deserializeArtifact(const Item &item) const;

    StorageInterface *m_storage;
    SerializerInterface *m_serializer;
    MonitorInterface *m_monitor;
    bool m_ownInterfaces;

    mutable ArtifactProvider::WeakPtr m_inboxProvider;
};

}

#endif // AKONADI_ARTIFACTQUERIES_H
