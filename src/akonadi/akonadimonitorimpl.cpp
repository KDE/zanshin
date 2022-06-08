/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "akonadimonitorimpl.h"

#include <KCalCore/Todo>

#include <Akonadi/AttributeFactory>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Monitor>

#include "akonadi/akonadiapplicationselectedattribute.h"
#include "akonadi/akonaditimestampattribute.h"

using namespace Akonadi;

namespace
{
    template<typename T>
    QSet<T> listToSet(const QList<T> &list)
    {
        return {list.cbegin(), list.cend()};
    }
}

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
        Q_EMIT collectionChanged(collection);

    if (parts.contains("ZanshinSelected")
     && hasSupportedMimeTypes(collection)) {
        Q_EMIT collectionSelectionChanged(collection);
    }
}

bool MonitorImpl::hasSupportedMimeTypes(const Collection &collection)
{
    QSet<QString> mimeIntersection = listToSet(m_monitor->mimeTypesMonitored());
    mimeIntersection.intersect(listToSet(collection.contentMimeTypes()));
    return !mimeIntersection.isEmpty();
}
