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
#include <QtCore/QTimer>
#include <QtGui/QAbstractItemView>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QLineEdit>
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

QRect ActionListComboBox::finalizePopupGeometry(const QRect &geometry) const
{
    QRect result = geometry;

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
    width = tmp.width() + 2 * iconWidth;

    const int screenWidth = QApplication::desktop()->width();
    if ( width>screenWidth/2 ) {
        width = screenWidth/2;
    }

    if (width>geometry.width()) {
        QSize size = geometry.size();
        size.setWidth(width + 10);
        result.setSize(size);
    }

    const int dx = screenWidth - result.right();

    if (dx<0) {
        const int x = geometry.x() + dx;
        const int y = geometry.y();
        result.moveTopLeft(QPoint(x, y));
    }

    return result;
}

void ActionListComboBox::showPopup()
{
    QComboBox::showPopup();

    QRect geometry = finalizePopupGeometry(view()->parentWidget()->geometry());
    view()->parentWidget()->setGeometry(geometry);
}

void ActionListComboBox::childEvent(QChildEvent *event)
{
    if (event->polished() && qobject_cast<QLineEdit*>(event->child())) {
        QTimer::singleShot(0, this, SLOT(onLineEditPolished()));
    }
}

void ActionListComboBox::onLineEditPolished()
{
    /*disconnect signal editingFinished and returnPressed in
    actionListComboBox to fix the currentIndex, by default the combobox
    call QAbstractItemModel::match to find the good index and here it can't
    work because we set the lineEdit with the item name without the path and the
    item.data(DisplayRole) return the path + the name of item.*/

    disconnect(lineEdit(), SIGNAL(returnPressed()), this, 0);
    disconnect(lineEdit(), SIGNAL(editingFinished()), this, 0);
}
