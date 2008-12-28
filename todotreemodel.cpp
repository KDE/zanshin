/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/

#include "todotreemodel.h"

#include <akonadi/collection.h>
#include <akonadi/item.h>

#include "todoflatmodel.h"

TodoTreeModel::TodoTreeModel(QObject *parent)
    : QAbstractProxyModel(parent)
{
}

TodoTreeModel::~TodoTreeModel()
{

}

QModelIndex TodoTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0
     || row >= rowCount(parent)
     || column >= columnCount(parent)) {
        return QModelIndex();
    }

    Akonadi::Entity::Id parentId = idForIndex(parent);
    Akonadi::Entity::Id id = m_childrenMap[parentId].at(row);

    return createIndex(row, column, (void*)id);
}

QModelIndex TodoTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()
     || !m_parentMap.contains(index.internalId())) {
        return QModelIndex();
    }

    Akonadi::Entity::Id id = idForIndex(index);
    Akonadi::Entity::Id parentId = m_parentMap[id];

    if (parentId==-1) {
        return QModelIndex();
    }

    return indexForId(parentId);
}

int TodoTreeModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_childrenMap[-1].count();
    } else if (parent.column() == 0) { // Only one set of children per row
        Akonadi::Entity::Id id = idForIndex(parent);
        return m_childrenMap[id].count();
    }

    return 0;
}

int TodoTreeModel::columnCount(const QModelIndex &/*parent*/) const
{
    return TodoFlatModel::LastColumn + 1;
}

QVariant TodoTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return flatModel()->headerData(section, orientation, role);
}

QModelIndex TodoTreeModel::mapToSource(const QModelIndex &proxyIndex) const
{
    return flatModel()->indexForItem(Akonadi::Item(proxyIndex.internalId()),
                                     proxyIndex.column());
}

QModelIndex TodoTreeModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    Akonadi::Item item = flatModel()->itemForIndex(sourceIndex);
    return indexForId(item.id(), sourceIndex.column());
}

void TodoTreeModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (flatModel()) {
        disconnect(flatModel());
    }

    Q_ASSERT(sourceModel == 0 || qobject_cast<TodoFlatModel*>(sourceModel) != 0);
    QAbstractProxyModel::setSourceModel(sourceModel);

    onSourceInsertRows(QModelIndex(), 0, flatModel()->rowCount() - 1);

    connect(flatModel(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(onSourceDataChanged(const QModelIndex&, const QModelIndex&)));
    connect(flatModel(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(onSourceInsertRows(const QModelIndex&, int, int)));
    connect(flatModel(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            this, SLOT(onSourceRemoveRows(const QModelIndex&, int, int)));
    connect(flatModel(), SIGNAL(collectionChanged(const Akonadi::Collection&)),
            this, SLOT(onSourceCollectionChanged(const Akonadi::Collection&)));
}

void TodoTreeModel::onSourceInsertRows(const QModelIndex &/*sourceIndex*/, int begin, int end)
{
    for (int i = begin; i <= end; i++) {
        // Retrieve the item from the source model
        Akonadi::Item item = flatModel()->itemForIndex(flatModel()->index(i, 0));
        QString remoteId = flatModel()->data(flatModel()->index(i, TodoFlatModel::RemoteId)).toString();
        m_remoteIdMap[item.id()] = remoteId;
        m_remoteIdReverseMap[remoteId] = item.id();
    }

    for (int i = end; i >= begin; i--) {
        // Retrieve the item from the source model
        Akonadi::Item item = flatModel()->itemForIndex(flatModel()->index(i, 0));
        Akonadi::Entity::Id id = item.id();

        QModelIndex parentIdIndex = flatModel()->index(i, TodoFlatModel::ParentRemoteId);
        QString remoteParentId = flatModel()->data(parentIdIndex).toString();
        Akonadi::Entity::Id parentId = -1;
        if (!remoteParentId.isEmpty()) {
            parentId = m_remoteIdReverseMap[remoteParentId];
        }

        QModelIndex parentIndex = flatModel()->indexForItem(Akonadi::Item(parentId), 0);
        QModelIndex parentParentIdIndex = flatModel()->indexForItem(Akonadi::Item(parentId),
                                                                    TodoFlatModel::ParentRemoteId);

        /*
         * Fill in the tree maps
         */
        int row = m_childrenMap[parentId].count();
        QModelIndex proxyParentIndex;
        if (parentId!=-1) {
            proxyParentIndex = indexForId(parentId);
        }

        beginInsertRows(proxyParentIndex, row, row);
        m_childrenMap[parentId] << id;
        m_parentMap[id] = parentId;
        endInsertRows();
    }
}

void TodoTreeModel::onSourceRemoveRows(const QModelIndex &/*Id*/, int begin, int end)
{
    for (int i = begin; i <= end; ++i) {
        QModelIndex sourceIndex = flatModel()->index(i, 0);
        QModelIndex proxyIndex = mapFromSource(sourceIndex);
        Akonadi::Item item = flatModel()->itemForIndex(sourceIndex);
        QString remoteId = flatModel()->data(flatModel()->index(i, TodoFlatModel::RemoteId)).toString();

        QHash<Akonadi::Entity::Id, QList<Akonadi::Entity::Id> >::iterator it = m_childrenMap.find(item.id());
        if (it != m_childrenMap.end()) {
            Akonadi::Entity::Id idKey = it.key();
            QList<Akonadi::Entity::Id> idList = it.value();
            while (!idList.isEmpty()) {
                beginRemoveRows(proxyIndex.child(0, 0), 0, 0);
                Akonadi::Entity::Id id = idList.takeFirst();
                endRemoveRows();

                beginInsertRows(QModelIndex(), 0, 0);
                QList<Akonadi::Entity::Id> idEmpty;
                m_childrenMap[id] = idEmpty;
                endInsertRows();
            }
            m_childrenMap[idKey] = idList;
        }

        beginRemoveRows(proxyIndex.parent(), proxyIndex.row(), proxyIndex.row());

        m_remoteIdMap.remove(item.id());
        m_remoteIdReverseMap.remove(remoteId);

        Akonadi::Entity::Id parent = m_parentMap[item.id()];
        QList<Akonadi::Entity::Id> idList = m_childrenMap[parent];
        idList.removeOne(item.id());
        m_childrenMap[parent] = idList;

        m_childrenMap.remove(item.id());
        m_parentMap.remove(item.id());

        endRemoveRows();
    }
}

void TodoTreeModel::onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end)
{
    for (int row = begin.row(); row <= end.row(); ++row) {
        QModelIndex sourceIndex = flatModel()->index(row, 0);

        QModelIndex proxyIndex = mapFromSource(sourceIndex);
        emit dataChanged(index(proxyIndex.row(), 0, proxyIndex.parent()),
                         index(proxyIndex.row(), TodoFlatModel::LastColumn, proxyIndex.parent()));

        QModelIndex parentRemoteIdIndex = flatModel()->index(row, TodoFlatModel::ParentRemoteId);
        QString parentRemoteId = flatModel()->data(parentRemoteIdIndex).toString();
        Akonadi::Entity::Id newParentId = -1;
        if (!parentRemoteId.isEmpty()) {
            newParentId = m_remoteIdReverseMap[parentRemoteId];
        }

        QString itemRemoteId = flatModel()->data(sourceIndex).toString();
        Akonadi::Entity::Id itemId = -1;
        if (!itemRemoteId.isEmpty()) {
            itemId = m_remoteIdReverseMap[itemRemoteId];
        }

        Akonadi::Entity::Id oldParentId = -1;
        if (m_parentMap.contains(itemId)) {
            oldParentId = m_parentMap[itemId];
        }

        if (oldParentId != newParentId) {
            int oldRow = m_childrenMap[oldParentId].indexOf(itemId);
            beginRemoveRows(indexForId(oldParentId), oldRow, oldRow);
            m_childrenMap[oldParentId].removeAll(itemId);
            m_parentMap.remove(itemId);
            endRemoveRows();

            int newRow = m_childrenMap[newParentId].size();
            beginInsertRows(indexForId(newParentId), newRow, newRow);
            m_childrenMap[newParentId] << itemId;
            m_parentMap[itemId] = newParentId;
            endInsertRows();
        }
    }
}

void TodoTreeModel::onSourceCollectionChanged(const Akonadi::Collection &collection)
{
    m_childrenMap.clear();
    m_parentMap.clear();
    m_remoteIdMap.clear();
    m_remoteIdReverseMap.clear();

    reset();

    emit collectionChanged(collection);
}

Akonadi::Entity::Id TodoTreeModel::idForIndex(const QModelIndex &index) const
{
    return index.isValid() ? index.internalId() : -1;
}

QModelIndex TodoTreeModel::indexForId(Akonadi::Entity::Id id, int column) const
{
    if (id==-1) {
        return QModelIndex();
    }

    Akonadi::Entity::Id parentId = m_parentMap[id];
    int row = m_childrenMap[parentId].indexOf(id);

    return createIndex(row, column, (void*)id);
}

Akonadi::Item TodoTreeModel::itemForIndex(const QModelIndex &index) const
{
    return flatModel()->itemForIndex(mapToSource(index));
}

QModelIndex TodoTreeModel::indexForItem(const Akonadi::Item &item, const int column) const
{
    return mapFromSource(flatModel()->indexForItem(item, column));
}

void TodoTreeModel::setCollection(const Akonadi::Collection &collection)
{
    flatModel()->setCollection(collection);
}

Akonadi::Collection TodoTreeModel::collection() const
{
    return flatModel()->collection();
}

TodoFlatModel *TodoTreeModel::flatModel() const
{
    return qobject_cast<TodoFlatModel*>(sourceModel());
}
