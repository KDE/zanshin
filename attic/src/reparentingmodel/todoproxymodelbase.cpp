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
#include <Akonadi/EntityTreeModel>

#include "todonode.h"
#include "todonodemanager.h"

TodoProxyModelBase::TodoProxyModelBase(MappingType type, QObject *parent)
    : QAbstractProxyModel(parent),
      m_manager(new TodoNodeManager(this, (type==MultiMapping ? true : false))),
      m_mappingType(type)
{
}

TodoProxyModelBase::~TodoProxyModelBase()
{
    delete m_manager;
}

QModelIndex TodoProxyModelBase::index(int row, int column, const QModelIndex &parent) const
{
    return m_manager->index(row, column, parent);
}

QModelIndex TodoProxyModelBase::buddy(const QModelIndex &index) const
{
    return index;
}

QModelIndex TodoProxyModelBase::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    TodoNode *node = m_manager->nodeForIndex(index);
    Q_ASSERT(node);
    if (!node) {
        return QModelIndex();
    }
    TodoNode *parent = node->parent();

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
        return sourceModel()->columnCount(mapToSource(parent));
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
    if (!index.isValid()) {
        kDebug() << "invalid index: " << index << role;
        return QVariant();
    }

    Q_ASSERT(index.model() == this);
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
        kWarning() << "Never call mapFromSource() for a MultiMapping model";
        //This is useful for selecting an item in the list
        //If we just return an empty index we break EntityTreeModel::modelIndexesForItem
        QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(sourceIndex);
        if (nodes.isEmpty()) {
            return QModelIndex();
        }
        return  m_manager->indexForNode(nodes.first(), sourceIndex.column());
    }

    TodoNode *node = m_manager->nodeForSourceIndex(sourceIndex);
    return m_manager->indexForNode(node, sourceIndex.column());
}

void TodoProxyModelBase::setSourceModel(QAbstractItemModel *model)
{
    init();

    if (sourceModel()) {
        disconnect(sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(onSourceDataChanged(QModelIndex,QModelIndex)));
        disconnect(sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(onSourceInsertRows(QModelIndex,int,int)));
        disconnect(sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(onSourceRemoveRows(QModelIndex,int,int)));
        disconnect(sourceModel(), SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
                this, SLOT(onRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
        disconnect(sourceModel(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
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

    resetInternalData();

    endResetModel();

    init();
}

void TodoProxyModelBase::resetInternalData()
{
    foreach(TodoNode* node, m_manager->roots()) {
        m_manager->removeNode(node);
        delete node;
    }
}


static QPair<QList<const QAbstractProxyModel*>, const TodoProxyModelBase*> proxies(const QAbstractItemModel *model)
{
    QList<const QAbstractProxyModel *> proxyChain;
    const QAbstractProxyModel *proxy = qobject_cast<const QAbstractProxyModel *>( model );
    const QAbstractItemModel *_model = model;
    const TodoProxyModelBase *base = 0;
    while ( proxy )
    {
        proxyChain.prepend( proxy );
        _model = proxy->sourceModel();
        base = qobject_cast<const TodoProxyModelBase *>( _model );
        if (base) {
            break;
        }
        proxy = qobject_cast<const QAbstractProxyModel *>( _model );
    }
    Q_ASSERT(base);
    return qMakePair(proxyChain, base);
}

static QModelIndex proxiedIndex( const QModelIndex &idx, QList<const QAbstractProxyModel *> proxyChain )
{
  QListIterator<const QAbstractProxyModel *> it( proxyChain );
  QModelIndex _idx = idx;
  while ( it.hasNext() )
    _idx = it.next()->mapFromSource( _idx );
  return _idx;
}

// We need this beacause the ETM version doesn't work with multiparenting models. modelIndexesForItem uses mapFromSource to traverse the model stack, and since mapFromSource only returns one item, we might end up with the wrong one at some point. In this specific case the selection model above the todoproxymodelbase filtered the other instance, resulting in the item not being found.
// We can't fix this for as long as we use mapFromSource (it would have to return a list of model indexes), therefore the function has been reimplemented here and must be called whenever a TodoProxyModelBase is in the stack.
// Note that this implementation would currently break with several TodoProxyModelBases in a modelstack. 
// Maybe activate through a datachanged signal or alike, such a singla would be properly multiplied and arrive at all indexes?
QModelIndexList TodoProxyModelBase::modelIndexesForItem(const QAbstractItemModel* model, const Akonadi::Item& item)
{
    QPair<QList<const QAbstractProxyModel*>, const TodoProxyModelBase*> pair = proxies(model);
    const QModelIndexList sourceIndexes = Akonadi::EntityTreeModel::modelIndexesForItem(pair.second->sourceModel(), item);
    QModelIndexList indexes;
    foreach (const QModelIndex &idx, sourceIndexes) {
        indexes << pair.second->mapFromSourceAll(idx);
    }
    QModelIndexList proxyList;
    foreach( const QModelIndex &idx, indexes ) {
        const QModelIndex pIdx = proxiedIndex( idx, pair.first );
        if ( pIdx.isValid() )
            proxyList << pIdx;
    }
    return proxyList;
}
