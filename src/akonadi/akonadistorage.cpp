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


#include "akonadistorage.h"

#include <algorithm>

#include <KCalCore/Todo>

#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/Notes/NoteUtils>

#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonadistoragesettings.h"

using namespace Akonadi;

class CollectionJob : public CollectionFetchJob, public CollectionFetchJobInterface
{
public:
    using CollectionFetchJob::CollectionFetchJob;

    Collection::List collections() const
    {
        auto collections = CollectionFetchJob::collections();
        // Why the hell isn't fetchScope() const and returning a reference???
        auto self = const_cast<CollectionJob*>(this);
        const auto allowedMimeTypes = self->fetchScope().contentMimeTypes().toSet();

        collections.erase(std::remove_if(collections.begin(), collections.end(),
                                         [allowedMimeTypes] (const Collection &collection) {
                                            auto mimeTypes = collection.contentMimeTypes().toSet();
                                            return mimeTypes.intersect(allowedMimeTypes).isEmpty();
                                         }),
                          collections.end());

        return collections;
    }
};

class ItemJob : public ItemFetchJob, public ItemFetchJobInterface
{
public:
    using ItemFetchJob::ItemFetchJob;

    Item::List items() const { return ItemFetchJob::items(); }
};

Storage::Storage()
{
}

Storage::~Storage()
{
}

Collection Storage::defaultTaskCollection()
{
    return StorageSettings::instance().defaultTaskCollection();
}

KJob *Storage::createItem(Item item, Collection collection)
{
    return new ItemCreateJob(item, collection);
}

KJob *Storage::updateItem(Item item)
{
    return new ItemModifyJob(item);
}

CollectionFetchJobInterface *Storage::fetchCollections(Collection collection, StorageInterface::FetchDepth depth)
{
    auto job = new CollectionJob(collection, jobTypeFromDepth(depth));

    auto scope = job->fetchScope();
    scope.setContentMimeTypes({ KCalCore::Todo::todoMimeType(), NoteUtils::noteMimeType() });
    scope.setIncludeStatistics(true);
    scope.setAncestorRetrieval(CollectionFetchScope::All);
    job->setFetchScope(scope);

    return job;
}

ItemFetchJobInterface *Storage::fetchItems(Collection collection)
{
    auto job = new ItemJob(collection);

    auto scope = job->fetchScope();
    scope.fetchFullPayload();
    scope.fetchAllAttributes();
    scope.setAncestorRetrieval(ItemFetchScope::All);
    job->setFetchScope(scope);

    return job;
}

CollectionFetchJob::Type Storage::jobTypeFromDepth(StorageInterface::FetchDepth depth)
{
    auto jobType = CollectionJob::Base;

    switch (depth) {
    case Base:
        jobType = CollectionJob::Base;
        break;
    case FirstLevel:
        jobType = CollectionJob::FirstLevel;
        break;
    case Recursive:
        jobType = CollectionJob::Recursive;
        break;
    default:
        qFatal("Unexpected enum value");
        break;
    }

    return jobType;
}
