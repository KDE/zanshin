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

#include "pimitemstructurecache.h"
#include "pimitem.h"
#include "pimitemfactory.h"
#include <quuid.h>


PimItemStructureCache::PimItemStructureCache(PimItemRelation::Type type)
:   VirtualRelationCache(),
    mType(type)
{
}

TreeNode PimItemStructureCache::createNode(const PimItemTreeNode &node)
{
    Id id = getUidMapping(node.uid);
    if (!node.knowsParents) {
        return TreeNode(node.name, id);
    }
    QList<TreeNode> parents;
    foreach(const PimItemTreeNode &parentNode, node.parentNodes) {
        parents << createNode(parentNode);
    }
    return TreeNode(node.name, id, parents);
}

Relation PimItemStructureCache::createRelation(const PimItemRelation &relation, const Id itemId)
{
    QList<TreeNode> parents;
    foreach(const PimItemTreeNode &n, relation.parentNodes) {
        parents << createNode(n);
    }
    return Relation(itemId, parents);
}


Relation PimItemStructureCache::getRelationTree(Id id, const Akonadi::Item &item)
{
    PimItem::Ptr pimitem(PimItemFactory::getItem(item));
    Q_ASSERT (!pimitem.isNull());
    foreach(const PimItemRelation &rel, pimitem->getRelations()) {
//         kDebug() << rel.type;
        if (rel.type == mType) {
            return createRelation(rel, id); //TODO merge multiple relations
        }
    }
    return Relation(id, QList<TreeNode>());
}

QList<PimItemTreeNode> PimItemStructureCache::getParentTreeNodes(Id id)
{
    QList<PimItemTreeNode> list;
    IdList parents = values(id, mParents);
    foreach (Id parent, parents) {
        list << PimItemTreeNode(getUid(parent), mNames.value(parent), getParentTreeNodes(parent));
        kDebug() << mNames.value(parent);
    }
    return list;
}

void PimItemStructureCache::updateRelationTree(Akonadi::Item &item)
{
//     kDebug() << item.id();
    PimItem::Ptr pimitem(PimItemFactory::getItem(item));
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

QList<TreeNode> PimItemStructureCache::getParentList(Id id)
{
    QList<TreeNode> list;
    IdList parents = values(id, mParents);
    foreach (Id parent, parents) {
        list << TreeNode(mNames.value(parent), parent, getParentList(parent));
    }
    return list;
}

void PimItemStructureCache::addNode(const QString& name, const IdList& parents)
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
    Id id = getNextId();
    addUidMapping(QUuid::createUuid().toByteArray(), id);
    mergeNode(TreeNode(name, id, parentNodes));
}



ProjectStructureCache::ProjectStructureCache()
{

}

Relation ProjectStructureCache::getRelationTree(Id id, const Akonadi::Item& item)
{
    PimItem::Ptr pimitem(PimItemFactory::getItem(item));
    Q_ASSERT (!pimitem.isNull());
    const QByteArray uid = pimitem->getUid().toLatin1();
//     qDebug() << "######### " << item.url().url() << id << uid << pimitem->getRelations().size();
    addUidMapping(uid, id);
    QList<TreeNode> parents;
    foreach(const PimItemRelation &rel, pimitem->getRelations()) {
//         qDebug() << "relation " << rel.type << rel.parentNodes.size();
        if (rel.type == PimItemRelation::Project) {
            foreach (const PimItemTreeNode &p, rel.parentNodes) {
                if (p.uid.isEmpty()) {
                    kWarning() << "empty parent on item: " << item.id();
                    continue;
                }
                Id projectId = getUidMapping(p.uid);
//                 qDebug() << p.uid << projectId;
                parents << TreeNode(p.name, projectId);
            }
        }
    }
    return Relation(id, parents);
}

Id ProjectStructureCache::addCollection(const Akonadi::Collection &col)
{
    if (!mCollectionMapping.contains(col.id())) {
        mCollectionMapping.insert(col.id(), getNextId());
    }
    return mCollectionMapping.value(col.id());
}

bool ProjectStructureCache::hasChildren(Id id) const
{
    //FIXME hotspot
    return mParents.rightContains(id);
}

Akonadi::Entity::Id ProjectStructureCache::itemId(Id id) const
{
    if (!mItemIdCache.values().contains(id)) {
        return -1;
    }
    return mItemIdCache.key(id);
}

IdList ProjectStructureCache::getChildren(Id id) const
{
    return getAffectedChildItems(id);
}

void ProjectStructureCache::printCache()
{
//     qDebug() << "itemids: " << mItemIdCache;
//     qDebug() << "collections: " << mCollectionMapping;
//     qDebug() << "parents " << mParents;
//     qDebug() << "uids " << mUidMapping;
}
