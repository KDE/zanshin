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

#ifndef ZANSHIN_CONTEXTSMODEL_H
#define ZANSHIN_CONTEXTSMODEL_H

#include <QtGui/QSortFilterProxyModel>

class TodoCategoriesModel;

class ContextsModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ContextsModel(QObject *parent = 0);
    virtual ~ContextsModel();

    virtual void setSourceModel(QAbstractItemModel *sourceModel);

protected:
    virtual bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const;
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    TodoCategoriesModel *categoriesModel() const;
};

#endif

