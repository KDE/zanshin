/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>
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

#include "actionlistview.h"

#include <QtGui/QHeaderView>

#include "actionlistdelegate.h"
#include "actionduedatedelegate.h"

ActionListView::ActionListView(QWidget *parent)
    : Akonadi::ItemView(parent)
{
    setRootIsDecorated(false);
    setItemDelegate(new ActionListDelegate(this));
    setItemDelegateForColumn(2, new ActionDueDateDelegate(this));
    setAnimated(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
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

void ActionListView::startDrag(Qt::DropActions supportedActions)
{
    ActionListDelegate *delegate = qobject_cast<ActionListDelegate*>(itemDelegate());
    if (delegate) {
        delegate->setDragModeCount(selectedIndexes().size());
    }

    Akonadi::ItemView::startDrag(supportedActions);
}

QModelIndex ActionListView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    QModelIndex index = currentIndex();
    QModelIndex newIndex;

    if (!index.isValid()) {
        index = model()->index(0, 0);

        while (index.isValid() && (model()->flags(index) & Qt::ItemIsEnabled) == 0) {
            index = indexBelow(index);
        }

        return index;
    }

    switch (cursorAction) {
    case MoveLeft:
        if (index.column()==0) {
            return index;
        }

        return index.sibling(index.row(), index.column()-1);

    case MoveRight:
        if (index.column()==model()->columnCount(index)-1) {
            return index;
        }

        return index.sibling(index.row(), index.column()+1);

    case MoveUp:
    case MoveDown:
        newIndex = Akonadi::ItemView::moveCursor(cursorAction, modifiers);
        return newIndex.sibling(newIndex.row(), index.column());

    default:
        return Akonadi::ItemView::moveCursor(cursorAction, modifiers);
    }
}
