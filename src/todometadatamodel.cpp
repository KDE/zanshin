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
    : QSortFilterProxyModel(parent)
{
    connect(this, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(onSourceInsertRows(const QModelIndex&, int, int)));
    connect(this, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            this, SLOT(onSourceRemoveRows(const QModelIndex&, int, int)));
    connect(this, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(onSourceDataChanged(const QModelIndex&, const QModelIndex&)));

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

    Qt::ItemFlags flags = sourceModel()->flags(mapToSource(index));

    if (index.isValid()) {
        if (index.column()==0) {
            Akonadi::Item item = sourceModel()->data(mapToSource(index), Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
            if (item.isValid() && itemTypeFromItem(item)==Zanshin::StandardTodo) {
                flags|= Qt::ItemIsUserCheckable;
            }
        } else if (index.column()==4) {
            flags&= ~Qt::ItemIsEditable;
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
        return QSortFilterProxyModel::data(index, role);
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
            return QSortFilterProxyModel::data(index, role);
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
        return QSortFilterProxyModel::data(index, role);
    }
    return QSortFilterProxyModel::data(index, role);
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

        // Emit dataChanged to notify that the todo is a project todo
        QModelIndex parentIndex = m_indexMap[relatedUid];
        if (parentIndex.data(Zanshin::ItemTypeRole).toInt()==Zanshin::ProjectTodo) {
            emit dataChanged(parentIndex, parentIndex);
        }

    }
}

void TodoMetadataModel::onSourceRemoveRows(const QModelIndex &parent, int begin, int end)
{
    for (int i = begin; i <= end; ++i) {
        QModelIndex child = index(i, 0, parent);
        KCalCore::Todo::Ptr todo = todoFromIndex(child);

        if (!todo) {
            continue;
        }

        QString uid = todo->uid();

        QString relatedUid = todo->relatedTo();

        m_parentMap.remove(uid);
        m_childrenMap[relatedUid].removeAll(uid);
        m_indexMap.remove(uid);
    }
}

void TodoMetadataModel::onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end)
{
    for (int row = begin.row(); row <= end.row(); ++row) {
        for (int column = begin.column(); column <= end.column(); ++column) {
            KCalCore::Todo::Ptr todo = todoFromIndex( index(row, column, begin.parent()) );

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
                m_parentMap[uid] = newRelatedUid;
                m_childrenMap[newRelatedUid] << uid;
                m_childrenMap[oldRelatedUid].removeAll(uid);
            }
        }
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
    if (comments.contains("X-Zanshin-Project")
     || m_childrenMap[todo->uid()].count()>0) {
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

QStringList TodoMetadataModel::categoriesFromItem(const Akonadi::Item &item) const
{
    KCalCore::Todo::Ptr todo = todoFromItem(item);
    if (todo) {
        return todo->categories();
    } else {
        return QStringList();
    }
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
