/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>
   Copyright 2008, 2009 Mario Bensi <nef@ipsquad.net>

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

#include "todocategoriesmodel.h"

#include <QtCore/QStringList>
#include <QtCore/QMimeData>

#include <KDE/Akonadi/ItemFetchJob>
#include <KDE/Akonadi/ItemFetchScope>
#include <KDE/KDebug>
#include <KDE/KIcon>
#include <KDE/KLocale>
#include <KDE/KUrl>

#include "categorymanager.h"
#include "todohelpers.h"
#include "todomodel.h"
#include "todonode.h"
#include "todonodemanager.h"

TodoCategoriesModel::TodoCategoriesModel(QObject *parent)
    : TodoProxyModelBase(MultiMapping, parent), m_categoryRootNode(0)
{
    connect(&CategoryManager::instance(), SIGNAL(categoryAdded(const QString&)),
            this, SLOT(createCategoryNode(const QString&)));
    connect(&CategoryManager::instance(), SIGNAL(categoryRemoved(const QString&)),
            this, SLOT(removeCategoryNode(const QString&)));
    connect(&CategoryManager::instance(), SIGNAL(categoryRenamed(const QString&, const QString&)),
            this, SLOT(renameCategoryNode(const QString&, const QString&)));
    connect(&CategoryManager::instance(), SIGNAL(categoryMoved(const QString&, const QString&)),
            this, SLOT(moveCategoryNode(const QString&, const QString&)));
}

TodoCategoriesModel::~TodoCategoriesModel()
{
}

void TodoCategoriesModel::onSourceInsertRows(const QModelIndex &sourceIndex, int begin, int end)
{
    for (int i = begin; i <= end; i++) {
        QModelIndex sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);

        if (!sourceChildIndex.isValid()) {
            continue;
        }

        TodoModel::ItemType type = (TodoModel::ItemType) sourceChildIndex.data(TodoModel::ItemTypeRole).toInt();
        if (type==TodoModel::StandardTodo) {
            QStringList categories = sourceModel()->data(sourceChildIndex, TodoModel::CategoriesRole).toStringList();

            if (categories.isEmpty()) {
                addChildNode(sourceChildIndex, m_inboxNode);

            } else {
                foreach (const QString &category, categories) {
                    TodoNode *parent = m_categoryMap[category];
                    Q_ASSERT(parent);
                    addChildNode(sourceChildIndex, parent);
                }
            }
        } else if (type==TodoModel::Collection) {
            onSourceInsertRows(sourceChildIndex, 0, sourceModel()->rowCount(sourceChildIndex)-1);
        }
    }
}

void TodoCategoriesModel::onSourceRemoveRows(const QModelIndex &sourceIndex, int begin, int end)
{
    for (int i = begin; i <= end; ++i) {
        QModelIndex sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);
        QModelIndexList proxyIndexes = mapFromSourceAll(sourceChildIndex);

        foreach (const QModelIndex &proxyIndex, proxyIndexes) {
            TodoNode *node = m_manager->nodeForIndex(proxyIndex);

            beginRemoveRows(proxyIndex.parent(), proxyIndex.row(), proxyIndex.row());
            m_manager->removeNode(node);
            delete node;
            endRemoveRows();
        }
    }
}

