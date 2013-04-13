/*
 * Copyright (C) 2012  Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pimitemrelationcache.h"
#include "pimitem.h"
#include "pimitemfactory.h"

TreeNode::TreeNode(const QString& n, const Id& i, const QList< TreeNode >& p)
:   name(n),
    id(i),
    parentNodes(p),
    knowsParents(true)
{
}

TreeNode::TreeNode(const QString& n, const Id& i)
:   name(n),
    id(i),
    knowsParents(false)
{
}

Relation::Relation(Id i, const QList< TreeNode >& p)
:   id(i),
    parentNodes(p)
{
}

Relation::Relation()
:   id(-1)
{
}

PimItemRelationCache::PimItemRelationCache()
:   QObject(),
    mIdCounter(1)
{

}

void PimItemRelationCache::mergeNode(const TreeNode &/*node*/)
{
}

Id PimItemRelationCache::addItem(const Akonadi::Item &item)
{
    Q_ASSERT(item.isValid());
    //TODO cache
    
//     kDebug() << pimitem->itemType();
    Id id = getOrCreateItemId(item);
    if (id < 0) {
        return -1;
    }
    const Relation &rel = getRelationTree(id, item);
    Q_ASSERT(rel.id == id);
//     qDebug() << " <<<<<<<<<<<<<<<<< " << item.url().url() << id << rel.id << rel.parentNodes.size();
    mParents.removeLeft(id);
    foreach (const TreeNode &node, rel.parentNodes) {
        Q_ASSERT(id != node.id);
        mParents.insert(id, node.id);
        mergeNode(node);
    }
//     kDebug() << item.id() << mParents.values(id);
    Q_ASSERT(mItemIdCache.contains(item.id()));
    return id;
}

Id PimItemRelationCache::getItemId(const Akonadi::Item &item) const
{
//     kDebug() << item.id();
//     qDebug() << "itemids: " << mItemIdCache;
//     qDebug() << "parents " << mParents;
//     qDebug() << "uids " << mUidMapping;
    Q_ASSERT(item.isValid());
    Q_ASSERT(mItemIdCache.contains(item.id()));
    return mItemIdCache.value(item.id());
}

Id PimItemRelationCache::getOrCreateItemId(const Akonadi::Item &item)
{
    Q_ASSERT(item.isValid());
    if (mItemIdCache.contains(item.id())) {
        return mItemIdCache.value(item.id());
    }
    Id id;
    PimItem::Ptr pimitem(PimItemFactory::getItem(item));
    Q_ASSERT (pimitem);
    const QByteArray uid = pimitem->getUid().toLatin1();
    if (uid.isEmpty()) {
        kWarning() << "empty uid: " << item.id();
        return -1;
    }
    Q_ASSERT(!uid.isEmpty());
    if (mUidMapping.contains(uid)) {
        id = mUidMapping.value(uid);
    } else {
        id = getNextId();
        mUidMapping.insert(uid, id);
    }
    mItemIdCache[item.id()] = id;
//    kDebug() << item.id() << id;
    return id;
}

bool PimItemRelationCache::isVirtual(Id id) const
{
    return !mItemIdCache.values().contains(id);
}

IdList PimItemRelationCache::values(Id key, const PimItemRelationCache::ParentMapping &map) const
{
    IdList parentNodes;
    PimItemRelationCache::ParentMapping::left_const_iterator i = map.constFindLeft(key);
    while (i != map.leftConstEnd() && i.key() == key) {
        parentNodes << i.value();
        ++i;
    }
    return parentNodes;
}

IdList PimItemRelationCache::getParents(Id id)
{
    return values(id, mParents);
}

IdList keys(Id key, const PimItemRelationCache::ParentMapping &map)
{
    IdList parentNodes;
    PimItemRelationCache::ParentMapping::right_const_iterator i = map.constFindRight(key);
    while (i != map.rightConstEnd() && i.key() == key) {
        parentNodes << i.value();
        ++i;
    }
    return parentNodes;
}

IdList PimItemRelationCache::getChildNodes(Id id) const
{
    IdList result;
    const IdList &list = keys(id, mParents);
    result.append(list);
    foreach (Id child, list) {
        result.append(getChildNodes(child));
    }
    return result;
}

IdList PimItemRelationCache::getAffectedChildItems(Id id) const
{
    IdList itemList;
    IdList itemsToUpdate = getChildNodes(id);
//     kDebug() << itemsToUpdate;
    foreach (Id update, itemsToUpdate) {
        if (isVirtual(update)) {
            continue;
        }
        itemList << update;
    }
    return itemList;
}

