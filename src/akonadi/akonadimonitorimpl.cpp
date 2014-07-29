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

#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Monitor>
#include <Akonadi/Notes/NoteUtils>
#include <Akonadi/TagFetchScope>

using namespace Akonadi;

MonitorImpl::MonitorImpl()
    : m_monitor(new Akonadi::Monitor)
{
    m_monitor->fetchCollection(true);
    m_monitor->setCollectionMonitored(Akonadi::Collection::root());

    m_monitor->setMimeTypeMonitored(KCalCore::Todo::todoMimeType());
    m_monitor->setMimeTypeMonitored(NoteUtils::noteMimeType());

    auto collectionScope = m_monitor->collectionFetchScope();
    collectionScope.setContentMimeTypes(m_monitor->mimeTypesMonitored());
    collectionScope.setIncludeStatistics(true);
    collectionScope.setAncestorRetrieval(CollectionFetchScope::All);
    m_monitor->setCollectionFetchScope(collectionScope);

    connect(m_monitor, SIGNAL(collectionAdded(Akonadi::Collection,Akonadi::Collection)), this, SIGNAL(collectionAdded(Akonadi::Collection)));
    connect(m_monitor, SIGNAL(collectionRemoved(Akonadi::Collection)), this, SIGNAL(collectionRemoved(Akonadi::Collection)));
    connect(m_monitor, SIGNAL(collectionChanged(Akonadi::Collection,QSet<QByteArray>)), this, SIGNAL(collectionChanged(Akonadi::Collection)));

    auto itemScope = m_monitor->itemFetchScope();
    itemScope.fetchFullPayload();
    itemScope.fetchAllAttributes();
    itemScope.setFetchTags(true);
    itemScope.tagFetchScope().setFetchIdOnly(false);
    itemScope.setAncestorRetrieval(ItemFetchScope::All);
    m_monitor->setItemFetchScope(itemScope);

    connect(m_monitor, SIGNAL(itemAdded(Akonadi::Item, Akonadi::Collection)), this, SIGNAL(itemAdded(Akonadi::Item)));
    connect(m_monitor, SIGNAL(itemRemoved(Akonadi::Item)), this, SIGNAL(itemRemoved(Akonadi::Item)));
    connect(m_monitor, SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)), this, SIGNAL(itemChanged(Akonadi::Item)));
    connect(m_monitor, SIGNAL(itemMoved(Akonadi::Item,Akonadi::Collection,Akonadi::Collection)), this, SIGNAL(itemMoved(Akonadi::Item)));

    connect(m_monitor, SIGNAL(tagAdded(Akonadi::Tag)), this, SIGNAL(tagAdded(Akonadi::Tag)));
    connect(m_monitor, SIGNAL(tagRemoved(Akonadi::Tag)), this, SIGNAL(tagRemoved(Akonadi::Tag)));
    connect(m_monitor, SIGNAL(tagChanged(Akonadi::Tag)), this, SIGNAL(tagChanged(Akonadi::Tag)));
}

MonitorImpl::~MonitorImpl()
{
}
