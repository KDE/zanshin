/* This file is part of Zanshin Todo.

   Copyright 2008 Thomas Thrainer <tom_t@gmx.at>
   Copyright 2009 Kevin Ottens <ervin@kde.org>

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

#include "actionduedatedelegate.h"

#include <akonadi/item.h>
#include <boost/shared_ptr.hpp>
#include <kcal/todo.h>
#include "kdateedit.h"

#include "actionlistmodel.h"

using namespace KPIM;

typedef boost::shared_ptr<KCal::Incidence> IncidencePtr;

ActionDueDateDelegate::ActionDueDateDelegate(QObject *parent)
    : ActionListDelegate(parent)
{
}

ActionDueDateDelegate::~ActionDueDateDelegate()
{
}

QWidget *ActionDueDateDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    KDateEdit *dateEdit = new KDateEdit(parent);

    return dateEdit;
}

void ActionDueDateDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    KDateEdit *dateEdit = static_cast<KDateEdit*>(editor);

    dateEdit->setDate(index.data(Qt::EditRole).toDate());
}

void ActionDueDateDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                         const QModelIndex &index) const
{
    KDateEdit *dateEdit = static_cast<KDateEdit*>(editor);

    model->setData(index, dateEdit->date());
}

void ActionDueDateDelegate::updateEditorGeometry(QWidget *editor,
                                                 const QStyleOptionViewItem &option,
                                                 const QModelIndex &index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}