void TodoCategoriesModel::onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end)
{
    for (int row=begin.row(); row<=end.row(); ++row) {
        QModelIndex sourceIndex = begin.sibling(row, 0);
        QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(sourceIndex);

        QSet<QString> oldCategories;
        QHash<QString, TodoNode*> nodeMap;
        foreach (TodoNode *node, nodes) {
            QModelIndex begin = m_manager->indexForNode(node, 0);
            QModelIndex end = m_manager->indexForNode(node, qMax(begin.column(), end.column()));
            emit dataChanged(begin, end);

            TodoNode *categoryNode = node->parent();
            if (categoryNode
             && categoryNode->data(0, TodoModel::ItemTypeRole).toInt()!=TodoModel::Inbox) {
                QString category = categoryNode->data(0, TodoModel::CategoryPathRole).toString();
                oldCategories << category;
                nodeMap[category] = node;
            }
        }

        QSet<QString> newCategories = QSet<QString>::fromList(sourceIndex.data(TodoModel::CategoriesRole).toStringList());

        QSet<QString> interCategories = newCategories;
        interCategories.intersect(oldCategories);
        newCategories-= interCategories;
        oldCategories-= interCategories;

        foreach (const QString &oldCategory, oldCategories) {
            TodoNode *parentNode = m_categoryMap[oldCategory];
            TodoNode *node = nodeMap[oldCategory];

            int oldRow = parentNode->children().indexOf(node);
            beginRemoveRows(m_manager->indexForNode(parentNode, 0), oldRow, oldRow);
            m_manager->removeNode(node);
            delete node;
            endRemoveRows();
        }

        if (!oldCategories.isEmpty()) {
            QStringList categories = sourceModel()->data(sourceIndex, TodoModel::CategoriesRole).toStringList();
            if (categories.isEmpty()) {
                addChildNode(sourceIndex, m_inboxNode);
            }
        } else if (!newCategories.isEmpty()) {
            TodoNode *node = 0;
            QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(sourceIndex);
            foreach (TodoNode *n, nodes) {
                if (n->parent() == m_inboxNode) {
                    node = n;
                    break;
                }
            }
            if (node) {
                int oldRow = m_inboxNode->children().indexOf(node);
                beginRemoveRows(m_manager->indexForNode(m_inboxNode, 0), oldRow, oldRow);
                m_manager->removeNode(node);
                delete node;
                endRemoveRows();
            }
        }

        foreach (const QString &newCategory, newCategories) {
            TodoNode *parent = m_categoryMap[newCategory];
            Q_ASSERT(parent);
            addChildNode(sourceIndex, parent);
        }
    }
}

void TodoCategoriesModel::init()
{
    TodoProxyModelBase::init();

    if (!m_categoryRootNode) {
        beginInsertRows(QModelIndex(), 1, 1);

        TodoNode *node = new TodoNode;
        node->setData(i18n("Categories"), 0, Qt::DisplayRole);
        node->setData(KIcon("document-multiple"), 0, Qt::DecorationRole);
        node->setRowData(TodoModel::CategoryRoot, TodoModel::ItemTypeRole);

        m_categoryRootNode = node;
        m_manager->insertNode(m_categoryRootNode);

        endInsertRows();
    }
}

TodoNode *TodoCategoriesModel::createInbox() const
{
    TodoNode *node = new TodoNode;

    node->setData(i18n("No Category"), 0, Qt::DisplayRole);
    node->setData(KIcon("mail-folder-inbox"), 0, Qt::DecorationRole);
    node->setRowData(TodoModel::Inbox, TodoModel::ItemTypeRole);

    return node;
}

void TodoCategoriesModel::createCategoryNode(const QString &categoryPath)
{
    //TODO: Order them along a tree
    TodoNode* parentNode = m_categoryRootNode;
    QString categoryName = categoryPath;
    if (categoryPath.contains(CategoryManager::pathSeparator())) {
        QString parentCategory = categoryPath.left(categoryPath.lastIndexOf(CategoryManager::pathSeparator()));
        categoryName = categoryPath.split(CategoryManager::pathSeparator()).last();
        parentNode = m_categoryMap[parentCategory];
        if (!parentNode) {
            CategoryManager::instance().addCategory(parentCategory);
            parentNode = m_categoryMap[parentCategory];
        }
    }

    int row = parentNode->children().size();

    beginInsertRows(m_manager->indexForNode(parentNode, 0), row, row);

    TodoNode *node = new TodoNode(parentNode);
    node->setData(categoryName, 0, Qt::DisplayRole);
    node->setData(categoryName, 0, Qt::EditRole);
    node->setData(categoryPath, 0, TodoModel::CategoryPathRole);
    node->setData(KIcon("view-pim-notes"), 0, Qt::DecorationRole);
    node->setRowData(TodoModel::Category, TodoModel::ItemTypeRole);

    m_categoryMap[categoryPath] = node;
    m_manager->insertNode(node);

    endInsertRows();
}

