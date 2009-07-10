/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

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

#include "actionlistdelegate.h"

#include <akonadi/item.h>
#include <boost/shared_ptr.hpp>
#include <kcal/todo.h>

#include "actionlistmodel.h"

typedef boost::shared_ptr<KCal::Incidence> IncidencePtr;

ActionListDelegate::ActionListDelegate(QObject *parent)
    : QStyledItemDelegate(parent), m_dragModeCount(0)
{

}

ActionListDelegate::~ActionListDelegate()
{

}

QSize ActionListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    if (m_dragModeCount>0) {
        if (index.column()==0) {
            return QStyledItemDelegate::sizeHint(option, index);
        } else {
            return QSize();
        }
    }

    QSize res = QStyledItemDelegate::sizeHint(option, index);

    if (res.height() < 28) {
        res.setHeight(28);
    }

    if (rowType(index)==TodoFlatModel::FolderTodo || !isInFocus(index)) {
        res.setHeight(32);
    }

    return res;
}

void ActionListDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    if (m_dragModeCount>0) {
        QStyleOptionViewItemV4 opt = option;
        opt.rect.setHeight(sizeHint(option, index).height());
        m_dragModeCount--;
        if (index.column()==0) {
            return QStyledItemDelegate::paint(painter, opt, index);
        } else {
            return;
        }
    }

    TodoFlatModel::ItemType type = rowType(index);

    QStyleOptionViewItemV4 opt = option;

    if (type==TodoFlatModel::FolderTodo || !isInFocus(index)) {
        opt.decorationSize = QSize(1, 1);
        opt.displayAlignment = Qt::AlignHCenter|Qt::AlignBottom;
        opt.font.setItalic(true);
        opt.font.setPointSizeF(opt.font.pointSizeF()*0.75);

    } else if (type!=TodoFlatModel::StandardTodo) {
        opt.decorationSize = QSize(24, 24);
        opt.font.setWeight(QFont::Bold);

    } else if (index.column()==0 && index.parent().isValid()) {
        opt.rect.adjust(40, 0, 0, 0);
    }

    if (isCompleted(index)) {
        opt.font.setStrikeOut(true);
    } else if (isOverdue(index)) {
        opt.palette.setColor(QPalette::Text, QColor(Qt::red));
        opt.palette.setColor(QPalette::HighlightedText, QColor(Qt::red));
    }

    QStyledItemDelegate::paint(painter, opt, index);
}

void ActionListDelegate::updateEditorGeometry(QWidget *editor,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);

    TodoFlatModel::ItemType type = rowType(index);

    if (type==TodoFlatModel::StandardTodo
     && index.column()==0
     && index.parent().isValid()) {
        QRect r = editor->geometry();
        r.adjust(40, 0, 0, 0);
        editor->setGeometry(r);
    }
}

bool ActionListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index)
{
    QStyleOptionViewItemV4 opt = option;

    TodoFlatModel::ItemType type = rowType(index);

    if (type==TodoFlatModel::StandardTodo
     && index.column()==0
     && index.parent().isValid()) {
        opt.rect.adjust(40, 0, 0, 0);
    }

    return QStyledItemDelegate::editorEvent(event, model, opt, index);
}

TodoFlatModel::ItemType ActionListDelegate::rowType(const QModelIndex &index)
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

    return model->isInFocus(index);
}

bool ActionListDelegate::isCompleted(const QModelIndex &index) const
{
    return index.model()->data(index.sibling(index.row(), 0), Qt::CheckStateRole).toInt()==Qt::Checked;
}

bool ActionListDelegate::isOverdue(const QModelIndex &index) const
{
    const ActionListModel *model = qobject_cast<const ActionListModel*>(index.model());

    if (model==0) {
        return false;
    }

    TodoFlatModel::ItemType type = rowType(index);

    if (type==TodoFlatModel::Category) {
        return false;
    }

    Akonadi::Item item = model->itemForIndex(index);
    const IncidencePtr incidence = item.payload<IncidencePtr>();
    KCal::Todo *todo = dynamic_cast<KCal::Todo*>(incidence.get());

    return todo->isOverdue();
}

void ActionListDelegate::setDragModeCount(int count)
{
    m_dragModeCount = count;
}
