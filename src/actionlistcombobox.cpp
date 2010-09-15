/* This file is part of Zanshin Todo.

   Copyright 2010 Mario Bensi <mbensi@ipsquad.net>

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

#include "actionlistcombobox.h"
#include "combomodel.h"

#include <QtCore/QEvent>
#include <QtGui/QAbstractItemView>

ActionListComboBox::ActionListComboBox(bool isFiltered, QWidget *parent)
    : QComboBox(parent), m_isReleaseEvent(false)
{
    if (isFiltered) {
        view()->viewport()->installEventFilter(this);
    }
}

bool ActionListComboBox::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease && object==view()->viewport()) {
        m_isReleaseEvent = true;
    }
    return QComboBox::eventFilter(object,event);
}

void ActionListComboBox::showPopup()
{
    QComboBox::showPopup();

    int width = 0;
    const int itemCount = count();
    const int iconWidth = iconSize().width() + 4;
    const QFontMetrics &fm = fontMetrics();

    for (int i = 0; i < itemCount; ++i) {
        const int textWidth = fm.width(itemText(i));
        if (itemIcon(i).isNull()) {
            width = (qMax(width, textWidth));
        } else {
            width = (qMax(width, textWidth + iconWidth));
        }
    }

    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    QSize tmp(width, 0);
    tmp = style()->sizeFromContents(QStyle::CT_ComboBox, &opt, tmp, this);
    width = tmp.width();

    if (width>view()->width()) {
        QSize size = view()->parentWidget()->size();
        size.setWidth(width + 10);
        view()->parentWidget()->resize(size);
    }
}

void ActionListComboBox::hidePopup()
{
    if (m_isReleaseEvent) {
        m_isReleaseEvent = false;
        return;
    }
    QComboBox::hidePopup();
}

void ActionListComboBox::showItem(const QModelIndex &index)
{
    if (!index.isValid()) {
        ComboModel *comboModel = static_cast<ComboModel*>(model());
        setEditText(comboModel->selectedItems().join(", "));
    }
}
