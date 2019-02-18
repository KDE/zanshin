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


#include "akonadimonitorimpl.h"

#include <KCalCore/Todo>

#include <AkonadiCore/AttributeFactory>
#include <AkonadiCore/CollectionFetchScope>
#include <AkonadiCore/ItemFetchScope>
#include <AkonadiCore/Monitor>

#include "akonadi/akonadiapplicationselectedattribute.h"
#include "akonadi/akonaditimestampattribute.h"

using namespace Akonadi;

MonitorImpl::MonitorImpl()
    : m_monitor(new Akonadi::Monitor(this))
{
    AttributeFactory::registerAttribute<ApplicationSelectedAttribute>();
    AttributeFactory::registerAttribute<TimestampAttribute>();

    m_monitor->fetchCollection(true);
    m_monitor->setCollectionMonitored(Akonadi::Collection::root());

    m_monitor->setMimeTypeMonitored(KCalCore::Todo::todoMimeType());

    auto collectionScope = m_monitor->collectionFetchScope();
    collectionScope.setContentMimeTypes(m_monitor->mimeTypesMonitored());
    collectionScope.setIncludeStatistics(true);
    collectionScope.setAncestorRetrieval(CollectionFetchScope::All);
    m_monitor->setCollectionFetchScope(collectionScope);

    connect(m_monitor, &Akonadi::Monitor::collectionAdded, this, &MonitorImpl::collectionAdded);
    connect(m_monitor, &Akonadi::Monitor::collectionRemoved, this, &MonitorImpl::collectionRemoved);
    connect(m_monitor, static_cast<void(Akonadi::Monitor::*)(const Collection &, const QSet<QByteArray> &)>(&Akonadi::Monitor::collectionChanged),
            this, &MonitorImpl::onCollectionChanged);

    auto itemScope = m_monitor->itemFetchScope();
    itemScope.fetchFullPayload();
    itemScope.fetchAllAttributes();
    itemScope.setFetchTags(false);
    itemScope.setAncestorRetrieval(ItemFetchScope::All);
    m_monitor->setItemFetchScope(itemScope);

    connect(m_monitor, &Akonadi::Monitor::itemAdded, this, &MonitorImpl::itemAdded);
    connect(m_monitor, &Akonadi::Monitor::itemRemoved, this, &MonitorImpl::itemRemoved);
    connect(m_monitor, &Akonadi::Monitor::itemChanged, this, &MonitorImpl::itemChanged);
    connect(m_monitor, &Akonadi::Monitor::itemMoved, this, &MonitorImpl::itemMoved);
}

MonitorImpl::~MonitorImpl()
{
}

void MonitorImpl::onCollectionChanged(const Collection &collection, const QSet<QByteArray> &parts)
{
    // Will probably need to be expanded and to also fetch the full parent chain before emitting in some cases
    static const QSet<QByteArray> allowedParts = QSet<QByteArray>() << "NAME"
                                                                    << "REMOTEID"
                                                                    << "AccessRights"
                                                                    << "ENTITYDISPLAY"
                                                                    << "ZanshinSelected"
                                                                    << "ZanshinTimestamp";

    QSet<QByteArray> partsIntersection = parts;
    partsIntersection.intersect(allowedParts);
    if (!partsIntersection.isEmpty())
        emit collectionChanged(collection);

    if (parts.contains("ZanshinSelected")
     && hasSupportedMimeTypes(collection)) {
        emit collectionSelectionChanged(collection);
    }
}

bool MonitorImpl::hasSupportedMimeTypes(const Collection &collection)
{
    QSet<QString> mimeIntersection = m_monitor->mimeTypesMonitored().toSet();
    mimeIntersection.intersect(collection.contentMimeTypes().toSet());
    return !mimeIntersection.isEmpty();
}
