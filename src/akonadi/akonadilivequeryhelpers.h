/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_LIVEQUERYHELPERS_H
#define AKONADI_LIVEQUERYHELPERS_H

#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

#include "domain/livequery.h"
#include "domain/task.h"

namespace Akonadi {

class LiveQueryHelpers
{
public:
    typedef QSharedPointer<LiveQueryHelpers> Ptr;

    typedef Domain::LiveQueryInput<Collection>::FetchFunction CollectionFetchFunction;
    typedef Domain::LiveQueryInput<Item>::FetchFunction ItemFetchFunction;

    LiveQueryHelpers(const SerializerInterface::Ptr &serializer,
                     const StorageInterface::Ptr &storage);

    CollectionFetchFunction fetchAllCollections(QObject *parent) const;
    CollectionFetchFunction fetchCollections(const Collection &root, QObject *parent) const;
    CollectionFetchFunction fetchItemCollection(const Item &item, QObject *parent) const;

    ItemFetchFunction fetchItems(QObject *parent) const;
    ItemFetchFunction fetchItems(const Collection &collection, QObject *parent) const;
    ItemFetchFunction fetchItemsForContext(const Domain::Context::Ptr &context, QObject *parent) const;

    /// Returns a fetch function which calls a LiveQueryInput::AddFunction (provided as argument to the fetch function)
    /// with the given task, then its parent, its grandparent etc. up until the project.
    ItemFetchFunction fetchTaskAndAncestors(Domain::Task::Ptr task, QObject *parent) const;

    ItemFetchFunction fetchSiblings(const Item &item, QObject *parent) const;

private:
    SerializerInterface::Ptr m_serializer;
    StorageInterface::Ptr m_storage;
};

}

#endif // AKONADI_LIVEQUERYHELPERS_H
