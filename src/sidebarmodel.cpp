/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>
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

#include "sidebarmodel.h"

#include <KDE/KCalCore/Todo>

#include <KDebug>
#include <KIcon>

#include "globaldefs.h"

SideBarModel::SideBarModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

SideBarModel::~SideBarModel()
{
}

bool SideBarModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.data(Zanshin::ItemTypeRole).toInt()==Zanshin::Inbox) {
        return true;

    } else if (right.data(Zanshin::ItemTypeRole).toInt()==Zanshin::Inbox) {
        return false;

    } else {
        return QSortFilterProxyModel::lessThan(left, right);
    }
}

bool SideBarModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &/*sourceParent*/) const
{
    return sourceColumn==0;
}

bool SideBarModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex sourceChild = sourceModel()->index(sourceRow, 0, sourceParent);
    return sourceChild.data(Zanshin::ItemTypeRole).toInt()!=Zanshin::StandardTodo;
}

bool SideBarModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (!sourceModel()) {
        return QAbstractProxyModel::dropMimeData(data, action, row, column, parent);
    }
    return sourceModel()->dropMimeData(data, action, row, column, parent);
}

