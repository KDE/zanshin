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
#include <KCalCore/Todo>

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
//     kDebug() << item.id() << mParents.values(id);

    return id;
}

Id PimItemRelations::getItemId(const Akonadi::Item &item) const
{
    Q_ASSERT(mItemIdCache.contains(item.id()));
    return mItemIdCache.value(item.id());
}


QString PimItemRelations::getName(Id id)
{
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
//         Q_ASSERT(mItemIdCache.values().contains(update));
//         itemList << mItemIdCache.key(update);
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


CategoriesStructure::CategoriesStructure()
:   PimItemRelations()
{

}

const QChar pathSeparator()
{
    return QChar(0x2044);
}

Id CategoriesStructure::getCategoryId(const QString& categoryPath) const
{
    return mCategoryMap.value(categoryPath);
}


QString CategoriesStructure::getCategoryPath(Id id) const
{
    Q_ASSERT(mNames.contains(id));
    QString path(pathSeparator()+mNames.value(id));
    if (mParents.contains(id)) {
        Q_ASSERT(mParents.values(id).size() == 1);
        path.prepend(getCategoryPath(mParents.values(id).first()));
    }
    return path;
}

void CategoriesStructure::rebuildCache()
{
    kWarning();
    mCategoryMap.clear();
    foreach(Id id, mNames.keys()) {
        if (!isVirtual(id)) {
            continue;
        }
        kDebug() << id;
        mCategoryMap.insert(getCategoryPath(id), id);
    }
}


TreeNode CategoriesStructure::createCategoryNode(const QString &categoryPath)
{
    //TODO cache
//     if (mCategoryMap.contains(categoryPath)) {
//         return TreeNode node(categoryName, mCategoryMap.value(categoryPath), parentNodes);
//     }
    QString categoryName = categoryPath;
    Q_ASSERT(!categoryName.isEmpty());
    kDebug() << categoryPath << categoryName;
    QList<TreeNode> parentNodes;
    if (categoryPath.contains(pathSeparator())) {
        const QStringList &categories = categoryPath.split(pathSeparator());
        Q_ASSERT(!categories.isEmpty());
        categoryName = categories.last();
        const QString &parentCategory = categoryPath.left(categoryPath.lastIndexOf(pathSeparator()));
        if (!parentCategory.isEmpty()) {
            parentNodes << createCategoryNode(parentCategory);
        }
    }
    if (!mCategoryMap.contains(categoryPath)) {
        mCategoryMap[categoryPath] = mIdCounter++;
        kDebug() << "new Id " << categoryName << mIdCounter;
    }
//     kDebug() << categoryName << mCategoryMap.value(categoryPath);
    return TreeNode(categoryName, mCategoryMap.value(categoryPath), parentNodes);
}


void CategoriesStructure::addCategoryNode(const QString& categoryPath, const IdList &parents)
{
    kDebug() << categoryPath << parents;
    Q_ASSERT(!categoryPath.isEmpty());
    if (parents.isEmpty()) {
        mergeNode(createCategoryNode(categoryPath));
        return;
    }
    foreach (Id parent, parents) {
        Q_ASSERT(mCategoryMap.values().contains(parent));
        mergeNode(createCategoryNode(mCategoryMap.key(parent)+pathSeparator()+categoryPath));
    }
}


Relation CategoriesStructure::getRelationTree(const Akonadi::Item& item)
{
    if (!item.isValid()) {
        return Relation();
    }
    if (!item.hasPayload<KCalCore::Todo::Ptr>()) {
        qWarning() << "not a todo";
        return Relation();
    }
    KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();
    if (!todo) {
        qWarning() << "not a todo";
        return Relation();
    }
    
    QStringList categories = todo->categories();
//     kDebug() << categories;
    QList<TreeNode> parents;
    foreach (const QString &category, categories) {
        parents << createCategoryNode(category);
    }
    return Relation(getOrCreateItemId(item), parents);
}

QStringList getCategories(const Relation &rel)
{
    return QStringList();
}

void CategoriesStructure::updateRelationTree(Akonadi::Item &item)
{
//     QStringList categories = getCategories(rel);
//     KCalCore::Todo::Ptr todo = item.getItem().payload<KCalCore::Todo::Ptr>();
//     todo->setCategories(categories);
    //TODO save


//     if (!item.isValid()) {
//         return false;
//     }
// 
//     KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();
// 
//     if (!todo) {
//         return false;
//     }
// 
//     QStringList categories = todo->categories();
//     if (categories.contains(categoryPath)) {
//         categories.removeAll(categoryPath);
//         todo->setCategories(categories);
//         new Akonadi::ItemModifyJob(item);
//         return true;
//     }
//     return false;
}

