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

#include "actionlistview.h"

#include <QtGui/QHeaderView>

#include "actionlistdelegate.h"

ActionListView::ActionListView(QWidget *parent)
    : Akonadi::ItemView(parent)
{
    setRootIsDecorated(false);
    setItemDelegate(new ActionListDelegate(this));
    setAnimated(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setIndentation(0);
    setStyleSheet("QTreeView::branch { background: palette(base) }");
}

ActionListView::~ActionListView()
{

}

void ActionListView::setModel(QAbstractItemModel *model)
{
    QByteArray headerState = header()->saveState();

    Akonadi::ItemView::setModel(model);
    expandAll();
    connectModel(model);

    header()->restoreState(headerState);
}

void ActionListView::setRootIndex(const QModelIndex &index)
{
    Akonadi::ItemView::setRootIndex(index);
    expandAll();
}

void ActionListView::expandBranch(const QModelIndex& parent)
{
    QModelIndex index = parent;

    while (index.isValid()) {
        expand(index);
        index = index.parent();
    }
}

void ActionListView::connectModel(QAbstractItemModel *model) const
{
    connect(model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(expandBranch(const QModelIndex&)));
}


