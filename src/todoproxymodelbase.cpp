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

#include "todoproxymodelbase.h"

#include <QtCore/QStringList>

#include <KDE/KDebug>
#include <KDE/KIcon>
#include <KDE/KLocale>

#include "todonode.h"
#include "todonodemanager.h"

TodoProxyModelBase::TodoProxyModelBase(MappingType type, QObject *parent)
    : QAbstractProxyModel(parent),
      m_manager(new TodoNodeManager(this, (type==MultiMapping ? true : false))),
      m_inboxNode(0), m_mappingType(type)
{
}

TodoProxyModelBase::~TodoProxyModelBase()
{
    delete m_manager;
}

void TodoProxyModelBase::init()
{
    if (!m_inboxNode) {
        beginInsertRows(QModelIndex(), 0, 0);
        m_inboxNode = createInbox();
        m_manager->insertNode(m_inboxNode);
        endInsertRows();
    }
}

QModelIndex TodoProxyModelBase::index(int row, int column, const QModelIndex &parent) const
{
    return m_manager->index(row, column, parent);
}

QModelIndex TodoProxyModelBase::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    TodoNode *parent = m_manager->nodeForIndex(index)->parent();

    if (parent == 0) {
        return QModelIndex();
    } else {
        return m_manager->indexForNode(parent, 0);
    }
}

int TodoProxyModelBase::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_manager->roots().size();
    } else if (parent.column() == 0) { // Only one set of children per row
        TodoNode *node = m_manager->nodeForIndex(parent);
        return node->children().size();
    }

    return 0;
}

int TodoProxyModelBase::columnCount(const QModelIndex &parent) const
{
    if (!sourceModel()) {
        return 1;
    }

    if (parent.isValid()) {
        TodoNode *node = m_manager->nodeForIndex(parent);
        if (node && node->children().size() > 0) {
            return sourceModel()->columnCount();
        } else {
            return 0;
        }
    } else {
        if (sourceModel()->rowCount(mapToSource(parent))) {
            return sourceModel()->columnCount(mapToSource(parent));
        } else {
            return 1;
        }
    }
}

bool TodoProxyModelBase::hasChildren(const QModelIndex &parent) const
{
    // Since Qt 4.8, QAbstractProxyModel just forwards hasChildren to the source model
    // so that's not based on the current model rowCount() and columnCount(), because
    // of that it's easy to end up with inconsistent replies between rowCount() and
    // hasChildren() for instance (resulting in empty views or crashes in proxies).
    //
    // So, here we just use the hasChildren() implementation inherited from QAbstractItemModel
    // since it is based on rowCount() and columnCount().

    return QAbstractItemModel::hasChildren(parent);
}

QVariant TodoProxyModelBase::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical) {
        return QAbstractProxyModel::headerData(section, orientation, role);
    } else {
        if (!sourceModel()) {
            return QVariant();
        }

        return sourceModel()->headerData(section, orientation, role);
    }
}

QVariant TodoProxyModelBase::data(const QModelIndex &index, int role) const
{
    TodoNode *node = m_manager->nodeForIndex(index);

    if (node == 0) {
        return QVariant();
    }

    return node->data(index.column(), role);
}

Qt::ItemFlags TodoProxyModelBase::flags(const QModelIndex &index) const
{
    TodoNode *node = m_manager->nodeForIndex(index);

    if (node == 0) {
        return Qt::NoItemFlags;
    }

    return node->flags(index.column());
}

QModelIndex TodoProxyModelBase::mapToSource(const QModelIndex &proxyIndex) const
{
    TodoNode *node = m_manager->nodeForIndex(proxyIndex);

    if (!node) {
        return QModelIndex();
    } else {
        QModelIndex rowSourceIndex = node->rowSourceIndex();
        return rowSourceIndex.sibling(rowSourceIndex.row(), proxyIndex.column());
    }
}

QList<QModelIndex> TodoProxyModelBase::mapFromSourceAll(const QModelIndex &sourceIndex) const
{
    if (m_mappingType==SimpleMapping) {
        kError() << "Never call mapFromSourceAll() for a SimpleMapping model";
        return QList<QModelIndex>();
    }

    QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(sourceIndex);
    QList<QModelIndex> indexes;

    foreach (TodoNode *node, nodes) {
        indexes << m_manager->indexForNode(node, sourceIndex.column());
    }

    return indexes;
}

QModelIndex TodoProxyModelBase::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (m_mappingType==MultiMapping) {
        kError() << "Never call mapFromSource() for a MultiMapping model";
        return QModelIndex();
    }

    TodoNode *node = m_manager->nodeForSourceIndex(sourceIndex);
    return m_manager->indexForNode(node, sourceIndex.column());
}

void TodoProxyModelBase::setSourceModel(QAbstractItemModel *model)
{
    init();

    if (sourceModel()) {
        connect(sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(onSourceDataChanged(QModelIndex,QModelIndex)));
        connect(sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(onSourceInsertRows(QModelIndex,int,int)));
        connect(sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(onSourceRemoveRows(QModelIndex,int,int)));
        connect(sourceModel(), SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
                this, SLOT(onRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
        connect(sourceModel(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
                this, SLOT(onRowsMoved(QModelIndex,int,int,QModelIndex,int)));
        connect(sourceModel(), SIGNAL(modelReset()),
                this, SLOT(onModelReset()));
    }

    if (model) {
        connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(onSourceDataChanged(QModelIndex,QModelIndex)));
        connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(onSourceInsertRows(QModelIndex,int,int)));
        connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(onSourceRemoveRows(QModelIndex,int,int)));
        connect(model, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
                this, SLOT(onRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
        connect(model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
                this, SLOT(onRowsMoved(QModelIndex,int,int,QModelIndex,int)));
        connect(model, SIGNAL(modelReset()),
                this, SLOT(onModelReset()));
    }

    QAbstractProxyModel::setSourceModel(model);

    onSourceInsertRows(QModelIndex(), 0, sourceModel()->rowCount() - 1);
}

TodoNode *TodoProxyModelBase::addChildNode(const QModelIndex &sourceIndex, TodoNode *parent)
{
    QModelIndex proxyParentIndex = m_manager->indexForNode(parent, 0);
    int row = 0;

    if (parent) {
        row = parent->children().size();
    } else {
        row = m_manager->roots().size();
    }

    beginInsertRows(proxyParentIndex, row, row);

    TodoNode *child = new TodoNode(sourceIndex, parent);
    m_manager->insertNode(child);

    endInsertRows();

    return child;
}

void TodoProxyModelBase::onRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &/*destinationParent*/, int /*destinationRow*/)
{
    onSourceRemoveRows(sourceParent, sourceStart, sourceEnd);
}

void TodoProxyModelBase::onRowsMoved(const QModelIndex &/*parent*/, int start, int end, const QModelIndex &destination, int row)
{
    onSourceInsertRows(destination, row, row + end - start);
}

void TodoProxyModelBase::onModelReset()
{
    beginResetModel();

    foreach(TodoNode* node, m_manager->roots()) {
        m_manager->removeNode(node);
        delete node;
    }

    m_inboxNode = 0;

    endResetModel();

    init();
}
