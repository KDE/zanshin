/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Christian Mollekopf <chrigi_1@fastmail.fm>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "projectstrategy.h"
#include <globaldefs.h>
#include <todonode.h>
#include <KLocalizedString>
#include <KIcon>

ProjectStrategy::ProjectStrategy()
:   ReparentingStrategy(),
    mInbox(1)
{
    mReparentOnRemoval = false;
}

void ProjectStrategy::init()
{
    ReparentingStrategy::init();
    //FIXME we should be setting this earlier, here the inserted signals have already been emitted
    TodoNode *node = createNode(mInbox, IdList(), "Inbox");
    node->setData(i18n("Inbox"), 0, Qt::DisplayRole);
    node->setData(KIcon("mail-folder-inbox"), 0, Qt::DecorationRole);
    node->setRowData(Zanshin::Inbox, Zanshin::ItemTypeRole);
}


Id ProjectStrategy::getId(const QModelIndex &sourceChildIndex)
{
    Zanshin::ItemType type = (Zanshin::ItemType) sourceChildIndex.data(Zanshin::ItemTypeRole).toInt();
    if (type==Zanshin::Collection) {
        Akonadi::Collection::Id id = sourceChildIndex.data(Akonadi::EntityTreeModel::CollectionIdRole).value<Akonadi::Collection::Id>();
        if (!mCollectionMapping.contains(id)) {
            mCollectionMapping.insert(id, getNextId());
        }
//         kDebug() << "collection id: " << id << mCollectionMapping.value(id);
        return mCollectionMapping.value(id);
    }
    const QString &uid = sourceChildIndex.data(Zanshin::UidRole).toString();
    if (!mUidMapping.contains(uid)) {
        mUidMapping.insert(uid, getNextId());
    }
    return mUidMapping.value(uid);
}

IdList ProjectStrategy::getParents(const QModelIndex &sourceChildIndex, const IdList &ignore)
{
    Id id = getId(sourceChildIndex);
    Zanshin::ItemType type = (Zanshin::ItemType) sourceChildIndex.data(Zanshin::ItemTypeRole).toInt();
//     kDebug() << id << type;
    if (type==Zanshin::Collection) {
        const QModelIndex &parent = sourceChildIndex.parent();
        if (parent.isValid()) {
            return IdList() << getId(parent);
        }
        return IdList();
    }
    const QString &parentUid = sourceChildIndex.data(Zanshin::ParentUidRole).toString();
//     kDebug() << parentUid;
    if (type==Zanshin::ProjectTodo && parentUid.isEmpty()) {
//         kDebug() << "get source parent";
        const QModelIndex &parent = sourceChildIndex.parent();
        if (parent.isValid()) {
            return IdList() << getId(parent);
        }
        return IdList();
    } else if (type==Zanshin::StandardTodo && parentUid.isEmpty()) {
        return IdList() << mInbox;
    }
//     Q_ASSERT(type==Zanshin::StandardTodo);
    if (!mUidMapping.contains(parentUid)) {
        mUidMapping.insert(parentUid, getNextId());

    }
    return IdList() << mUidMapping.value(parentUid);
}

void ProjectStrategy::reset()
{
    mUidMapping.clear();
    mCollectionMapping.clear();
    ReparentingStrategy::reset();
}
