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


#include "categoriesstrategy.h"
#include <categorymanager.h>
#include <todohelpers.h>
#include <todonode.h>
#include <KIcon>
#include <KUrl>
#include <KLocalizedString>
#include <QMimeData>
#include <KCalCore/Todo>

CategoriesStrategy::CategoriesStrategy()
:   ReparentingStrategy(),
    mInbox(1),
    mRoot(2),
    mRelations(new CategoriesStructure())
{
    mReparentOnRemoval = true;
    connect(mRelations.data(), SIGNAL(virtualNodeAdded(Id, IdList, QString)), this, SLOT(createVirtualNode(Id, IdList, QString)));
//     connect(&CategoryManager::instance(), SIGNAL(categoryAdded(QString)),
//             this, SLOT(createCategoryNode(QString)));
//     connect(&CategoryManager::instance(), SIGNAL(categoryRemoved(QString)),
//             this, SLOT(removeCategoryNode(QString)));
//     connect(&CategoryManager::instance(), SIGNAL(categoryRenamed(QString,QString)),
//             this, SLOT(renameCategoryNode(QString,QString)));
//     connect(&CategoryManager::instance(), SIGNAL(categoryMoved(QString,QString)),
//             this, SLOT(moveCategoryNode(QString,QString)));
}

void CategoriesStrategy::init()
{
    ReparentingStrategy::init();
    TodoNode *node = createNode(mInbox, IdList(), "No Context");
    node->setData(i18n("No Context"), 0, Qt::DisplayRole);
    node->setData(KIcon("mail-folder-inbox"), 0, Qt::DecorationRole);
    node->setRowData(Zanshin::Inbox, Zanshin::ItemTypeRole);

    TodoNode *node2 = createNode(mRoot, IdList(), "Contexts");
    node2->setData(i18n("Contexts"), 0, Qt::DisplayRole);
    node2->setData(KIcon("document-multiple"), 0, Qt::DecorationRole);
    node2->setRowData(Zanshin::CategoryRoot, Zanshin::ItemTypeRole);
}

Id translateFrom(Id id)
{
    //TODO proper id mapping
    return id+10;
}

Id translateTo(Id id)
{
    //TODO proper id mapping
    return id-10;
}

IdList translateFrom(IdList l)
{
    IdList list;
    foreach (Id id, l) {
        list << translateFrom(id);
    }
    return list;
}

Id CategoriesStrategy::getId(const QModelIndex &sourceChildIndex)
{
    Zanshin::ItemType type = (Zanshin::ItemType) sourceChildIndex.data(Zanshin::ItemTypeRole).toInt();
    if (type!=Zanshin::StandardTodo) { //Filter all other items
        return -1;
    }
    return translateFrom(mRelations->addItem(sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>()));
//     const QString &uid = sourceChildIndex.data(Zanshin::UidRole).toString();
//     Q_ASSERT(!uid.isEmpty());
//     if (!mUidMapping.contains(uid)) {
//         mUidMapping.insert(uid, getNextId());
//     }
//     return mUidMapping.value(uid);
}

IdList CategoriesStrategy::getParents(const QModelIndex &sourceChildIndex, const IdList& ignore)
{
    Id id = getId(sourceChildIndex);
    Q_ASSERT(sourceChildIndex.data(Zanshin::ItemTypeRole).toInt()==Zanshin::StandardTodo);

    IdList parents = translateFrom(mRelations->getParents(mRelations->addItem(sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>())));
/*    
    QStringList categories = sourceChildIndex.data(Zanshin::CategoriesRole).toStringList();
    IdList parents;
    foreach (const QString &category, categories) {
        parents << createCategoryNode(category);
    }
    mNodeCategories.insert(id, parents);*/
    foreach(Id i, ignore) {
        parents.removeAll(i);
    }
    if (parents.isEmpty()) {
        return IdList() << mInbox;
    }
    return parents;
}

// QString getCategoryName(const QString &categoryPath)
// {
//     QString categoryName = categoryPath;
//     if (categoryPath.contains(CategoryManager::pathSeparator())) {
//         categoryName = categoryPath.split(CategoryManager::pathSeparator()).last();
//     }
//     return categoryName;
// }

// void CategoriesStrategy::createCategoryNode(const QString &categoryPath)
// {
//     if (mCategoryMap.contains(categoryPath)) {
//         return mCategoryMap.value(categoryPath);
//     }
//     Id parent = mRoot;
//     const QString &categoryName = getCategoryName(categoryPath);
//     if (categoryPath.contains(CategoryManager::pathSeparator())) {
//         const QString &parentCategory = categoryPath.left(categoryPath.lastIndexOf(CategoryManager::pathSeparator()));
//         parent = createCategoryNode(parentCategory);
//     }
// 
//     Id id = getNextId();
//     mCategoryMap[categoryPath] = id;
//     mNodeParentMap.insertMulti(id, parent);
//     kDebug() << id << categoryPath;
//     createNode(id, IdList() << mNodeParentMap.values(id), categoryName);
// }

