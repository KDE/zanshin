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

    if (index.model()->rowCount(index.sibling(index.row(), 0))>0
     && index.row()>0) {
        res.setHeight(res.height()*2);
    }

    if (res.height()<28) {
        res.setHeight(28);
    }

    return res;
}

void ActionListDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt = option;
    opt.decorationSize = QSize(24, 24);

    if (index.model()->rowCount(index.sibling(index.row(), 0))>0) {
        opt.font.setWeight(QFont::Bold);
        opt.decorationAlignment = Qt::AlignBottom;
        opt.displayAlignment = Qt::AlignBottom;
    }

    QStyledItemDelegate::paint(painter, opt, index);
}
