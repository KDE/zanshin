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

#include <QtGui/QApplication>
#include <QtCore/QEvent>
#include <QtGui/QAbstractItemView>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QMouseEvent>

ActionListComboBox::ActionListComboBox(QWidget *parent)
    : QComboBox(parent), m_autoHidePopupEnabled(false)
{
}

void ActionListComboBox::setAutoHidePopupEnabled(bool autoHidePopupEnabled)
{
    if (autoHidePopupEnabled == m_autoHidePopupEnabled) {
        return;
    }
    if (autoHidePopupEnabled) {
        view()->removeEventFilter(view()->parentWidget());
        view()->viewport()->removeEventFilter(view()->parentWidget());
        view()->viewport()->installEventFilter(this);
    } else {
        view()->viewport()->removeEventFilter(this);
        view()->installEventFilter(view()->parentWidget());
        view()->viewport()->installEventFilter(view()->parentWidget());
    }
}
bool ActionListComboBox::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease && object==view()->viewport()) {
        QMouseEvent *ev = static_cast<QMouseEvent*>(event);
        QModelIndex index = view()->indexAt(ev->pos());
        setCurrentIndex(index.row());
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
        QModelIndex index = model()->index(i, 0);
        if (index.isValid()) {
            const int textWidth = fm.width(index.data().toString());
            if (itemIcon(i).isNull()) {
                width = (qMax(width, textWidth));
            } else {
                width = (qMax(width, textWidth + iconWidth));
            }
        }
    }

    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    QSize tmp(width, 0);
    tmp = style()->sizeFromContents(QStyle::CT_ComboBox, &opt, tmp, this);
    width = tmp.width();

    const int screenWidth = QApplication::desktop()->width();
    if ( width>screenWidth/2 ) {
        width = screenWidth/2;
    }

    if (width>view()->width()) {
        QSize size = view()->parentWidget()->size();
        size.setWidth(width + 10);
        view()->parentWidget()->resize(size);
    }

    const int viewRight = view()->parentWidget()->mapToGlobal(view()->parentWidget()->rect().bottomRight()).x();
    const int dx = screenWidth - viewRight;

    if (dx<0) {
        const int x = view()->parentWidget()->geometry().x() + dx;
        const int y = view()->parentWidget()->geometry().y();
        view()->parentWidget()->move( x, y );
    }
}
