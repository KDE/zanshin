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
#include <KUrl>

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




PimItemRelationCache::PimItemRelationCache()
:   QObject(),
    mIdCounter(1)
{

}


void PimItemRelationCache::mergeNode(const TreeNode &node)
{
}

Id PimItemRelationCache::addItem(const Akonadi::Item &item)
{
    Q_ASSERT(item.isValid());
    //TODO cache
    
//     kDebug() << pimitem->itemType();
    Id id = getOrCreateItemId(item);
    const Relation &rel = getRelationTree(id, item);
    qDebug() << " <<<<<<<<<<<<<<<<< " << item.url().url() << id << rel.id << rel.parentNodes.size();
    mParents.remove(id);
    foreach (const TreeNode &node, rel.parentNodes) {
        mParents.insert(id, node.id);
        mergeNode(node);
    }
//     kDebug() << item.id() << mParents.values(id);
    Q_ASSERT(mItemIdCache.contains(item.id()));
    return id;
}

Id PimItemRelationCache::getItemId(const Akonadi::Item &item) const
{
    kDebug() << item.id();
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
    QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
    Q_ASSERT (!pimitem.isNull());
    QByteArray uid = pimitem->getUid().toLatin1();
    if (mUidMapping.contains(uid)) {
        id = mUidMapping.value(uid);
    } else {
        id = mIdCounter++;
    }
    mItemIdCache[item.id()] = id;
//    kDebug() << item.id() << id;
    return id;
}

bool PimItemRelationCache::isVirtual(Id id) const
{
    return !mItemIdCache.values().contains(id);
}

IdList PimItemRelationCache::getParents(Id id)
{
    return mParents.values(id);
}

IdList PimItemRelationCache::getChildNodes(Id id) const
{
    IdList result;
    const IdList &list = mParents.keys(id);
    result.append(list);
    foreach (Id child, list) {
        result.append(getChildNodes(child));
    }
    return result;
}

