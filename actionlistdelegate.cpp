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

#include "actionlistdelegate.h"

#include "actionlistmodel.h"

ActionListDelegate::ActionListDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

ActionListDelegate::~ActionListDelegate()
{

}

QSize ActionListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    QSize res = QStyledItemDelegate::sizeHint(option, index);

    if (res.height() < 28) {
        res.setHeight(28);
    }

    if (rowType(index)==TodoFlatModel::FolderTodo || !isInFocus(index)) {
        res.setHeight(1);
    }

    return res;
}

void ActionListDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    TodoFlatModel::ItemType type = rowType(index);

    if (type==TodoFlatModel::FolderTodo || !isInFocus(index)) {
        return;
    }

    QStyleOptionViewItemV4 opt = option;
    opt.decorationSize = QSize(24, 24);

    if (type!=TodoFlatModel::StandardTodo) {
        opt.font.setWeight(QFont::Bold);
    } else if (index.column()==0 && index.parent().isValid()) {
        opt.rect.adjust(40, 0, 0, 0);
    }

    QStyledItemDelegate::paint(painter, opt, index);
}

TodoFlatModel::ItemType ActionListDelegate::rowType(const QModelIndex &index) const
{
    const ActionListModel *model = qobject_cast<const ActionListModel*>(index.model());

    if (model==0) {
        return TodoFlatModel::StandardTodo;
    }

    QModelIndex sourceIndex = model->mapToSource(index);
    QModelIndex rowTypeIndex = sourceIndex.sibling(sourceIndex.row(), TodoFlatModel::RowType);

    QVariant value = model->sourceModel()->data(rowTypeIndex);
    if (!value.isValid()) {
        return TodoFlatModel::StandardTodo;
    }

    return (TodoFlatModel::ItemType)value.toInt();
}

bool ActionListDelegate::isInFocus(const QModelIndex &index) const
{
    const ActionListModel *model = qobject_cast<const ActionListModel*>(index.model());

    if (model==0) {
        return true;
    }

    QModelIndex focusIndex = model->sourceFocusIndex();

    if (!focusIndex.isValid()) {
        return true;
    }

    QModelIndex sourceIndex = model->mapToSource(index);
    sourceIndex = sourceIndex.sibling(sourceIndex.row(), 0);

    while (sourceIndex.isValid()) {
        if (focusIndex==sourceIndex) {
            return true;
        }
        sourceIndex = sourceIndex.parent();
    }

    return false;
}
