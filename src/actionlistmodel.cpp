/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

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

#include "actionlistmodel.h"

#include <akonadi/item.h>

#include <QtCore/QStringList>

#include "todocategoriesmodel.h"
#include "todoflatmodel.h"
#include "todotreemodel.h"

ActionListModel::ActionListModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_mode(StandardMode)
{
    setDynamicSortFilter(true);
    sort(0);
}

ActionListModel::~ActionListModel()
{
}

Akonadi::Item ActionListModel::itemForIndex(const QModelIndex &index) const
{
    QAbstractItemModel *source = sourceModel();

    if (TodoFlatModel *flat = qobject_cast<TodoFlatModel*>(source)) {
        return flat->itemForIndex(mapToSource(index));
    } else if (TodoTreeModel *tree = qobject_cast<TodoTreeModel*>(source)) {
        return tree->itemForIndex(mapToSource(index));
    } else if (TodoCategoriesModel *categories = qobject_cast<TodoCategoriesModel*>(source)) {
        return categories->itemForIndex(mapToSource(index));
    }

    return Akonadi::Item();
}

Qt::ItemFlags ActionListModel::flags(const QModelIndex &index) const
{
    QModelIndex sourceIndex = mapToSource(index);
    QModelIndex rowTypeIndex = sourceIndex.sibling(sourceIndex.row(), TodoFlatModel::RowType);
    int rowType = sourceModel()->data(rowTypeIndex).toInt();

    if (rowType==TodoFlatModel::FolderTodo || !isInFocus(index)) {
        return 0;
    } else {
        return QSortFilterProxyModel::flags(index);
    }
}

QVariant ActionListModel::data(const QModelIndex &index, int role) const
{
    QModelIndex sourceIndex = mapToSource(index);
    QModelIndex columnIndex = sourceIndex.sibling(sourceIndex.row(), index.column());
    if (columnIndex.column()==TodoFlatModel::Categories && role==Qt::DisplayRole) {
        return sourceModel()->data(columnIndex, Qt::EditRole);
    }

    return sourceModel()->data(columnIndex, role);
}

void ActionListModel::setMode(Mode mode)
{
    m_mode = mode;
    invalidate();
}

void ActionListModel::setSourceFocusIndex(const QModelIndex &sourceIndex)
{
    m_sourceFocusIndex = sourceIndex;
    invalidate();
}

QModelIndex ActionListModel::sourceFocusIndex() const
{
    return m_sourceFocusIndex;
}

bool ActionListModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &/*sourceParent*/) const
{
    return sourceColumn!=TodoFlatModel::RemoteId
        && sourceColumn!=TodoFlatModel::ParentRemoteId
        && sourceColumn!=TodoFlatModel::RowType
        && (sourceColumn!=TodoFlatModel::Categories || (m_mode!=ContextMode && m_mode!=NoContextMode))
        && (sourceColumn!=TodoFlatModel::ParentSummary || (m_mode!=ProjectMode && m_mode!=NoProjectMode));
}

bool ActionListModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    switch (m_mode) {
    case StandardMode:
    case ContextMode:
    case ProjectMode:
        sourceIndex = sourceModel()->index(sourceRow, TodoFlatModel::RowType, sourceParent);
        if (!sourceParent.isValid() && sourceModel()->data(sourceIndex).toInt()==TodoFlatModel::StandardTodo) {
            return false;
        }
        break;
    case NoProjectMode:
        sourceIndex = sourceModel()->index(sourceRow, TodoFlatModel::RowType, sourceParent);
        if (sourceModel()->data(sourceIndex).toInt()!=TodoFlatModel::StandardTodo) {
            return false;
        }
        sourceIndex = sourceModel()->index(sourceRow, TodoFlatModel::ParentRemoteId, sourceParent);
        if (!sourceModel()->data(sourceIndex).toString().isEmpty()) {
            return false;
        }
        break;
    case NoContextMode:
        sourceIndex = sourceModel()->index(sourceRow, TodoFlatModel::RowType, sourceParent);
        if (sourceModel()->data(sourceIndex).toInt()!=TodoFlatModel::StandardTodo) {
            return false;
        }
        sourceIndex = sourceModel()->index(sourceRow, TodoFlatModel::Categories, sourceParent);
        if (!sourceModel()->data(sourceIndex).toStringList().isEmpty()) {
            return false;
        }
        break;
    }

    sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    QModelIndex i = sourceIndex;
    while (i.isValid()) {
        if (m_sourceFocusIndex == i) {
            return true;
        }
        i = i.parent();
    }

    i = m_sourceFocusIndex;
    while (i.isValid()) {
        if (sourceIndex == i) {
            return true;
        }
        i = i.parent();
    }

    return !m_sourceFocusIndex.isValid();
}

bool ActionListModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.column()==0 && right.column()==0) {
        return sourceModel()->rowCount(right)!=0 && sourceModel()->rowCount(left)==0;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

bool ActionListModel::isInFocus(const QModelIndex &index) const
{
    if (!m_sourceFocusIndex.isValid()) {
        return true;
    }

    QModelIndex sourceIndex = mapToSource(index);
    sourceIndex = sourceIndex.sibling(sourceIndex.row(), 0);

    while (sourceIndex.isValid()) {
        if (m_sourceFocusIndex==sourceIndex) {
            return true;
        }
        sourceIndex = sourceIndex.parent();
    }

    return false;
}
