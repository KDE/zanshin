/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "querytreemodelbase.h"

#include <QMimeData>
#include <QStringList>

#include <algorithm>

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
        return nullptr;
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
    Q_EMIT m_model->dataChanged(topLeft, bottomRight);
}

QueryTreeModelBase::QueryTreeModelBase(QueryTreeNodeBase *rootNode, QObject *parent)
    : QAbstractItemModel(parent),
      m_rootIndexFlag(Qt::ItemIsDropEnabled),
      m_rootNode(rootNode)
{
}

QueryTreeModelBase::~QueryTreeModelBase()
{
    delete m_rootNode;
}

QHash<int, QByteArray> QueryTreeModelBase::roleNames() const
{
    auto roles = QAbstractItemModel::roleNames();
    roles.insert(ObjectRole, "object");
    roles.insert(IconNameRole, "icon");
    roles.insert(IsDefaultRole, "default");
    return roles;
}

Qt::ItemFlags QueryTreeModelBase::flags(const QModelIndex &index) const
{
    if (!isModelIndexValid(index))
        return m_rootIndexFlag;

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

    const_cast<QueryTreeModelBase *>(this)->fetchAdditionalInfo(index);

    return nodeFromIndex(index)->data(role);
}

bool QueryTreeModelBase::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!isModelIndexValid(index)) {
        return false;
    }

    return nodeFromIndex(index)->setData(value, role);
}

bool QueryTreeModelBase::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    // If that's not holding that mime type we can't do the cycle checking
    // this is relevant only for internal drag and drop anyway
    if (data->hasFormat(QStringLiteral("application/x-zanshin-indexes"))) {
        const auto indexes = data->property("indexes").value<QModelIndexList>();
        foreach (const auto &index, indexes) {
            auto p = parent;
            while (p.isValid()) {
                if (p == index) // Oops, we found a cycle (one of the indexes is parent of the drop point)
                    return false;
                p = p.parent();
            }
        }
    }

    return nodeFromIndex(parent)->dropMimeData(data, action);
}

QMimeData *QueryTreeModelBase::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty())
        return nullptr;

    auto data = createMimeData(indexes);
    data->setData(QStringLiteral("application/x-zanshin-indexes"), "indexes");
    data->setProperty("indexes", QVariant::fromValue(indexes));
    return data;
}

QStringList QueryTreeModelBase::mimeTypes() const
{
    return QAbstractItemModel::mimeTypes() << QStringLiteral("application/x-zanshin-object")
                                           << QStringLiteral("application/x-zanshin-indexes");
}

Qt::DropActions QueryTreeModelBase::supportedDragActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions QueryTreeModelBase::supportedDropActions() const
{
    return Qt::MoveAction;
}

QueryTreeNodeBase *QueryTreeModelBase::nodeFromIndex(const QModelIndex &index) const
{
    return index.isValid() ? static_cast<QueryTreeNodeBase*>(index.internalPointer()) : m_rootNode;
}

void QueryTreeModelBase::setRootIndexFlag(Qt::ItemFlags flags)
{
    m_rootIndexFlag = flags;
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