void TodoCategoriesModel::removeCategoryNode(const QString &category)
{
    if (!m_categoryMap.contains(category)) {
        return;
    }

    TodoNode *node = m_categoryMap[category];

    QList<TodoNode*> children = node->children();
    foreach (TodoNode* child, children) {
        QModelIndex childIndex = m_manager->indexForNode(child, 0);
        if (childIndex.data(TodoModel::ItemTypeRole).toInt() == TodoModel::Category) {
            CategoryManager::instance().removeCategory(childIndex.data(TodoModel::CategoryPathRole).toString());
        } else {
            QStringList categories = childIndex.data(TodoModel::CategoriesRole).toStringList();
            if (categories.empty()) {
                child->setParent(m_inboxNode);
            } else {
                beginRemoveRows(childIndex.parent(), childIndex.row(), childIndex.row());
                m_manager->removeNode(child);
                delete child;
                endRemoveRows();
            }
        }
    }

    QModelIndex index = m_manager->indexForNode(node, 0);
    beginRemoveRows(index.parent(), index.row(), index.row());
    m_manager->removeNode(node);
    m_categoryMap.remove(category);
    delete node;
    endRemoveRows();
}

void TodoCategoriesModel::renameCategoryNode(const QString &oldCategoryPath, const QString &newCategoryPath)
{
    TodoNode *node = m_categoryMap[oldCategoryPath];
    m_categoryMap[newCategoryPath] = node;
    m_categoryMap.remove(oldCategoryPath);

    QList<TodoNode*> children = node->children();
    foreach (TodoNode* child, children) {
        QModelIndex childIndex = m_manager->indexForNode(child, 0);
        if (childIndex.data(TodoModel::ItemTypeRole).toInt() == TodoModel::Category) {
            QString childPath = childIndex.data(TodoModel::CategoryPathRole).toString();
            QString newChildPath = childPath;
            newChildPath = newChildPath.replace(oldCategoryPath, newCategoryPath);
            CategoryManager::instance().renameCategory(childPath, newChildPath);
        }
    }

    QString newCategory = newCategoryPath.split(CategoryManager::pathSeparator()).last();
    node->setData(newCategory, 0, Qt::DisplayRole);
    node->setData(newCategory, 0, Qt::EditRole);
    node->setData(newCategoryPath, 0, TodoModel::CategoryPathRole);

    QModelIndex index = m_manager->indexForNode(node, 0);
    emit dataChanged(index, index);
}

void TodoCategoriesModel::moveCategoryNode(const QString &oldCategoryPath, const QString &newCategoryPath)
{
    TodoNode *node = m_categoryMap[oldCategoryPath];

    QList<TodoNode*> children = node->children();
    foreach (TodoNode* child, children) {
        QModelIndex childIndex = m_manager->indexForNode(child, 0);
        if (childIndex.data(TodoModel::ItemTypeRole).toInt() == TodoModel::Category) {
            QString ChildPath = childIndex.data(TodoModel::CategoryPathRole).toString();
            QString newChildPath = ChildPath;
            newChildPath = newChildPath.replace(oldCategoryPath, newCategoryPath);
            CategoryManager::instance().moveCategory(ChildPath, newChildPath);
        } else {
            CategoryManager::instance().moveTodoToCategory(childIndex, newCategoryPath, TodoModel::Category);
        }
    }

}

Qt::ItemFlags TodoCategoriesModel::flags(const QModelIndex &index) const
{
    TodoModel::ItemType type = (TodoModel::ItemType) index.data(TodoModel::ItemTypeRole).toInt();
    if (type == TodoModel::Inbox || type == TodoModel::CategoryRoot) {
        return Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    }
    return TodoProxyModelBase::flags(index) | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
}