void PimItemRelationCache::moveNode(Id id, IdList parents)
{
    kDebug() << id << parents;
    IdList itemList = getAffectedChildItems(id);
    if (!isVirtual(id)) {
        itemList << id;
    }

    mParents.removeLeft(id);
    foreach(Id parent, parents) {
        mParents.insert(id, parent);
    }
    rebuildCache();
    emit parentsChanged(id, parents);
    emit updateItems(itemList);
}

void PimItemRelationCache::removeNodeRecursive(Id id)
{
//     kDebug() << id;
    mParents.removeLeft(id);
//     if (mItemIdCache.values().contains(id)) {
//         mItemIdCache.remove(mItemIdCache.key(id));
//     }
    Q_ASSERT(!mParents.leftContains(id));

    const IdList &children = getChildNodes(id);
    foreach (Id child, children) {
        removeNodeRecursive(child);
    }
}

void PimItemRelationCache::removeNode(Id id)
{
    if (!mParents.leftContains(id)) {
        return;
    }
    const IdList &itemList = getAffectedChildItems(id);
//     kDebug() << id;
    removeNodeRecursive(id);

    rebuildCache();

    emit nodeRemoved(id);
    emit updateItems(itemList);
}

Id PimItemRelationCache::getNextId()
{
    return mIdCounter++;
}

void PimItemRelationCache::addUidMapping(const QByteArray& uid, Id id)
{
    Q_ASSERT(!uid.isEmpty());
    Q_ASSERT(!mUidMapping.contains(uid) || (getId(uid) == id));
    if (!mUidMapping.contains(uid)) {
        mUidMapping.insert(uid, id);
    }
}

Id PimItemRelationCache::getId(const QByteArray& uid) const
{
    return mUidMapping.value(uid);
}

QByteArray PimItemRelationCache::getUid(Id id) const
{
    //No empty uids in map
    Q_ASSERT(!mUidMapping.key(id).isEmpty() || !mUidMapping.values().contains(id));
    return mUidMapping.key(id);
}

Id PimItemRelationCache::getUidMapping(const QByteArray& uid)
{
    if (!mUidMapping.contains(uid)) {
        mUidMapping.insert(uid, getNextId());
    }
    return getId(uid);
}

QHash< QByteArray, Id > PimItemRelationCache::uidMapping() const
{
    return mUidMapping;
}








VirtualRelationCache::VirtualRelationCache()
:   PimItemRelationCache()
{

}

void VirtualRelationCache::mergeNode(const TreeNode &node)
{
//     kDebug() << node.id << node.name;
    const bool created = !mNames.contains(node.id);
    if (mNames.value(node.id) != node.name || created) {
        mNames.insert(node.id, node.name);
        //TODO the names need some changing for projects as the name comes from the item itself and not one of its children
        if (!created && !node.name.isEmpty()) {
            emit virtualNodeRenamed(node.id, node.name);
        }
    }

    PimItemRelationCache::mergeNode(node);
    //TODO emit changes if changed
    if (node.knowsParents) {
        mParents.removeLeft(node.id);
        foreach (const TreeNode &parentNode, node.parentNodes) {
            mParents.insert(node.id, parentNode.id);
            mergeNode(parentNode);
        }
    }
    
    if (created) {
//         kDebug() << "created node " << node.id << mParents.values(node.id) << node.name;
        QString name = node.name;
        if (name.isEmpty()) {
            name = "noname";
        }
//         Q_ASSERT(!node.name.isEmpty());
        emit virtualNodeAdded(node.id, values(node.id, mParents), name);
    }
}

QString VirtualRelationCache::getName(Id id)
{
//     kDebug() << id << mNames.value(id);
//     Q_ASSERT(mNames.contains(id));
    return mNames.value(id);
}

void VirtualRelationCache::removeNodeRecursive(Id id)
{
    mNames.remove(id);
    Q_ASSERT(!mNames.contains(id));
    PimItemRelationCache::removeNodeRecursive(id);
}

void VirtualRelationCache::removeNode(Id id)
{
    if (!mParents.leftContains(id) && !mNames.contains(id)) {
        return;
    }
    PimItemRelationCache::removeNode(id);
}

void VirtualRelationCache::renameNode(Id id, const QString &name)
{
    if (name == mNames.value(id)) {
        return;
    }
    IdList itemList = getAffectedChildItems(id);
    if (!isVirtual(id)) {
        itemList << id;
    }
    mNames.insert(id, name);
    rebuildCache();
    emit virtualNodeRenamed(id, name);
    emit updateItems(itemList);
}

