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

#include "contextsmodel.h"

#include "todocategoriesmodel.h"
#include "todoflatmodel.h"

ContextsModel::ContextsModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

ContextsModel::~ContextsModel()
{
}

void ContextsModel::setSourceModel(TodoCategoriesModel *sourceModel)
{
    Q_ASSERT(sourceModel==0 || qobject_cast<TodoCategoriesModel*>(sourceModel)!=0);
    QSortFilterProxyModel::setSourceModel(sourceModel);
}

TodoCategoriesModel *ContextsModel::categoriesModel() const
{
    return qobject_cast<TodoCategoriesModel*>(sourceModel());
}

bool ContextsModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &/*sourceParent*/) const
{
    return sourceColumn==TodoFlatModel::Summary;
}

bool ContextsModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = categoriesModel()->index(sourceRow,
                                                 TodoFlatModel::RemoteId,
                                                 sourceParent);

    return !categoriesModel()->data(index).isValid();
}

