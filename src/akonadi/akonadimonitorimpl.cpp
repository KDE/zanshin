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
#include <Akonadi/Notes/NoteUtils>
#include <AkonadiCore/TagFetchScope>

#include "akonadi/akonadiapplicationselectedattribute.h"
#include "akonadi/akonaditimestampattribute.h"

using namespace Akonadi;

MonitorImpl::MonitorImpl()
    : m_monitor(new Akonadi::Monitor)
{
    AttributeFactory::registerAttribute<ApplicationSelectedAttribute>();
    AttributeFactory::registerAttribute<TimestampAttribute>();

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
    connect(m_monitor, SIGNAL(collectionChanged(Akonadi::Collection,QSet<QByteArray>)), this, SLOT(onCollectionChanged(Akonadi::Collection,QSet<QByteArray>)));

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
    connect(m_monitor, SIGNAL(itemsTagsChanged(Akonadi::Item::List, QSet<Akonadi::Tag>, QSet<Akonadi::Tag>)), this, SLOT(onItemsTagsChanged(Akonadi::Item::List,QSet<Akonadi::Tag>,QSet<Akonadi::Tag>)));

    connect(m_monitor, SIGNAL(tagAdded(Akonadi::Tag)), this, SIGNAL(tagAdded(Akonadi::Tag)));
    connect(m_monitor, SIGNAL(tagRemoved(Akonadi::Tag)), this, SIGNAL(tagRemoved(Akonadi::Tag)));
    connect(m_monitor, SIGNAL(tagChanged(Akonadi::Tag)), this, SIGNAL(tagChanged(Akonadi::Tag)));
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

void MonitorImpl::onItemsTagsChanged(const Akonadi::Item::List &items, const QSet<Akonadi::Tag> &addedTags, const QSet<Akonadi::Tag> &removedTags)
{
    // Because itemChanged is not emitted on tag removal, we need to listen to itemsTagsChanged and
    // emit the itemChanged only in this case (avoid double emits in case of tag dissociation / association)
    // So if both list are empty it means we are just seeing a tag being removed so we update its related items
    if (addedTags.isEmpty() && removedTags.isEmpty()) {
        foreach (const Item &item, items)
            emit itemChanged(item);
    }
}

bool MonitorImpl::hasSupportedMimeTypes(const Collection &collection)
{
    QSet<QString> mimeIntersection = m_monitor->mimeTypesMonitored().toSet();
    mimeIntersection.intersect(collection.contentMimeTypes().toSet());
    return !mimeIntersection.isEmpty();
}
