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
#include "reparentingmodel.h"
#include <globaldefs.h>
#include <todonode.h>
#include <todohelpers.h>
#include <KLocalizedString>
#include <KIcon>
#include <QMimeData>
#include <KCalCore/Todo>
#include <KUrl>

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
    Q_ASSERT(!uid.isEmpty());
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

Qt::ItemFlags ProjectStrategy::flags(const QModelIndex& index, Qt::ItemFlags existingFlags)
{
    Zanshin::ItemType type = (Zanshin::ItemType)index.data(Zanshin::ItemTypeRole).toInt();

    if (type == Zanshin::Inbox) {
        return Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    }

    Qt::ItemFlags flags = m_model->sourceModel()->flags(m_model->mapToSource(index));
    Akonadi::Collection collection;

    if (type==Zanshin::Collection) {
        collection = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();

    } else if (type==Zanshin::ProjectTodo) {
        // We use ParentCollectionRole instead of Akonadi::Item::parentCollection() because the
        // information about the rights is not valid on retrieved items.
        collection = index.data(Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
    }

    if (!(collection.rights() & Akonadi::Collection::CanCreateItem)) {
        flags&= ~Qt::ItemIsDropEnabled;
    } else {
        flags|= Qt::ItemIsDropEnabled;
    }

    return flags;
}

bool ProjectStrategy::onDropMimeData(Id id, const QMimeData* mimeData, Qt::DropAction action)
{
    if (action != Qt::MoveAction || !KUrl::List::canDecode(mimeData)) {
        return false;
    }

    KUrl::List urls = KUrl::List::fromMimeData(mimeData);

    Akonadi::Collection collection;
    Zanshin::ItemType parentType = (Zanshin::ItemType)getData(id, Zanshin::ItemTypeRole).toInt();
    if (parentType == Zanshin::Collection) {
        collection = getData(id, Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    } else {
        const Akonadi::Item parentItem = getData(id, Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
        collection = parentItem.parentCollection();
    }

    QString parentUid = getData(id, Zanshin::UidRole).toString();

    foreach (const KUrl &url, urls) {
        const Akonadi::Item urlItem = Akonadi::Item::fromUrl(url);
        if (urlItem.isValid()) {
            Akonadi::Item item = TodoHelpers::fetchFullItem(urlItem);

            if (!item.isValid()) {
                return false;
            }

            if (item.hasPayload<KCalCore::Todo::Ptr>()) {
                TodoHelpers::moveTodoToProject(item, parentUid, parentType, collection);
            }
        }
    }

    return true;
}

