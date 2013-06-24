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
#include "todohelpers.h"

AkonadiDataStore::AkonadiDataStore()
{
}

AkonadiDataStore::~AkonadiDataStore()
{
}

bool AkonadiDataStore::moveTodoToProject(const PimItemIndex &node, const PimItemIndex &parent)
{
    PimItemIndex::ItemType parentType = parent.type;
    Zanshin::ItemType parentItemType = Zanshin::StandardTodo;
    Akonadi::Collection collection;
    switch (parentType) {
    case PimItemIndex::Inbox:
        parentItemType = Zanshin::Inbox;
        collection = node.item.parentCollection();
        break;
    case PimItemIndex::Collection:
        parentItemType = Zanshin::Collection;
        collection = node.collection;
        break;
    case PimItemIndex::Project:
        parentItemType = Zanshin::ProjectTodo;
    case PimItemIndex::Todo: // Fall through
        collection = parent.item.parentCollection();
        break;
    default:
        qFatal("Unsupported parent type");
        break;
    }

    if (!TodoHelpers::moveTodoToProject(node.item, parent.uid, parentItemType, collection)) {
        return false;
    }

    return true;
}