QMimeData *TodoCategoriesModel::mimeData(const QModelIndexList &indexes) const
{
    QModelIndexList sourceIndexes;
    QStringList categoriesList;
    foreach (const QModelIndex &proxyIndex, indexes) {
        TodoNode *node = m_manager->nodeForIndex(proxyIndex);
        QModelIndex index = m_manager->indexForNode(node, 0);
        TodoModel::ItemType type = (TodoModel::ItemType) index.data(TodoModel::ItemTypeRole).toInt();
        if (type==TodoModel::StandardTodo) {
            sourceIndexes << mapToSource(proxyIndex);
        } else {
            categoriesList << proxyIndex.data(TodoModel::CategoryPathRole).toString();
        }
    }

    if (!sourceIndexes.isEmpty()) {
        return sourceModel()->mimeData(sourceIndexes);
    } else {
        QMimeData *mimeData = new QMimeData();
        QString sep = CategoryManager::pathSeparator();
        sep += CategoryManager::pathSeparator();
        QByteArray categories = categoriesList.join(sep).toUtf8();
        mimeData->setData("application/x-vnd.zanshin.category", categories);
        return mimeData;
    }
}

QStringList TodoCategoriesModel::mimeTypes() const
{
    QStringList types;
    types << sourceModel()->mimeTypes();
    types << "application/x-vnd.zanshin.category";
    return types;
}


bool TodoCategoriesModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action,
                                 int /*row*/, int /*column*/, const QModelIndex &parent)
{
    if (action != Qt::MoveAction || (!KUrl::List::canDecode(mimeData) && !mimeData->hasFormat("application/x-vnd.zanshin.category"))) {
        return false;
    }

    QString parentCategory = parent.data(TodoModel::CategoryPathRole).toString();
    TodoModel::ItemType parentType = (TodoModel::ItemType)parent.data(TodoModel::ItemTypeRole).toInt();

    if (KUrl::List::canDecode(mimeData)) {
        KUrl::List urls = KUrl::List::fromMimeData(mimeData);
        foreach (const KUrl &url, urls) {
            const Akonadi::Item urlItem = Akonadi::Item::fromUrl(url);
            if (urlItem.isValid()) {
                Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(urlItem);
                Akonadi::ItemFetchScope scope;
                scope.setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
                scope.fetchFullPayload();
                job->setFetchScope(scope);

                if ( !job->exec() ) {
                    return false;
                }

                foreach (const Akonadi::Item &item, job->items()) {
                    if (item.hasPayload<KCalCore::Todo::Ptr>()) {
                        QModelIndexList indexes = Akonadi::EntityTreeModel::modelIndexesForItem(sourceModel(), item);
                        if (indexes.isEmpty()) {
                            return false;
                        }
                        QModelIndex index = indexes.first();
                        return CategoryManager::instance().moveTodoToCategory(index, parentCategory, parentType);
                    }
                }
            }
        }
    } else {
        QByteArray categories = mimeData->data("application/x-vnd.zanshin.category");
        QString sep = CategoryManager::pathSeparator();
        sep += CategoryManager::pathSeparator();
        QStringList categoriesPath = QString::fromUtf8(categories.data()).split(sep);
        foreach (QString categoryPath, categoriesPath) {
            QString CategoryName = categoryPath.split(CategoryManager::pathSeparator()).last();
            QString newCategoryPath = parentCategory + CategoryManager::pathSeparator() + CategoryName;
            if (newCategoryPath != categoryPath) {
                CategoryManager::instance().moveCategory(categoryPath, newCategoryPath);
            }
        }
    }
    return true;
}

Qt::DropActions TodoCategoriesModel::supportedDropActions() const
{
    return sourceModel()->supportedDropActions();
}

bool TodoCategoriesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role!=Qt::EditRole || !index.isValid()) {
        return TodoProxyModelBase::setData(index, value, role);
    }

    if (index.column()==0) {
        QString oldCategoryPath = index.data(TodoModel::CategoryPathRole).toString();
        QString newCategoryName = value.toString();
        QString newCategoryPath = oldCategoryPath.left(oldCategoryPath.lastIndexOf(CategoryManager::pathSeparator())+1) + newCategoryName;
        CategoryManager::instance().renameCategory(oldCategoryPath, newCategoryPath);
        return true;
    }

    return TodoProxyModelBase::setData(index, value, role);
}
