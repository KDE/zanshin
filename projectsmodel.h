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

#ifndef ZANSHIN_PROJECTSMODEL_H
#define ZANSHIN_PROJECTSMODEL_H

#include <QtGui/QSortFilterProxyModel>

class TodoTreeModel;

class ProjectsModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ProjectsModel(QObject *parent = 0);
    virtual ~ProjectsModel();

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual void setSourceModel(TodoTreeModel *sourceModel);

protected:
    virtual bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const;
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    TodoTreeModel *treeModel() const;
};

#endif

