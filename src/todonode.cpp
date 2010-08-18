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

#include "todonode.h"

#include <KDebug>

const QChar TodoNode::pathSeparator = QChar(0x2044);

TodoNode::TodoNode(const QModelIndex &rowSourceIndex, TodoNode *parent)
    : m_parent(parent), m_rowSourceIndex(rowSourceIndex)
{
    init();
}

TodoNode::TodoNode(TodoNode *parent)
    : m_parent(parent)
{
    init();
}

TodoNode::~TodoNode()
{
    if (m_parent) {
        m_parent->m_children.removeAll(this);
    }

    qDeleteAll(m_children);
}

void TodoNode::init()
{
    if (m_parent) {
        m_parent->m_children << this;
    }

    // Setting default flags
    if (!m_rowSourceIndex.isValid()) {
        m_flags = Qt::ItemIsEnabled|Qt::ItemIsSelectable;
    }
}

TodoNode *TodoNode::parent() const
{
    return m_parent;
}

QList<TodoNode*> TodoNode::children() const
{
    return m_children;
}

QModelIndex TodoNode::rowSourceIndex() const
{
    return m_rowSourceIndex;
}

QVariant TodoNode::data(int column, int role) const
{
    if (m_rowSourceIndex.isValid()) {
        return m_rowSourceIndex.sibling(m_rowSourceIndex.row(), column).data(role);
    } else {
        QPair<int, int> key(-1, role);
        if (!m_data.contains(key)) { // Row wide info has priority
            key.first = column;
        }
        return m_data[key];
    }
}

void TodoNode::setData(const QVariant &value, int column, int role)
{
    if (m_rowSourceIndex.isValid()) {
        QAbstractItemModel *model = const_cast<QAbstractItemModel*>(m_rowSourceIndex.model());
        model->setData(m_rowSourceIndex, value, role);
    } else {
        m_data[QPair<int, int>(column, role)] = value;
    }
}

void TodoNode::setRowData(const QVariant &value, int role)
{
    if (m_rowSourceIndex.isValid()) {
        QAbstractItemModel *model = const_cast<QAbstractItemModel*>(m_rowSourceIndex.model());
        model->setData(m_rowSourceIndex, value, role);
    } else {
        m_data[QPair<int, int>(-1, role)] = value;
    }
}

Qt::ItemFlags TodoNode::flags() const
{
    if (m_rowSourceIndex.isValid()) {
        QAbstractItemModel *model = const_cast<QAbstractItemModel*>(m_rowSourceIndex.model());
        return model->flags(m_rowSourceIndex);
    } else {
        return m_flags;
    }
}

void TodoNode::setFlags(Qt::ItemFlags flags)
{
    if (m_rowSourceIndex.isValid()) {
        kWarning() << "Trying to set flags on a non-virtual node.";
        return;
    }

    m_flags = flags;
}