void CategoriesStrategy::createVirtualNode(Id id, IdList parents, const QString& name)
{
    IdList p = translateFrom(parents);
    if (p.isEmpty()) {
        p << mRoot;
    }
    createNode(translateFrom(id), p, name);
}


void CategoriesStrategy::setData(TodoNode* node, Id id)
{
    kDebug() << id;
    if (id == mInbox || id == mRoot) {
        return;
    }
//     Q_ASSERT(mCategoryMap.values().contains(id));
//     const QString &categoryPath = mCategoryMap.key(id);
//     const QString &categoryName = getCategoryName(categoryPath);
    const QString &categoryName = mRelations->getName(translateTo(id));
    node->setData(categoryName, 0, Qt::DisplayRole);
    node->setData(categoryName, 0, Qt::EditRole);
//     node->setData(categoryPath, 0, Zanshin::CategoryPathRole);
    node->setData(KIcon("view-pim-notes"), 0, Qt::DecorationRole);
    node->setRowData(Zanshin::Category, Zanshin::ItemTypeRole);
}

bool CategoriesStrategy::reparentOnParentRemoval(Id child) const
{
//     kDebug() << child;
//     if (mCategoryMap.values().contains(child)) {
//         return false;
//     }
    return true;
}


// void CategoriesStrategy::removeCategoryNode(const QString &category)
// {
//     if (!mCategoryMap.contains(category)) {
//         return;
//     }
//     removeNode(mCategoryMap.value(category));
//     //TODO check that child categories are removed and child todos updated correctly
// }
// 
// void CategoriesStrategy::renameCategoryNode(const QString &oldCategoryPath, const QString &newCategoryPath)
// {
//     if (!mCategoryMap.contains(oldCategoryPath)) {
//         return;
//     }
//     Id id = mCategoryMap.value(oldCategoryPath);
//     renameNode(id, newCategoryPath);
//     mCategoryMap[newCategoryPath] = id;
//     mCategoryMap.remove(oldCategoryPath);
//     //TODO probably change renameNode into setData(id, QMap);
//     //TODO node->setData(newCategoryPath, 0, Zanshin::CategoryPathRole);
//     //TODO rename child nodes
//     
// //     TodoNode *node = m_categoryMap[oldCategoryPath];
// //     m_categoryMap[newCategoryPath] = node;
// //     m_categoryMap.remove(oldCategoryPath);
// // 
// //     QList<TodoNode*> children = node->children();
// //     foreach (TodoNode* child, children) {
// //         QModelIndex childIndex = m_manager->indexForNode(child, 0);
// //         if (childIndex.data(Zanshin::ItemTypeRole).toInt() == Zanshin::Category) {
// //             QString childPath = childIndex.data(Zanshin::CategoryPathRole).toString();
// //             QString newChildPath = childPath;
// //             newChildPath = newChildPath.replace(oldCategoryPath, newCategoryPath);
// //             CategoryManager::instance().renameCategory(childPath, newChildPath);
// //         }
// //     }
// // 
// //     QString newCategory = newCategoryPath.split(CategoryManager::pathSeparator()).last();
// //     node->setData(newCategory, 0, Qt::DisplayRole);
// //     node->setData(newCategory, 0, Qt::EditRole);
// //     node->setData(newCategoryPath, 0, Zanshin::CategoryPathRole);
// // 
// //     QModelIndex index = m_manager->indexForNode(node, 0);
// //     emit dataChanged(index, index);
// }
// 
// void CategoriesStrategy::moveCategoryNode(const QString &oldCategoryPath, const QString &newCategoryPath)
// {
// //     updateParents();
// //     TodoNode *node = m_categoryMap[oldCategoryPath];
// // 
// //     QList<TodoNode*> children = node->children();
// //     foreach (TodoNode* child, children) {
// //         QModelIndex childIndex = m_manager->indexForNode(child, 0);
// //         if (childIndex.data(Zanshin::ItemTypeRole).toInt() == Zanshin::Category) {
// //             QString childPath = childIndex.data(Zanshin::CategoryPathRole).toString();
// //             CategoryManager::instance().moveCategory(childPath, newCategoryPath, Zanshin::Category);
// //         } else {
// //             CategoryManager::instance().moveTodoToCategory(childIndex, newCategoryPath, Zanshin::Category);
// //         }
// //     }
// //TODO handle
// 
// }

