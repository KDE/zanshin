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


#include "pimitemrelations.h"
#include <pimitem.h>
#include <quuid.h>

TreeNode::TreeNode(const QString& n, const Id& i, const QList< TreeNode >& p)
:   name(n),
    id(i),
    parentNodes(p)
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





PimItemRelations::PimItemRelations()
:   QObject(),
    mIdCounter(1)
{

}

void PimItemRelations::mergeNode(const TreeNode &node)
{
    kDebug() << node.id << node.name;
    bool created = false;
    if (!mNames.contains(node.id)) {
        created = true;
    }
    if (mNames.value(node.id) != node.name) {
        mNames.insert(node.id, node.name);
        if (!created) {
            emit virtualNodeRenamed(node.id, node.name);
        }
    }
    
    //TODO emit changes if changed
    mParents.remove(node.id);
    Q_ASSERT(node.parentNodes.size() <= 1);
    foreach (const TreeNode &parentNode, node.parentNodes) {
        mParents.insert(node.id, parentNode.id);
        mergeNode(parentNode);
    }
    if (created) {
        kDebug() << "created node " << node.id << mParents.values(node.id) << node.name;
        Q_ASSERT(!node.name.isEmpty());
        emit virtualNodeAdded(node.id, mParents.values(node.id), node.name);
    }
}

Id PimItemRelations::addItem(const Akonadi::Item &item)
{
    //TODO cache
    const Relation &rel = getRelationTree(item);
    Id id = rel.id;
    mParents.remove(id);
    foreach (const TreeNode &node, rel.parentNodes) {
        mParents.insert(id, node.id);
        mergeNode(node);
    }
    kDebug() << item.id() << mParents.values(id);
    Q_ASSERT(mItemIdCache.contains(item.id()));
    return id;
}

Id PimItemRelations::getItemId(const Akonadi::Item &item) const
{
    kDebug() << item.id();
    Q_ASSERT(mItemIdCache.contains(item.id()));
    return mItemIdCache.value(item.id());
}


QString PimItemRelations::getName(Id id)
{
    kDebug() << id << mNames.value(id);
    Q_ASSERT(mNames.contains(id));
    return mNames.value(id);
}

IdList PimItemRelations::getParents(Id id)
{
    return mParents.values(id);
}

IdList PimItemRelations::getChildNodes(Id id) const
{
    IdList result;
    const IdList &list = mParents.keys(id);
    result.append(list);
    foreach (Id child, list) {
        result.append(getChildNodes(child));
    }
    return result;
}

IdList PimItemRelations::getAffectedChildItems(Id id) const
{
    QList<Akonadi::Item::Id> itemList;
    IdList itemsToUpdate = getChildNodes(id);
    kDebug() << itemsToUpdate;
    foreach (Id update, itemsToUpdate) {
        if (isVirtual(update)) {
            continue;
        }
        itemList << update;
    }
    return itemList;
}

void PimItemRelations::moveNode(Id id, IdList parents)
{
    kDebug() << id << parents;
    IdList itemList = getAffectedChildItems(id);
    if (!isVirtual(id)) {
        itemList << id;
    }

    mParents.remove(id);
    foreach(Id parent, parents) {
        mParents.insert(id, parent);
    }
    rebuildCache();
    emit parentsChanged(id, parents);
    emit updateItems(itemList);
}

void PimItemRelations::removeNodeRecursive(Id id)
{
    kDebug() << id;
    mParents.remove(id);
    mNames.remove(id);
    Q_ASSERT(!mParents.contains(id));
    Q_ASSERT(!mNames.contains(id));

    const IdList &children = getChildNodes(id);
    foreach (Id child, children) {
        removeNodeRecursive(child);
    }
}

void PimItemRelations::removeNode(Id id)
{
    if (!mParents.contains(id) && !mNames.contains(id)) {
        return;
    }
    const IdList &itemList = getAffectedChildItems(id);
    kDebug() << id;
    removeNodeRecursive(id);

    rebuildCache();

    emit nodeRemoved(id);
    emit updateItems(itemList);
}

