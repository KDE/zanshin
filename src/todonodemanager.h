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

#ifndef ZANSHIN_TODONODEMANAGER_H
#define ZANSHIN_TODONODEMANAGER_H

#include <QtCore/QPersistentModelIndex>

class TodoNode;
class TodoProxyModelBase;

class TodoNodeManager
{
public:
    explicit TodoNodeManager(TodoProxyModelBase *model, bool multiMapping = false);

    QModelIndex index(int row, int column, TodoNode *parent) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;

    QModelIndex indexForNode(TodoNode *node, int column) const;
    TodoNode *nodeForIndex(const QModelIndex &index) const;

    TodoNode *nodeForSourceIndex(const QModelIndex &sourceIndex) const;
    QList<TodoNode*> nodesForSourceIndex(const QModelIndex &sourceIndex) const;

    void insertNode(TodoNode *node);
    void removeNode(TodoNode *node);

    QList<TodoNode*> roots() const;

    bool isMultiMapping() const;

protected:
    TodoProxyModelBase *m_model;
    bool m_multiMapping;
    QList<TodoNode*> m_roots;
    QMultiHash<QPersistentModelIndex, TodoNode*> m_nodes;
};

#endif

