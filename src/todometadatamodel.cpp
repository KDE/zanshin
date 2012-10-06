/* This file is part of Zanshin Todo.

   Copyright 2008-2011 Mario Bensi <nef@ipsquad.net>

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

#include "todometadatamodel.h"

#include <KDebug>
#include <KIcon>
#include <KLocale>

#include "globaldefs.h"
#include <abstractpimitem.h>
#include <pimitem.h>
#include <incidenceitem.h>

QStringList getParentProjects(const QList<PimItemRelation> &relations)
{
    QStringList parents;
    foreach (const PimItemRelation &rel, relations) {
        if (rel.type == PimItemRelation::Project) {
            foreach(const PimItemTreeNode &node, rel.parentNodes) {
                parents << node.uid;
            }
        }
    }
    return parents;
}


TodoMetadataModel::TodoMetadataModel(QObject *parent)
    : KIdentityProxyModel(parent)
{
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(onSourceInsertRows(QModelIndex,int,int)));
    connect(this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(onSourceRemoveRows(QModelIndex,int,int)));
    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(onSourceDataChanged(QModelIndex,QModelIndex)));
    connect(this, SIGNAL(modelReset()),
            this, SLOT(onModelReset()));

    onSourceInsertRows(QModelIndex(), 0, rowCount()-1);
}

TodoMetadataModel::~TodoMetadataModel()
{
}

Qt::ItemFlags TodoMetadataModel::flags(const QModelIndex &index) const
{
    if (!sourceModel()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = Qt::NoItemFlags;

    if (index.isValid()) {
        flags = sourceModel()->flags(mapToSource(index));
        Zanshin::ItemType type = (Zanshin::ItemType)index.data(Zanshin::ItemTypeRole).toInt();
        if (index.column()==0) {
            Akonadi::Item item = index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
            if (item.isValid() && type==Zanshin::StandardTodo) {
                flags|= Qt::ItemIsUserCheckable;
            }
        } else if (index.column()==4) {
            flags&= ~Qt::ItemIsEditable;
        }

        if (type==Zanshin::Collection) {
            flags&= ~Qt::ItemIsEditable;
            flags&= ~Qt::ItemIsDragEnabled;
        }
    }

    return flags;
}

QVariant TodoMetadataModel::data(const QModelIndex &index, int role) const
{
    if (!sourceModel()) {
        return QVariant();
    }

    const Akonadi::Item &item = sourceModel()->data(mapToSource(index), Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();

    QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
    if (pimitem.isNull()) {
        if (role==Zanshin::ItemTypeRole) {
            Akonadi::Collection collection = sourceModel()->data(mapToSource(index), Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
            if (collection.isValid()) {
                return Zanshin::Collection;
            }
        }
        return KIdentityProxyModel::data(index, role);
    }
    switch (role) {
    case Qt::CheckStateRole:
        if ((pimitem->itemType() == AbstractPimItem::Todo) && index.column()==0 && itemTypeFromItem(item)==Zanshin::StandardTodo) {
            return (pimitem->getStatus() == AbstractPimItem::Complete) ? Qt::Checked : Qt::Unchecked;
        } else {
            return QVariant();
        }
    case Zanshin::UidRole:
        return pimitem->getUid();
    case Zanshin::ParentUidRole:
        return getParentProjects(pimitem->getRelations());
//     case Zanshin::AncestorsUidRole:
//         return ancestorsUidFromItem(item);
    case Zanshin::ItemTypeRole:
        return itemTypeFromItem(item);
    case Zanshin::DataTypeRole:
        switch (index.column()) {
            case 1 :
                return Zanshin::ProjectType;
            case 2 :
                return Zanshin::CategoryType;
            default:
                return Zanshin::StandardType;
        }
    case Zanshin::ChildIndexesRole:
        return QVariant::fromValue(childIndexesFromIndex(index));
    default:
        return KIdentityProxyModel::data(index, role);
    }
    return KIdentityProxyModel::data(index, role);
}

void TodoMetadataModel::onSourceInsertRows(const QModelIndex &parent, int begin, int end)
{
    for (int i = begin; i <= end; i++) {
        QModelIndex child = index(i, 0, parent);
        onSourceInsertRows(child, 0, rowCount(child)-1);

        const Akonadi::Item item = sourceModel()->data(mapToSource(child), Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
        QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
        if (pimitem.isNull()) {
            continue;
        }

        const QString uid = pimitem->getUid();

        m_indexMap[uid] = child;

        const QStringList parents = getParentProjects(pimitem->getRelations());

        if (parents.isEmpty()) {
            continue;
        }
        QString relatedUid = parents.first();

        m_parentMap[uid] = relatedUid;
        m_childrenMap[relatedUid] << uid;


        if (!m_indexMap.contains(relatedUid)) {
            continue;
        }

        // Emit dataChanged to notify that the todo is a project todo
        QModelIndex parentIndex = m_indexMap[relatedUid];
        if (parentIndex.data(Zanshin::ItemTypeRole).toInt()==Zanshin::ProjectTodo
         && m_childrenMap[relatedUid].size() == 1) {
            emit dataChanged(parentIndex, parentIndex);
        }

    }
}

void TodoMetadataModel::onSourceRemoveRows(const QModelIndex &parent, int begin, int end)
{
    for (int i = begin; i <= end; ++i) {
        QModelIndex child = index(i, 0, parent);

        cleanupDataForSourceIndex(child);
    }
}

void TodoMetadataModel::onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end)
{
    for (int row = begin.row(); row <= end.row(); ++row) {
        for (int column = begin.column(); column <= end.column(); ++column) {
            QModelIndex child = index(row, column, begin.parent());

            const Akonadi::Item item = sourceModel()->data(mapToSource(child), Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
            QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
            if (pimitem.isNull()) {
                continue;
            }

            const QString uid = pimitem->getUid();
            const QStringList parents = getParentProjects(pimitem->getRelations());
            QString newRelatedUid;
            if (!parents.isEmpty()) {
                newRelatedUid = parents.first();
            }

            QString oldRelatedUid;
            if (m_parentMap.contains(uid)) {
                oldRelatedUid = m_parentMap[uid];
            }

            if (newRelatedUid!=oldRelatedUid) {
                if (newRelatedUid.isEmpty()) {
                    m_parentMap.remove(uid);
                } else {
                    m_parentMap[uid] = newRelatedUid;
                    m_childrenMap[newRelatedUid] << uid;
                }
                m_childrenMap[oldRelatedUid].removeAll(uid);
            }

            if (child.data(Zanshin::ItemTypeRole).toInt()==Zanshin::ProjectTodo) {
                foreach (const QModelIndex &index, childIndexesFromIndex(child)) {
                    emit dataChanged(index, index);
                }
            }
        }
    }
}

void TodoMetadataModel::cleanupDataForSourceIndex(const QModelIndex &index)
{
    for (int row=0; row<rowCount(index); row++) {
        QModelIndex child = this->index(row, 0, index);
        cleanupDataForSourceIndex(child);
    }

    const Akonadi::Item item = sourceModel()->data(mapToSource(index), Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
    if (pimitem.isNull()) {
        return;
    }

    const QString uid = pimitem->getUid();
    const QStringList relatedUid = getParentProjects(pimitem->getRelations());
    if (relatedUid.isEmpty()) {
        return;
    }

    QModelIndex parentIndex = m_indexMap[relatedUid.first()];
    Zanshin::ItemType parentType = (Zanshin::ItemType)parentIndex.data(Zanshin::ItemTypeRole).toInt();

    m_parentMap.remove(uid);
    m_childrenMap[relatedUid.first()].removeAll(uid);
    m_indexMap.remove(uid);

    if (parentType==Zanshin::ProjectTodo
     && parentIndex.data(Zanshin::ItemTypeRole).toInt()!=parentType) {
        emit dataChanged(parentIndex, parentIndex);
    }
}

Zanshin::ItemType TodoMetadataModel::itemTypeFromItem(const Akonadi::Item &item) const
{
    QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
    if (pimitem.isNull() || (pimitem->itemType() != AbstractPimItem::Todo)) {
        return Zanshin::StandardTodo;
    }

    const QString uid = pimitem->getUid();

    if (static_cast<IncidenceItem*>(pimitem.data())->isProject()) {
        return Zanshin::ProjectTodo;
    } else {
        return Zanshin::StandardTodo;
    }
}

// QStringList TodoMetadataModel::ancestorsUidFromItem(const Akonadi::Item &item) const
// {
//     QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
//     if (pimitem.isNull()) {
//         return QStringList();
//     }
// 
//     QString id = pimitem->getUid();
// 
//     QStringList result;
//     while (m_parentMap.contains(id)) {
//         const QString parentId = m_parentMap[id];
//         Q_ASSERT(!parentId.isEmpty());
//         result << parentId;
//         id = parentId;
//     }
//     return result;
// }

QModelIndexList TodoMetadataModel::childIndexesFromIndex(const QModelIndex &idx) const
{
    QModelIndexList indexes;
    QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(sourceModel()->data(mapToSource(idx), Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>()));
    if (pimitem.isNull()) {
        return indexes;
    }
    QString parent = pimitem->getUid();
    for (int i = 0; i < rowCount(idx.parent()); ++i) {
        QModelIndex child = index(i, idx.column(), idx.parent());
        QScopedPointer<AbstractPimItem> item(PimItemUtils::getItem(sourceModel()->data(mapToSource(child), Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>()));
        if (item.isNull()) {
            continue;
        }
        if (m_parentMap[item->getUid()] == parent) {
            indexes << child;
        }
    }
    return indexes;
}

void TodoMetadataModel::setSourceModel(QAbstractItemModel *model)
{
    KIdentityProxyModel::setSourceModel(model);

    onSourceInsertRows(QModelIndex(), 0, rowCount() - 1);
}

void TodoMetadataModel::onModelReset()
{
    m_parentMap.clear();
    m_childrenMap.clear();
    m_indexMap.clear();
}

