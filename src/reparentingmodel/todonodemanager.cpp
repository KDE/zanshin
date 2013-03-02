/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>
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

#include "todonodemanager.h"

#include "todonode.h"
#include "todoproxymodelbase.h"
#include <QDebug>

TodoNodeManager::TodoNodeManager(TodoProxyModelBase *model, bool multiMapping)
    : m_model(model), m_multiMapping(multiMapping)
{

}

QModelIndex TodoNodeManager::index(int row, int column, TodoNode *parent) const
{
    if (row < 0 || column < 0) {
        return QModelIndex();
    }
    //We check the following two conditions only in asserts for performance reasons because they are very expensive in here
    //indexForNode results in nested calls up to the root node and should be avoided
    //The asserts are commented because they break the modeltest. It is safe to not check here, because rows have already
    //been checked, and we don't care about columns.
    //Q_ASSERT(row < m_model->rowCount(indexForNode(parent, 0)));
    //Q_ASSERT(column < m_model->columnCount(indexForNode(parent, 0)));

    return m_model->createIndex(row, column, parent);
}

QModelIndex TodoNodeManager::index(int row, int column, const QModelIndex &parent) const
{
    TodoNode *p = nodeForIndex(parent);
    if ( (p && (row >= p->children().size())) ||
        (!p && (row >= m_roots.size())) ) {
        return QModelIndex();
    }
    return index(row, column, p);
}

QModelIndex TodoNodeManager::indexForNode(TodoNode *node, int column) const
{
    if (!node) {
        return QModelIndex();
    } else {
        TodoNode *parent = node->parent();
        int row = 0;
        if (parent) {
            row = parent->children().indexOf(node);
        } else {
            row = m_roots.indexOf(node);
        }

        return index(row, column, parent);
    }
}

TodoNode *TodoNodeManager::nodeForIndex(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 0;
    }

    TodoNode *parentNode = static_cast<TodoNode*>(index.internalPointer());
    if (parentNode != 0) {
        Q_ASSERT(parentNode->children().size() > index.row());
        if (parentNode->children().size() <= index.row())
            return 0;
        return parentNode->children().at(index.row());
    } else {
        return m_roots.at(index.row());
    }
}

TodoNode *TodoNodeManager::nodeForSourceIndex(const QModelIndex &sourceIndex) const
{
    Q_ASSERT(!m_multiMapping);

    QModelIndex rowSourceIndex = sourceIndex.sibling(sourceIndex.row(), 0);
    if (rowSourceIndex.isValid() && m_nodes.contains(rowSourceIndex)) {
        return m_nodes.value(rowSourceIndex);
    } else {
        return 0;
    }
}

QList<TodoNode*> TodoNodeManager::nodesForSourceIndex(const QModelIndex &sourceIndex) const
{
    Q_ASSERT(m_multiMapping);

    QModelIndex rowSourceIndex = sourceIndex.sibling(sourceIndex.row(), 0);
    if (rowSourceIndex.isValid() && m_nodes.contains(rowSourceIndex)) {
        return m_nodes.values(sourceIndex);
    } else {
        return QList<TodoNode*>();
    }
}

void TodoNodeManager::insertNode(TodoNode *node)
{
    Q_ASSERT(node);
    if (node->rowSourceIndex().isValid()) {
        Q_ASSERT( (!m_multiMapping && m_nodes.count(node->rowSourceIndex())==0) || m_multiMapping );
        m_nodes.insert(node->rowSourceIndex(), node);
    }

    if (node->parent()==0) {
        m_roots << node;
    }
}

void TodoNodeManager::removeNode(TodoNode *node)
{
    Q_ASSERT(node);
    if (node->rowSourceIndex().isValid()) {
        m_nodes.remove(node->rowSourceIndex(), node);
    }

    if (node->parent()==0) {
        m_roots.removeAll(node);
    }
}

// void TodoNodeManager::replaceNode(TodoNode *oldNode, TodoNode *newNode)
// {
//     removeNode(oldNode);
//     foreach()
// }

QList<TodoNode*> TodoNodeManager::roots() const
{
    return m_roots;
}

bool TodoNodeManager::isMultiMapping() const
{
    return m_multiMapping;
}
