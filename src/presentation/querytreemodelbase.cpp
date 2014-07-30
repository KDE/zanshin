/* This file is part of Zanshin

   Copyright 2014 Mario Bensi <mbensi@ipsquad.net>
   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#include "querytreemodelbase.h"

using namespace Presentation;

QueryTreeNodeBase::QueryTreeNodeBase(QueryTreeNodeBase *parent, QueryTreeModelBase *model)
    : m_parent(parent),
      m_model(model)
{
}

QueryTreeNodeBase::~QueryTreeNodeBase()
{
    qDeleteAll(m_childNode);
}

int QueryTreeNodeBase::row()
{
    return m_parent ? m_parent->m_childNode.indexOf(this) : -1;
}

QueryTreeNodeBase *QueryTreeNodeBase::parent() const
{
    return m_parent;
}

QueryTreeNodeBase *QueryTreeNodeBase::child(int row) const
{
    if (row >= 0 && row < m_childNode.size())
        return m_childNode.value(row);
    else
        return 0;
}

void QueryTreeNodeBase::insertChild(int row, QueryTreeNodeBase *node)
{
    m_childNode.insert(row, node);
}

void QueryTreeNodeBase::appendChild(QueryTreeNodeBase *node)
{
    m_childNode.append(node);
}

void QueryTreeNodeBase::removeChildAt(int row)
{
    delete m_childNode.takeAt(row);
}

int QueryTreeNodeBase::childCount() const
{
    return m_childNode.size();
}

QModelIndex QueryTreeNodeBase::index(int row, int column, const QModelIndex &parent) const
{
    return m_model->index(row, column, parent);
}

QModelIndex QueryTreeNodeBase::createIndex(int row, int column, void *data) const
{
    return m_model->createIndex(row, column, data);
}

void QueryTreeNodeBase::beginInsertRows(const QModelIndex &parent, int first, int last)
{
    m_model->beginInsertRows(parent, first, last);
}

void QueryTreeNodeBase::endInsertRows()
{
    m_model->endInsertRows();
}

void QueryTreeNodeBase::beginRemoveRows(const QModelIndex &parent, int first, int last)
{
    m_model->beginRemoveRows(parent, first, last);
}

void QueryTreeNodeBase::endRemoveRows()
{
    m_model->endRemoveRows();
}

void QueryTreeNodeBase::emitDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    emit m_model->dataChanged(topLeft, bottomRight);
}

QueryTreeModelBase::QueryTreeModelBase(QueryTreeNodeBase *rootNode, QObject *parent)
    : QAbstractItemModel(parent),
      m_rootNode(rootNode)
{
    auto roles = roleNames();
    roles.insert(ObjectRole, "object");
    roles.insert(IconNameRole, "icon");
    setRoleNames(roles);
}

QueryTreeModelBase::~QueryTreeModelBase()
{
    delete m_rootNode;
}

Qt::ItemFlags QueryTreeModelBase::flags(const QModelIndex &index) const
{
    if (!isModelIndexValid(index)) {
        return Qt::NoItemFlags;
    }

    return nodeFromIndex(index)->flags();
}

QModelIndex QueryTreeModelBase::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0)
        return QModelIndex();

    const QueryTreeNodeBase *parentNode = nodeFromIndex(parent);

    if (row < parentNode->childCount()) {
        QueryTreeNodeBase *node = parentNode->child(row);
        return createIndex(row, column, node);
    } else {
        return QModelIndex();
    }
}

QModelIndex QueryTreeModelBase::parent(const QModelIndex &index) const
{
    QueryTreeNodeBase *node = nodeFromIndex(index);
    if (!node->parent() || node->parent() == m_rootNode)
        return QModelIndex();
    else
        return createIndex(node->parent()->row(), 0, node->parent());
}

int QueryTreeModelBase::rowCount(const QModelIndex &index) const
{
    return nodeFromIndex(index)->childCount();
}

int QueryTreeModelBase::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant QueryTreeModelBase::data(const QModelIndex &index, int role) const
{
    if (!isModelIndexValid(index)) {
        return QVariant();
    }

    return nodeFromIndex(index)->data(role);
}

bool QueryTreeModelBase::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!isModelIndexValid(index)) {
        return false;
    }

    return nodeFromIndex(index)->setData(value, role);
}

QueryTreeNodeBase *QueryTreeModelBase::nodeFromIndex(const QModelIndex &index) const
{
    return index.isValid() ? static_cast<QueryTreeNodeBase*>(index.internalPointer()) : m_rootNode;
}

bool QueryTreeModelBase::isModelIndexValid(const QModelIndex &index) const
{
    bool valid = index.isValid()
        && index.column() == 0
        && index.row() >= 0;

    if (!valid)
        return false;

    const QueryTreeNodeBase *parentNode = nodeFromIndex(index.parent());
    const int count = parentNode->childCount();
    return index.row() < count;
}
