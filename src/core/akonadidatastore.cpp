/* This file is part of Zanshin Todo.

   Copyright 2013 Kevin Ottens <ervin@kde.org>
   Copyright 2013 Mario Bensi <nef@ipsquad.net>

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

#include "akonadidatastore.h"
#include "collectionitem.h"
#include "incidenceitem.h"
#include "noteitem.h"
#include "virtualitem.h"
#include "todohelpers.h"

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <KCalCore/Todo>

template<class T>
typename T::Ptr unwrap(const Akonadi::Item &item)
{
    Q_ASSERT(item.hasPayload<typename T::Ptr>());
    return item.payload< typename T::Ptr>();
}

AkonadiDataStore &AkonadiDataStore::instance()
{
    AkonadiDataStore *store = dynamic_cast<AkonadiDataStore*>(&DataStoreInterface::instance());
    Q_ASSERT(store != 0);
    return *store;
}

AkonadiDataStore::AkonadiDataStore()
{
}

AkonadiDataStore::~AkonadiDataStore()
{
}

bool AkonadiDataStore::isProject(const Akonadi::Item &item) const
{
    const KCalCore::Incidence::Ptr i = unwrap<KCalCore::Incidence>(item);
    if (i->comments().contains("X-Zanshin-Project")
     || !i->customProperty("Zanshin", "Project").isEmpty()) {
        return true;
    }
    return PimItemServices::projectInstance().hasChildren(i->uid());
}

PimItem::Ptr AkonadiDataStore::indexFromUrl(const KUrl &url) const
{
    const Akonadi::Item urlItem = Akonadi::Item::fromUrl(url);
    Q_ASSERT(urlItem.isValid());

    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(urlItem);
    job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    job->fetchScope().fetchFullPayload();
    if ( !job->exec() ) {
        return PimItem::Ptr();
    }

    Q_ASSERT(job->items().size()==1);
    const Akonadi::Item resolvedItem = job->items().first();
    Q_ASSERT(resolvedItem.isValid());

    if (AkonadiBaseItem::typeFromItem(resolvedItem) == PimItem::Todo) {
        return PimItem::Ptr(new IncidenceItem(resolvedItem));
    } else {
        return PimItem::Ptr(new NoteItem(resolvedItem));
    }
}

bool AkonadiDataStore::moveTodoToProject(const PimItem::Ptr &item, const PimItem::Ptr &parent)
{
    Zanshin::ItemType parentItemType = Zanshin::StandardTodo;
    Akonadi::Collection collection;
    switch (parent->itemType()) {
    case PimItem::Inbox:
        parentItemType = Zanshin::Inbox;
        collection = item.dynamicCast<AkonadiBaseItem>()->getItem().parentCollection();
        break;
    case PimItem::Collection:
        parentItemType = Zanshin::Collection;
        collection = parent.dynamicCast<CollectionItem>()->collection();
        break;
    case PimItem::Project:
        parentItemType = Zanshin::ProjectTodo;
    case PimItem::Todo: // Fall through
        collection = parent.dynamicCast<AkonadiBaseItem>()->getItem().parentCollection();
        break;
    default:
        qFatal("Unsupported parent type");
        break;
    }

    if (!TodoHelpers::moveTodoToProject(item.dynamicCast<AkonadiBaseItem>()->getItem(),
                                        parent->uid(), parentItemType, collection)) {
        return false;
    }

    return true;
}