IdList PimItemRelationCache::getAffectedChildItems(Id id) const
{
    QList<Akonadi::Item::Id> itemList;
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
//     kDebug() << id << parents;
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

void PimItemRelationCache::removeNodeRecursive(Id id)
{
//     kDebug() << id;
    mParents.remove(id);
//     if (mItemIdCache.values().contains(id)) {
//         mItemIdCache.remove(mItemIdCache.key(id));
//     }
    Q_ASSERT(!mParents.contains(id));

    const IdList &children = getChildNodes(id);
    foreach (Id child, children) {
        removeNodeRecursive(child);
    }
}

void PimItemRelationCache::removeNode(Id id)
{
    if (!mParents.contains(id)) { 
        return;
    }
    const IdList &itemList = getAffectedChildItems(id);
//     kDebug() << id;
    removeNodeRecursive(id);

    rebuildCache();

    emit nodeRemoved(id);
    emit updateItems(itemList);
}






PimItemRelations::PimItemRelations()
:   PimItemRelationCache()
{

}

void PimItemRelations::mergeNode(const TreeNode &node)
{
//     kDebug() << node.id << node.name;
    bool created = false;
    if (!mNames.contains(node.id)) {
        created = true;
    }
    if (mNames.value(node.id) != node.name || created) {
        mNames.insert(node.id, node.name);
        //TODO the names need some changing for projects as the name comes from the item itself and not one of its children
        if (!created && !node.name.isEmpty()) {
            emit virtualNodeRenamed(node.id, node.name);
        }
    }

    PimItemRelationCache::mergeNode(node);
    //TODO emit changes if changed
    mParents.remove(node.id);
    foreach (const TreeNode &parentNode, node.parentNodes) {
        mParents.insert(node.id, parentNode.id);
        mergeNode(parentNode);
    }
    
    if (created) {
//         kDebug() << "created node " << node.id << mParents.values(node.id) << node.name;
        QString name = node.name;
        if (name.isEmpty()) {
            name = "noname";
        }
//         Q_ASSERT(!node.name.isEmpty());
        emit virtualNodeAdded(node.id, mParents.values(node.id), name);
    }
}

QString PimItemRelations::getName(Id id)
{
//     kDebug() << id << mNames.value(id);
//     Q_ASSERT(mNames.contains(id));
    return mNames.value(id);
}

void PimItemRelations::removeNodeRecursive(Id id)
{
    mNames.remove(id);
    Q_ASSERT(!mNames.contains(id));
    PimItemRelationCache::removeNodeRecursive(id);
}

void PimItemRelations::removeNode(Id id)
{
    if (!mParents.contains(id) && !mNames.contains(id)) {
        return;
    }
    PimItemRelationCache::removeNode(id);
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


Relation PimItemRelationsStructure::getRelationTree(Id id, const Akonadi::Item &item)
{
    QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
    Q_ASSERT (!pimitem.isNull());
    foreach(const PimItemRelation &rel, pimitem->getRelations()) {
//         kDebug() << rel.type;
        if (rel.type == mType) {
            return createRelation(rel, id); //TODO merge multiple relations
        }
    }
    return Relation(id, QList<TreeNode>());
}

QList<PimItemTreeNode> PimItemRelationsStructure::getParentTreeNodes(Id id)
{
    QList<PimItemTreeNode> list;
    IdList parents = mParents.values(id);
    foreach (Id parent, parents) {
        list << PimItemTreeNode(mUidMapping.key(parent), mNames.value(parent), getParentTreeNodes(parent));
        kDebug() << mNames.value(parent);
    }
    return list;
}

void PimItemRelationsStructure::updateRelationTree(Akonadi::Item &item)
{
//     kDebug() << item.id();
    QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
    Q_ASSERT(!pimitem.isNull());
    Q_ASSERT(mItemIdCache.contains(item.id()));
    const Id id = mItemIdCache.value(item.id());
//     kDebug() << id;
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
//     foreach (Id id, mNames.keys()) {
//         kDebug() << id << mNames.value(id);
//     }
    
    Q_ASSERT(!name.isEmpty());
    QList<TreeNode> parentNodes;
    foreach (Id parent, parents) {
//         kDebug() << parent;
        parentNodes << TreeNode(getName(parent), parent, getParentList(parent));
        Q_ASSERT(!getName(parent).isEmpty());
    }
    mIdCounter++;
    Id id = mIdCounter;
    mUidMapping.insert(QUuid::createUuid().toByteArray(), id);
    mergeNode(TreeNode(name, id, parentNodes));
}







ProjectStructure::ProjectStructure()
{

}

Relation ProjectStructure::getRelationTree(Id id, const Akonadi::Item& item)
{
    QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
    Q_ASSERT (!pimitem.isNull());
    QByteArray uid = pimitem->getUid().toLatin1();
    qDebug() << "######### " << item.url().url() << id << uid << pimitem->getRelations().size();
    if (!mUidMapping.contains(uid)) {
        mUidMapping.insert(uid, id);
    }
    Q_ASSERT(mUidMapping.value(uid) == id);
    QList<TreeNode> parents;
    foreach(const PimItemRelation &rel, pimitem->getRelations()) {
        qDebug() << "relation " << rel.type << rel.parentNodes.size();
        if (rel.type == PimItemRelation::Project) {
            foreach (const PimItemTreeNode &p, rel.parentNodes) {
                qDebug() << p.uid;
                if (!mUidMapping.contains(p.uid)) {
                    mUidMapping.insert(p.uid, mIdCounter++);
                }
                Id projectId = mUidMapping.value(p.uid);
                parents << TreeNode(p.name, projectId);
            }
        }
    }
    return Relation(id, parents);
}

void ProjectStructure::updateRelationTree(Akonadi::Item& item)
{

}

Id ProjectStructure::addCollection(const Akonadi::Collection &col)
{
    if (!mCollectionMapping.contains(col.id())) {
        mCollectionMapping.insert(col.id(), mIdCounter++);
    }
    return mCollectionMapping.value(col.id());
}

bool ProjectStructure::hasChildren(Id id)
{
    return mParents.values().contains(id);
}

Id ProjectStructure::addItem(const Akonadi::Item &item)
{
    return PimItemRelationCache::addItem(item);
}


void ProjectStructure::printCache()
{
    qDebug() << mItemIdCache;
    qDebug() << mCollectionMapping;
    qDebug() << mParents;
    qDebug() << mUidMapping;

}


