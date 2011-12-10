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

#include <KDE/KCalCore/Todo>

#include <KDebug>
#include <KIcon>
#include <KLocale>

#include "globaldefs.h"

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

    Akonadi::Item item = sourceModel()->data(mapToSource(index), Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();

    if (!item.isValid() || !item.hasPayload<KCalCore::Todo::Ptr>()) {
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
        if (index.column()==0 && itemTypeFromItem(item)==Zanshin::StandardTodo) {
            return todoFromItem(item)->isCompleted() ? Qt::Checked : Qt::Unchecked;
        } else {
            return QVariant();
        }
    case Qt::DecorationRole:
        if (index.column()==0 && itemTypeFromItem(item)==Zanshin::ProjectTodo) {
            return KIcon("view-pim-tasks");
        } else {
            return KIdentityProxyModel::data(index, role);
        }
    case Zanshin::UidRole:
        return uidFromItem(item);
    case Zanshin::ParentUidRole:
        return relatedUidFromItem(item);
    case Zanshin::AncestorsUidRole:
        return ancestorsUidFromItem(item);
    case Zanshin::ItemTypeRole:
        return itemTypeFromItem(item);
    case Zanshin::CategoriesRole:
        return categoriesFromItem(item);
    case Zanshin::AncestorsCategoriesRole:
        return ancestorsCategoriesFromItem(item);
    case Zanshin::DataTypeRole:
        switch (index.column()) {
            case 1 :
                return Zanshin::ProjectType;
            case 2 :
                return Zanshin::CategoryType;
            default:
                return Zanshin::StandardType;
        }
    case Zanshin::ChildUidsRole:
        return childUidsFromItem(item);
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

        KCalCore::Todo::Ptr todo = todoFromIndex(child);

        if (!todo) {
            continue;
        }

        QString uid = todo->uid();

        m_indexMap[uid] = child;

        QString relatedUid = todo->relatedTo();

        if (relatedUid.isEmpty()) {
            continue;
        }

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
            KCalCore::Todo::Ptr todo = todoFromIndex(child);

            if (!todo) {
                continue;
            }

            QString uid = todo->uid();

            QString newRelatedUid = todo->relatedTo();
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


    KCalCore::Todo::Ptr todo = todoFromIndex(index);

    if (!todo) {
        return;
    }

    QString uid = todo->uid();

    QString relatedUid = todo->relatedTo();

    QModelIndex parentIndex = m_indexMap[relatedUid];
    Zanshin::ItemType parentType = (Zanshin::ItemType)parentIndex.data(Zanshin::ItemTypeRole).toInt();

    m_parentMap.remove(uid);
    m_childrenMap[relatedUid].removeAll(uid);
    m_indexMap.remove(uid);

    if (parentType==Zanshin::ProjectTodo
     && parentIndex.data(Zanshin::ItemTypeRole).toInt()!=parentType) {
        emit dataChanged(parentIndex, parentIndex);
    }
}

KCalCore::Todo::Ptr TodoMetadataModel::todoFromIndex(const QModelIndex &index) const
{
    Akonadi::Item item = sourceModel()->data(mapToSource(index), Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    return todoFromItem(item);
}

KCalCore::Todo::Ptr TodoMetadataModel::todoFromItem(const Akonadi::Item &item) const
{
    if (!item.isValid() || !item.hasPayload<KCalCore::Todo::Ptr>()) {
        return KCalCore::Todo::Ptr();
    } else {
        return item.payload<KCalCore::Todo::Ptr>();
    }
}

Zanshin::ItemType TodoMetadataModel::itemTypeFromItem(const Akonadi::Item &item) const
{
    KCalCore::Todo::Ptr todo = todoFromItem(item);

    QStringList comments = todo->comments();
    const int childCount = m_childrenMap.contains(todo->uid()) ? m_childrenMap[todo->uid()].count() : 0;
    if (comments.contains("X-Zanshin-Project")
     || childCount>0) {
        return Zanshin::ProjectTodo;
    } else {
        return Zanshin::StandardTodo;
    }
}

QString TodoMetadataModel::uidFromItem(const Akonadi::Item &item) const
{
    KCalCore::Todo::Ptr todo = todoFromItem(item);
    if (todo) {
        return todo->uid();
    } else {
        return QString();
    }
}

QString TodoMetadataModel::relatedUidFromItem(const Akonadi::Item &item) const
{
    KCalCore::Todo::Ptr todo = todoFromItem(item);
    if (todo) {
        return todo->relatedTo();
    } else {
        return QString();
    }
}

QStringList TodoMetadataModel::ancestorsUidFromItem(const Akonadi::Item &item) const
{
    QStringList result;
    KCalCore::Todo::Ptr todo = todoFromItem(item);

    if (todo) {
        QString id = todo->uid();
        while (m_parentMap.contains(id)) {
            const QString parentId = m_parentMap[id];
            Q_ASSERT(!parentId.isEmpty());
            result << parentId;
            id = parentId;
        }
    }

    return result;
}

QStringList TodoMetadataModel::ancestorsCategoriesFromItem(const Akonadi::Item &item) const
{
    QStringList ancestors = ancestorsUidFromItem(item);
    QStringList categories;
    foreach (QString uid, ancestors) {
        if (!m_indexMap.contains(uid)) {
            continue;
        }
        const QModelIndex &index = m_indexMap[uid];
        KCalCore::Todo::Ptr todo = todoFromIndex(index);
        if (todo) {
            categories << todo->categories();
        }
    }
    categories.removeDuplicates();
    return categories;
}

QStringList TodoMetadataModel::categoriesFromItem(const Akonadi::Item &item) const
{
    QStringList categories = ancestorsCategoriesFromItem(item);
    KCalCore::Todo::Ptr todo = todoFromItem(item);
    if (todo) {
        categories << todo->categories();
    }
    categories.removeDuplicates();
    return categories;
}

QStringList TodoMetadataModel::childUidsFromItem(const Akonadi::Item &item) const
{
    KCalCore::Todo::Ptr todo = todoFromItem(item);
    if (todo) {
        return m_childrenMap[todo->uid()];
    } else {
        return QStringList();
    }
}

QModelIndexList TodoMetadataModel::childIndexesFromIndex(const QModelIndex &idx) const
{
    QModelIndexList indexes;
    KCalCore::Todo::Ptr todo = todoFromIndex(idx);
    if (!todo) {
        return indexes;
    }
    QString parent = todo->uid();
    for (int i = 0; i < rowCount(idx.parent()); ++i) {
        QModelIndex child = index(i, idx.column(), idx.parent());
        todo = todoFromIndex(child);
        if (!todo) {
            continue;
        }
        if (m_parentMap[todo->uid()] == parent) {
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

