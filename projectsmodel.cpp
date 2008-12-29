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

#include "projectsmodel.h"

#include "todotreemodel.h"
#include "todoflatmodel.h"

ProjectsModel::ProjectsModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

ProjectsModel::~ProjectsModel()
{
}

void ProjectsModel::setSourceModel(TodoTreeModel *sourceModel)
{
    if (treeModel()) {
        disconnect(treeModel());
    }

    Q_ASSERT(sourceModel==0 || qobject_cast<TodoTreeModel*>(sourceModel)!=0);
    QSortFilterProxyModel::setSourceModel(sourceModel);

    connect(treeModel(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(invalidate()));
    connect(treeModel(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            this, SLOT(invalidate()));
}

TodoTreeModel *ProjectsModel::treeModel() const
{
    return qobject_cast<TodoTreeModel*>(sourceModel());
}

bool ProjectsModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &/*sourceParent*/) const
{
    return sourceColumn==TodoFlatModel::Summary;
}

bool ProjectsModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = treeModel()->index(sourceRow, 0, sourceParent);
    return treeModel()->rowCount(index)>0;
}

Qt::ItemFlags ProjectsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = treeModel()->flags(mapToSource(index));

    if (f & Qt::ItemIsUserCheckable) {
        f^= Qt::ItemIsUserCheckable;
    }

    return f;
}

QVariant ProjectsModel::data(const QModelIndex &index, int role) const
{
    if (role==Qt::CheckStateRole) {
        return QVariant();
    }

    return QSortFilterProxyModel::data(index, role);
}

