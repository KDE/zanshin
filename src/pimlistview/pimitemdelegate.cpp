/* This file is part of Zanshin Todo.

   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>

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


#include "pimitemdelegate.h"

#include <QApplication>
#include <QPainter>
#include <QTreeView>

#include <Akonadi/EntityTreeModel>

#include <kicon.h>

#include <KDebug>

PimItemDelegate::PimItemDelegate(QTreeView *view, QObject* parent)
:   QStyledItemDelegate(parent),
    m_treeView(view)
{

}


void PimItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    //painting code for sections
    if (!index.data(Akonadi::EntityTreeModel::ItemRole).canConvert<Akonadi::Item>()) { //is section
        if( index.column() != 0 ) {
            return QStyledItemDelegate::paint(painter, option, index);
        }

        QStyleOptionViewItemV4 opt = option;
        initStyleOption(&opt, index);

        //Draw Text
        QFont boldFont;
        boldFont.setBold(true);
        boldFont.setPointSize(11);

        opt.font = boldFont;

        return QStyledItemDelegate::paint(painter, opt, index);
    }

    //FIXME I doubt that this is the correct way to control the alignment...
    if (index.column() == 1) { //Date column
        QStyleOptionViewItemV4 opt = option;
        initStyleOption(&opt, index);
        opt.displayAlignment = Qt::AlignCenter;
        return QStyledItemDelegate::paint(painter, opt, index);
    }

    if (index.column() == 2) { //Status colorfield
            QStyledItemDelegate::paint(painter, option, index); //so the background is drawn

            const QVariant variant = index.data(Qt::DisplayRole);
            if (!variant.canConvert<QBrush>()) {
                return QStyledItemDelegate::paint(painter, option, index);
            }
            const QBrush brush = index.data(Qt::DisplayRole).value<QBrush>();
            //kDebug() << brush;
            painter->save();

            painter->setBrush(brush);
            QRect rect(option.rect.topLeft().x() + option.rect.width()/2-10, option.rect.topLeft().y() + option.rect.height()/2 - 5, 20, 10);
            painter->setRenderHint(QPainter::Antialiasing);
            painter->drawRoundedRect(rect, 5, 5);

            painter->restore();

            //TODO probably better to draw first a pixmap, and then let the style draw the pixmap (so highlighting is still active), without calling QSyledItemDelegate::paint at all
            //        QApplication::style()->drawItemPixmap();
            return;
    }
    
    QStyledItemDelegate::paint(painter, option, index);
}

QSize PimItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    //kDebug() << index.column();
    if (index.column() == 2) {
        return QSize(30, 20);
    }
    return QStyledItemDelegate::sizeHint(option, index);
}