void PimItemRelations::renameNode(Id id, const QString &name)
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

Id PimItemRelations::getOrCreateItemId(const Akonadi::Item &item)
{
    if (mItemIdCache.contains(item.id())) {
        return mItemIdCache.value(item.id());
    }
    Id id = mIdCounter++;
    mItemIdCache[item.id()] = id;
    return id;
}

bool PimItemRelations::isVirtual(Id id) const
{
    return !mItemIdCache.values().contains(id);
}


PimItemRelationsStructure::PimItemRelationsStructure(PimItemRelation::Type type)
:   PimItemRelations(),
    mType(type)
{

}

TreeNode PimItemRelationsStructure::createNode(const PimItemTreeNode &node)
{
    if (!mUidMapping.contains(node.uid)) {
        mUidMapping.insert(node.uid, mIdCounter++);
    }
    Id id = mUidMapping.value(node.uid);
    QList<TreeNode> parents;
    foreach(const PimItemTreeNode &parentNode, node.parentNodes) {
        parents << createNode(parentNode);
    }
    return TreeNode(node.name, id, parents);
}


Relation PimItemRelationsStructure::createRelation(const PimItemRelation &relation, const Id itemId)
{
    QList<TreeNode> parents;
    foreach(const PimItemTreeNode &n, relation.parentNodes) {
        parents << createNode(n);
    }
    return Relation(itemId, parents);
}


Relation PimItemRelationsStructure::getRelationTree(const Akonadi::Item& item)
{
    QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
    Q_ASSERT (!pimitem.isNull());
    kDebug() << pimitem->itemType();

    foreach(const PimItemRelation &rel, pimitem->getRelations()) {
        kDebug() << rel.type;
        if (rel.type == mType) {
            return createRelation(rel, getOrCreateItemId(item)); //TODO merge multiple relations
        }
    }
    return Relation(getOrCreateItemId(item), QList<TreeNode>());
}

QList<PimItemTreeNode> PimItemRelationsStructure::getParentTreeNodes(Id id)
{
    QList<PimItemTreeNode> list;
    IdList parents = mParents.values(id);
    foreach (Id parent, parents) {
        list << PimItemTreeNode(mUidMapping.key(parent), mNames.value(parent), getParentTreeNodes(parent));
    }
    return list;
}

void PimItemRelationsStructure::updateRelationTree(Akonadi::Item &item)
{
    kDebug() << item.id();
    QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
    Q_ASSERT(!pimitem.isNull());
    const Id id = mItemIdCache.value(item.id());
    kDebug() << id;
    QList<PimItemRelation> relations = pimitem->getRelations();
    int i = 0;
    foreach(const PimItemRelation &rel, pimitem->getRelations()) {
        if (rel.type == mType) {
            relations.removeAt(i);
        }
        i++;
    }
    relations << PimItemRelation(mType, getParentTreeNodes(id));
    pimitem->setRelations(relations);
    item = pimitem->getItem();
}

QList<TreeNode> PimItemRelationsStructure::getParentList(Id id)
{
    QList<TreeNode> list;
    IdList parents = mParents.values(id);
    foreach (Id parent, parents) {
        list << TreeNode(mNames.value(parent), parent, getParentList(parent));
    }
    return list;
}

void PimItemRelationsStructure::addNode(const QString& name, const IdList& parents)
{
    foreach (Id id, mNames.keys()) {
        kDebug() << id << mNames.value(id);
    }
    
    Q_ASSERT(!name.isEmpty());
    QList<TreeNode> parentNodes;
    foreach (Id parent, parents) {
        kDebug() << parent;
        parentNodes << TreeNode(getName(parent), parent, getParentList(parent)); //TODO parents
        Q_ASSERT(!getName(parent).isEmpty());
    }
    mIdCounter++;
    Id id = mIdCounter;
    mUidMapping.insert(QUuid::createUuid().toByteArray(), id);
    mergeNode(TreeNode(name, id, parentNodes));
}

void PimItemRelationsStructure::rebuildCache()
{

}

QString PimItemRelationsStructure::getPath(Id id) const
{
    return QString("/this is a path");
}

