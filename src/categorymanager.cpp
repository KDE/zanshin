/* This file is part of Zanshin Todo.

   Copyright 2011 Mario Bensi <nef@ipsquad.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/

#include "categorymanager.h"

#include <QtCore/QAbstractItemModel>

#include <KDE/Akonadi/ItemModifyJob>
#include <KDE/KCalCore/Todo>
#include <KDE/KDebug>
#include <KDE/KGlobal>
#include <KDE/KLocale>
#include <KDE/KMessageBox>

#include "globaldefs.h"
#include "todohelpers.h"

K_GLOBAL_STATIC(CategoryManager, s_categoryManager)

CategoryManager &CategoryManager::instance()
{
    return *s_categoryManager;
}


CategoryManager::CategoryManager(QObject *parent)
    : QObject(parent)
//     , m_model(0)
{
}

CategoryManager::~CategoryManager()
{
}

void CategoryManager::setCategoriesStructure(CategoriesStructure *s)
{
    m_categoriesStructure = s;
}

const QChar CategoryManager::pathSeparator()
{
    return QChar(0x2044);
}

void CategoryManager::addCategory(const QString &category, const QString &parentCategory)
{
    QString categoryPath;
    if (parentCategory.isEmpty()) {
        categoryPath = category;
    } else {
        categoryPath = parentCategory + CategoryManager::pathSeparator() + category;
    }
    m_categoriesStructure->createCategoryNode(categoryPath, IdList());
}


bool CategoryManager::removeCategories(QWidget* parent, const IdList& categories)
{
    QStringList categoryList;
    foreach (Id category, categories) {
        categoryList << m_categoriesStructure->getName(category);
    }
    QString categoryName = categoryList.join(", ");
    kDebug() << categories << categoryList;
    QString title;
    QString text;
    if (categories.size() > 1) {
        text = i18n("Do you really want to delete the context '%1'? All actions won't be associated to this context anymore.", categoryName);
        title = i18n("Delete Context");
    } else {
        text = i18n("Do you really want to delete the contexts '%1'? All actions won't be associated to those contexts anymore.", categoryName);
        title = i18n("Delete Contexts");
    }
    int button = KMessageBox::questionYesNo(parent, text, title);
    bool canRemove = (button==KMessageBox::Yes);

    if (!canRemove) {
        return false;
    }
    kDebug() << "remove " << categories;
    foreach (Id id, categories) {
        m_categoriesStructure->removeNode(id);
    }
    return true;
}


bool CategoryManager::removeCategory(const Id &id)
{
    kDebug() << id;
    m_categoriesStructure->removeNode(id);
    return true;
//     int pos = m_categories.indexOf(categoryPath);
//     if (pos != -1) {
//         removeCategoryFromTodo(QModelIndex(), categoryPath);
//         m_categories.removeAt(pos);
//         emit categoryRemoved(categoryPath);
//         return true;
//     }
//     return false;
}

// void CategoryManager::onSourceInsertRows(const QModelIndex &sourceIndex, int begin, int end)
// {
//     for (int i=begin; i<=end; ++i) {
//         QModelIndex sourceChildIndex = m_model->index(i, 0, sourceIndex);
//         if (!sourceChildIndex.isValid()) {
//             continue;
//         }
//         Zanshin::ItemType type = (Zanshin::ItemType) sourceChildIndex.data(Zanshin::ItemTypeRole).toInt();
//         if (type==Zanshin::StandardTodo) {
//             QStringList categories = m_model->data(sourceChildIndex, Zanshin::CategoriesRole).toStringList();
//             foreach (QString category, categories) {
//                 addCategory(category);
//             }
//         } else if (type==Zanshin::Collection) {
//             onSourceInsertRows(sourceChildIndex, 0, m_model->rowCount(sourceChildIndex)-1);
//         }
//     }
// }

// void CategoryManager::onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end)
// {
//     for (int row=begin.row(); row<=end.row(); ++row) {
//         QModelIndex sourceIndex = begin.sibling(row, 0);
//         QSet<QString> newCategories = QSet<QString>::fromList(sourceIndex.data(Zanshin::CategoriesRole).toStringList());
// 
//         QSet<QString> oldCategories = QSet<QString>::fromList(m_categories);
//         QSet<QString> interCategories = newCategories;
//         interCategories.intersect(oldCategories);
//         newCategories-= interCategories;
// 
//         foreach (const QString &newCategory, newCategories) {
//             addCategory(newCategory);
//         }
//     }
// }

// void CategoryManager::removeCategoryFromTodo(const QModelIndex &sourceIndex, const QString &categoryPath)
// {
//     for (int i=0; i < m_model->rowCount(sourceIndex); ++i) {
//         QModelIndex child = m_model->index(i, 0, sourceIndex);
// //         dissociateTodoFromCategory(child, categoryPath);
//         removeCategoryFromTodo(child, categoryPath);
//     }
// }

bool CategoryManager::dissociateFromCategory(const Akonadi::Item& item, Id category)
{
    kDebug() << item.id() << category;
    if (!item.isValid()) {
        return false;
    }
    Id id = m_categoriesStructure->addItem(item);
    IdList parents = m_categoriesStructure->getParents(id);
    parents.removeAll(category);
    m_categoriesStructure->moveNode(id, parents);
    return true;
}

bool CategoryManager::moveToCategory(Id id, Id category, Zanshin::ItemType parentType)
{
    kDebug() << id << category;
    if (parentType!=Zanshin::Category && parentType!=Zanshin::CategoryRoot) { //TODO shouldn't be necessary
        return false;
    }
    m_categoriesStructure->moveNode(id, IdList() << category);
    return true;
}



// bool CategoryManager::dissociateTodoFromCategory(const QModelIndex &index, const QString &categoryPath)
// {
//     if (!index.isValid()) {
//         return false;
//     }
// 
//     const Akonadi::Item item = index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
//     if (!item.isValid()) {
//         return false;
//     }
//     Id id = m_categoriesStructure->addItem(item);
//     IdList parents = m_categoriesStructure->getParents(id);
//     m_categoriesStructure->moveNode();
//     return true;
// // 
// //     KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();
// // 
// //     if (!todo) {
// //         return false;
// //     }
// // 
// //     QStringList categories = todo->categories();
// //     if (categories.contains(categoryPath)) {
// //         categories.removeAll(categoryPath);
// //         todo->setCategories(categories);
// //         new Akonadi::ItemModifyJob(item);
// //         return true;
// //     }
// //     return false;
// }

// void CategoryManager::renameCategory(const QString &oldCategoryPath, const QString &newCategoryPath)
// {
//     if (oldCategoryPath == newCategoryPath) {
//         return;
//     }
// 
//     emit categoryRenamed(oldCategoryPath, newCategoryPath);
// 
//     m_categories.removeAll(oldCategoryPath);
//     m_categories << newCategoryPath;
// 
//     renameCategory(QModelIndex(), oldCategoryPath, newCategoryPath);
// }

// void CategoryManager::renameCategory(const QModelIndex &sourceIndex, const QString &oldCategoryPath, const QString &newCategoryPath)
// {
//     for (int i=0; i < m_model->rowCount(sourceIndex); ++i) {
//         QModelIndex child = m_model->index(i, 0, sourceIndex);
//         if (child.isValid()) {
//             const Akonadi::Item item = child.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
//             if (item.isValid()) {
//                 KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();
//                 if (todo) {
//                     QStringList categories = todo->categories();
//                     if (categories.contains(oldCategoryPath)) {
//                         categories = categories.replaceInStrings(oldCategoryPath, newCategoryPath);
//                         todo->setCategories(categories);
//                         new Akonadi::ItemModifyJob(item);
//                     }
//                 }
//             }
//         }
//         renameCategory(child, oldCategoryPath, newCategoryPath);
//     }
// }

// void CategoryManager::moveCategory(const QString &oldCategoryPath, const QString &parentCategoryPath, Zanshin::ItemType parentType)
// {
// //     if (parentType!=Zanshin::Category && parentType!=Zanshin::CategoryRoot) {
// //         return;
// //     }
// // 
// //     QString categoryName = oldCategoryPath.split(CategoryManager::pathSeparator()).last();
// //     QString newCategoryPath;
// //     if (parentType==Zanshin::Category) {
// //         newCategoryPath = parentCategoryPath + CategoryManager::pathSeparator() + categoryName;
// //     } else {
// //         newCategoryPath = categoryName;
// //     }
// // 
// //     if (oldCategoryPath == newCategoryPath) {
// //         return;
// //     }
// //     addCategory(newCategoryPath);
// //     emit categoryMoved(oldCategoryPath, newCategoryPath);
// //     removeCategory(oldCategoryPath);
// }

// bool CategoryManager::moveTodoToCategory(const QModelIndex &index, const QString &categoryPath, const Zanshin::ItemType parentType)
// {
//     const Akonadi::Item item = index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
//     return moveTodoToCategory(item, categoryPath, parentType);
// }

// bool CategoryManager::moveTodoToCategory(const Akonadi::Item &item, const QString &categoryPath, const Zanshin::ItemType parentType)
// {
//     KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();
//     if (!todo) {
//         return false;
//     }
//     QStringList categories;
//     if (parentType != Zanshin::Inbox && parentType != Zanshin::CategoryRoot) {
//         categories= todo->categories();
//         if (!categories.contains(categoryPath)) {
//             categories << categoryPath;
//         }
//     }
//     todo->setCategories(categories);
//     new Akonadi::ItemModifyJob(item);
//     return true;
// }
