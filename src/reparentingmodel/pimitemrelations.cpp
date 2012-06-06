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
    mIdCounter(0)
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
    foreach (const TreeNode &parentNode, node.parentNodes) {
        mParents.insert(node.id, parentNode.id);
        mergeNode(parentNode);
    }
    if (created) {
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

    return id;
}

QString PimItemRelations::getName(Id id)
{
    return mNames.value(id);
}

IdList PimItemRelations::getParents(Id id)
{
    return mParents.values(id);
}

IdList PimItemRelations::getChildNodes(Id id)
{
    IdList result;
    const IdList &list = mParents.keys(id);
    result.append(list);
    foreach (Id child, list) {
        result.append(getChildNodes(child));
    }
    return result;
}

void PimItemRelations::moveNode(Id id, IdList parents)
{
    IdList itemsToUpdate = getChildNodes(id);

    mParents.remove(id);
    foreach(Id parent, parents) {
        mParents.insert(id, parent);
    }
    //get affected child nodes if virtual node
    //update all nodes
    rebuildCache();
//     updateNodes(itemsToUpdate);
//     foreach(Id item, itemsToUpdate) {
//         setRelationTree(mItems.value(item));
//     }
}

void PimItemRelations::removeNode(Id id)
{
    if (!mParents.contains(id) && !mNames.contains(id)) {
        return;
    }
    kDebug() << id;
    mParents.remove(id);
    mNames.remove(id);
    //get affected child nodes if virtual node
    //update all nodes
    rebuildCache();
    emit nodeRemoved(id);
}

void PimItemRelations::renameNode(Id id, const QString &name)
{
    if (name == mNames.value(id)) {
        return;
    }
    mNames.insert(id, name);
    //get affected child nodes if virtual node
    //update all nodes
    rebuildCache();
    emit virtualNodeRenamed(id, name);
}

Id PimItemRelations::getItemId(const Akonadi::Item &item)
{
    if (mItemIdCache.contains(item.id())) {
        return mItemIdCache.value(item.id());
    }
    Id id = mIdCounter++;
    mItemIdCache[item.id()] = id;
    return id;
}




CategoriesStructure::CategoriesStructure()
:   PimItemRelations()
{

}

const QChar pathSeparator()
{
    return QChar(0x2044);
}

QString getCategoryName(const QString &categoryPath)
{
    QString categoryName = categoryPath;
    if (categoryPath.contains(pathSeparator())) {
        categoryName = categoryPath.split(pathSeparator()).first();
    }
    return categoryName;
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
    foreach(Id id, mParents.keys()) {
        if (mItemIdCache.contains(id)) {
            continue;
        }
        mCategoryMap.insert(getCategoryPath(id), id);
    }
}


TreeNode CategoriesStructure::createCategoryNode(const QString &categoryPath)
{
    //TODO cache
//     if (mCategoryMap.contains(categoryPath)) {
//         return TreeNode node(categoryName, mCategoryMap.value(categoryPath), parentNodes);
//     }
    const QString &categoryName = getCategoryName(categoryPath);
    QList<TreeNode> parentNodes;
    if (categoryPath.contains(pathSeparator())) {
        const QString &parentCategory = categoryPath.left(categoryPath.lastIndexOf(pathSeparator()));
        parentNodes << createCategoryNode(parentCategory);
    }
    if (!mCategoryMap.contains(categoryPath)) {
        mCategoryMap[categoryPath] = mIdCounter++;
        kDebug() << categoryPath << mIdCounter;
    }
    return TreeNode(categoryName, mCategoryMap.value(categoryPath), parentNodes);
}

void CategoriesStructure::createCategoryNode(const QString& categoryPath, const IdList& parents)
{
    const TreeNode &node = createCategoryNode(categoryPath);
    //TODO parents
    mergeNode(node);
}


Relation CategoriesStructure::getRelationTree(const Akonadi::Item& item)
{
    if (!item.isValid()) {
        return Relation();
    }
    KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();

    if (!todo) {
        return Relation();
    }
    
    QStringList categories = todo->categories();
    QList<TreeNode> parents;
    foreach (const QString &category, categories) {
        parents << createCategoryNode(category);
    }
    return Relation(getItemId(item), parents);
}

QStringList getCategories(const Relation &rel)
{
    return QStringList();
}

void CategoriesStructure::setRelationTree(Akonadi::Item &item, const Relation &rel)
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

