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


#include "reparentingstrategy.h"
#include "globaldefs.h"
#include "reparentingmodel.h"
#include "todonode.h"
#include <klocalizedstring.h>
#include <KIcon>

ReparentingStrategy::ReparentingStrategy()
:   mReparentOnRemoval(true),
    mIdCounter(0)
{

}


IdList ReparentingStrategy::getParents(const QModelIndex & )
{
    return IdList();
}

void ReparentingStrategy::onNodeRemoval(const qint64& changed)
{

}

void ReparentingStrategy::setModel(ReparentingModel* model)
{
    m_model = model;
}

Id ReparentingStrategy::getNextId()
{
    return mIdCounter++;
}

TodoNode *ReparentingStrategy::createNode(Id id, Id pid, QString name)
{
    return m_model->createNode(id, pid, name);
}

bool ReparentingStrategy::reparentOnRemoval() const
{
    return mReparentOnRemoval;
}



TestReparentingStrategy::TestReparentingStrategy()
: ReparentingStrategy()
{

}

Id TestReparentingStrategy::getId(const QModelIndex &sourceChildIndex)
{
    if (!sourceChildIndex.data(IdRole).isValid()) {
        kWarning() << "error: missing idRole";
    }
    return sourceChildIndex.data(IdRole).value<Id>();
}

IdList TestReparentingStrategy::getParents(const QModelIndex &sourceChildIndex)
{
    if (!sourceChildIndex.isValid()) {
        kWarning() << "invalid index";
        return IdList();
    }

    if (!sourceChildIndex.data(ParentRole).isValid()) {
        return IdList();
    }
    const Id &parent = sourceChildIndex.data(ParentRole).value<Id>();
    if (parent < 0) {
        return IdList();
    }

    return IdList() << parent;
}



ProjectStrategy::ProjectStrategy()
:   ReparentingStrategy()
{
    mIdCounter = 2;
    mReparentOnRemoval = false;
}

void ProjectStrategy::init()
{
    ReparentingStrategy::init();
    TodoNode *node = createNode(mInbox, -1, "Inbox");
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

IdList ProjectStrategy::getParents(const QModelIndex &sourceChildIndex)
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
    mIdCounter = 2;
    mUidMapping.clear();
    mCollectionMapping.clear();
    
}


