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

#include "actionlistmodel.h"

#include "todoflatmodel.h"

ActionListModel::ActionListModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

ActionListModel::~ActionListModel()
{
}

void ActionListModel::setSourceFocusIndex(const QModelIndex &sourceIndex)
{
    m_sourceFocusIndex = sourceIndex;
    invalidate();
}

bool ActionListModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &/*sourceParent*/) const
{
    return sourceColumn!=TodoFlatModel::RemoteId && sourceColumn!=TodoFlatModel::ParentRemoteId;
}

bool ActionListModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);

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
