/* This file is part of Zanshin

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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

#ifndef AKONADI_LIVEQUERYHELPERS_H
#define AKONADI_LIVEQUERYHELPERS_H

#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

#include "domain/livequery.h"

namespace Akonadi {

class LiveQueryHelpers
{
public:
    typedef QSharedPointer<LiveQueryHelpers> Ptr;

    typedef Domain::LiveQueryInput<Collection>::FetchFunction CollectionFetchFunction;
    typedef Domain::LiveQueryInput<Item>::FetchFunction ItemFetchFunction;
    typedef Domain::LiveQueryInput<Tag>::FetchFunction TagFetchFunction;

    LiveQueryHelpers(const SerializerInterface::Ptr &serializer,
                     const StorageInterface::Ptr &storage);

    CollectionFetchFunction fetchAllCollections(StorageInterface::FetchContentTypes contentTypes) const;
    CollectionFetchFunction fetchCollections(const Collection &root, StorageInterface::FetchContentTypes contentTypes) const;

    ItemFetchFunction fetchItems(StorageInterface::FetchContentTypes contentTypes) const;
    ItemFetchFunction fetchItems(const Collection &collection) const;
    ItemFetchFunction fetchItems(const Tag &tag) const;

    ItemFetchFunction fetchSiblings(const Item &item) const;

    TagFetchFunction fetchTags() const;

private:
    SerializerInterface::Ptr m_serializer;
    StorageInterface::Ptr m_storage;
};

}

#endif // AKONADI_LIVEQUERYHELPERS_H
