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

#include <KDE/KDebug>
#include <KDE/KIcon>
#include <KDE/KLocale>

#include "todomodel.h"
#include "todonode.h"
#include "todonodemanager.h"

TodoCategoriesModel::TodoCategoriesModel(QObject *parent)
    : TodoProxyModelBase(MultiMapping, parent), m_categoryRootNode(0)
{
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
                    if (!parent) {
                        parent = createCategoryNode(category);
                    }
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
                QString category = categoryNode->data(0, Qt::DisplayRole).toString();
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
            if (!m_categoryMap.contains(newCategory)) {
                createCategoryNode(newCategory);
            }

            TodoNode *parent = m_categoryMap[newCategory];
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

TodoNode *TodoCategoriesModel::createCategoryNode(const QString &category)
{
    //TODO: Order them along a tree
    int row = m_categoryRootNode->children().size();

    beginInsertRows(m_manager->indexForNode(m_categoryRootNode, 0), row, row);

    TodoNode *node = new TodoNode(m_categoryRootNode);
    node->setData(category, 0, Qt::DisplayRole);
    node->setData(category, 0, Qt::EditRole);
    node->setData(KIcon("view-pim-notes"), 0, Qt::DecorationRole);
    node->setRowData(TodoModel::Category, TodoModel::ItemTypeRole);

    m_categoryMap[category] = node;
    m_manager->insertNode(node);

    endInsertRows();

    return node;
}