// IdList getAffectedItems(Id)
// {
//     //Build a list of all children (the ones we need to update with the updated category information)
// }
// 
// void CategoriesStrategy::removeCategory(const Id& toRemove)
// {
//     //It's a category remove from all children
//     //Remove subcategories
//     
//     
//     //remove cateogry from every todo
// }

void CategoriesStrategy::onNodeRemoval(const Id& changed)
{
    kDebug() << changed;
//     if (mCategoryMap.values().contains(changed)) {
// 
//         
//     }
    mRelations->removeNode(translateTo(changed));
}


QMimeData* CategoriesStrategy::mimeData(const QModelIndexList& indexes) const
{
//     QStringList categoriesList;
//     foreach (const QModelIndex &proxyIndex, indexes) {
//         categoriesList << proxyIndex.data(Zanshin::CategoryPathRole).toString();
//     }
// 
//     if (!categoriesList.isEmpty()) {
//         QMimeData *mimeData = new QMimeData();
//         QString sep = CategoryManager::pathSeparator();
//         sep += CategoryManager::pathSeparator();
//         QByteArray categories = categoriesList.join(sep).toUtf8();
//         mimeData->setData("application/x-vnd.zanshin.category", categories);
//         return mimeData;
//     }
    return 0;
}


QStringList CategoriesStrategy::mimeTypes()
{
//     return QStringList() << "application/x-vnd.zanshin.category";
return QStringList();
}

Qt::DropActions CategoriesStrategy::supportedDropActions() const
{
    return Qt::MoveAction;
}


Qt::ItemFlags CategoriesStrategy::flags(const QModelIndex& index, Qt::ItemFlags flags)
{
    Zanshin::ItemType type = (Zanshin::ItemType) index.data(Zanshin::ItemTypeRole).toInt();
    if (type == Zanshin::Inbox || type == Zanshin::CategoryRoot) {
        return Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    }
    return flags | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
}


bool CategoriesStrategy::onDropMimeData(Id id, const QMimeData *mimeData, Qt::DropAction action)
{
    if (action != Qt::MoveAction || (!KUrl::List::canDecode(mimeData) && !mimeData->hasFormat("application/x-vnd.zanshin.category"))) {
        return false;
    }
    return false;
    
//     QString parentCategory = getData(id, Zanshin::CategoryPathRole).toString();
//     Zanshin::ItemType parentType = (Zanshin::ItemType)getData(id, Zanshin::ItemTypeRole).toInt();
// 
//     if (KUrl::List::canDecode(mimeData)) {
//         KUrl::List urls = KUrl::List::fromMimeData(mimeData);
//         foreach (const KUrl &url, urls) {
//             const Akonadi::Item urlItem = Akonadi::Item::fromUrl(url);
//             if (urlItem.isValid()) {
//                 Akonadi::Item item = TodoHelpers::fetchFullItem(urlItem);
// 
//                 if (!item.isValid()) {
//                     return false;
//                 }
// 
//                 if (item.hasPayload<KCalCore::Todo::Ptr>()) {
//                     CategoryManager::instance().moveTodoToCategory(item, parentCategory, parentType);
//                 }
//             }
//         }
//     } else {
//         if (parentType!=Zanshin::Category && parentType!=Zanshin::CategoryRoot) {
//             return false;
//         }
//         QByteArray categories = mimeData->data("application/x-vnd.zanshin.category");
//         QString sep = CategoryManager::pathSeparator();
//         sep += CategoryManager::pathSeparator();
//         QStringList categoriesPath = QString::fromUtf8(categories.data()).split(sep);
//         foreach (QString categoryPath, categoriesPath) {
//             CategoryManager::instance().moveCategory(categoryPath, parentCategory, parentType);
//         }
//     }
//     return true;
}

bool CategoriesStrategy::onSetData(Id id, const QVariant& value, int role)
{
    Zanshin::ItemType type = (Zanshin::ItemType) getData(id, Zanshin::ItemTypeRole).toInt();
//     if (type==Zanshin::Category) {
//         QString oldCategoryPath = getData(id, Zanshin::CategoryPathRole).toString();
//         QString newCategoryName = value.toString();
//         QString newCategoryPath = oldCategoryPath.left(oldCategoryPath.lastIndexOf(CategoryManager::pathSeparator())+1) + newCategoryName;
//         CategoryManager::instance().renameCategory(oldCategoryPath, newCategoryPath);
//         return true;
//     }
//     return false;
    return false;
}


void CategoriesStrategy::reset()
{
    ReparentingStrategy::reset();
//     mCategoryMap.clear();
//     mUidMapping.clear();
//     foreach(QString category, CategoryManager::instance().categories()) {
//         CategoryManager::instance().removeCategory(category);
//     }
}


