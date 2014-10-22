/* This file is part of Zanshin

   Copyright 2014 Christian Mollekopf <mollekopf@kolabsys.com>
   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#include "datasourcedelegate.h"

#include <QApplication>
#include <QMouseEvent>

#include "presentation/querytreemodel.h"

using namespace Widgets;

const int DELEGATE_HEIGHT = 16;

DataSourceDelegate::DataSourceDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    m_pixmaps[AddToList] = QIcon::fromTheme("list-add").pixmap(DELEGATE_HEIGHT);
    m_pixmaps[RemoveFromList] = QIcon::fromTheme("list-remove").pixmap(DELEGATE_HEIGHT);
    m_pixmaps[Bookmark] = QIcon::fromTheme("bookmarks").pixmap(DELEGATE_HEIGHT);
}

static QRect createButtonRect(const QRect &itemRect, int position)
{
    static const int border = 2;
    const int side = itemRect.height() - (2 * border);
    const int offset = side * (position + 1) + border * (position + 2);
    return itemRect.adjusted(itemRect.width() - (offset + side), border, -offset, -border);
}

static QStyle *currentStyle(const QStyleOptionViewItem &option)
{
    QWidget const *widget = 0;
    if (const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option)) {
        widget = v3->widget;
    }
    QStyle *style = widget ? widget->style() : QApplication::style();
    return style;
}

static QStyleOptionButton createButtonOption(const QStyleOptionViewItemV4 &itemOption, const QPixmap &pixmap, int position)
{
    const QRect itemRect = itemOption.rect;
    const QRect buttonRect = createButtonRect(itemRect, position);

    QStyleOptionButton buttonOption;
    buttonOption.state = QStyle::State_Active | QStyle::State_Enabled;
    buttonOption.icon = pixmap;
    buttonOption.rect = buttonRect;
    buttonOption.iconSize = pixmap.size();
    return buttonOption;
}

static QList<DataSourceDelegate::Action> actionsForSource(const Domain::DataSource::Ptr &source, bool isHovered)
{
    auto actions = QList<DataSourceDelegate::Action>();

    if (source->contentTypes() == Domain::DataSource::NoContent)
        return actions;

    if (source->listStatus() == Domain::DataSource::Unlisted) {
        actions << DataSourceDelegate::AddToList;
    } else {
        actions << DataSourceDelegate::Bookmark;
        if (isHovered)
            actions << DataSourceDelegate::RemoveFromList;
    }

    return actions;
}

static Domain::DataSource::Ptr sourceForIndex(const QModelIndex &index)
{
    const auto data = index.data(Presentation::QueryTreeModel<Domain::DataSource::Ptr>::ObjectRole);
    const auto source = data.value<Domain::DataSource::Ptr>();
    Q_ASSERT(source);
    return source;
}

void DataSourceDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());

    const auto source = sourceForIndex(index);
    const auto isHovered = bool(opt.state & QStyle::State_MouseOver);

    QStyleOptionViewItemV4 option = opt;
    initStyleOption(&option, index);
    QStyledItemDelegate::paint(painter, option, index);

    QStyle *s = currentStyle(opt);

    int position = 0;
    foreach (Action action, actionsForSource(source, isHovered)) {
        QStyleOptionButton buttonOption = createButtonOption(option, m_pixmaps[action], position);
        if (action == Bookmark
         && source->listStatus() != Domain::DataSource::Bookmarked) {
            buttonOption.state &= ~QStyle::State_Enabled;
        }
        s->drawControl(QStyle::CE_PushButton, &buttonOption, painter, 0);
        position++;
    }
}

bool DataSourceDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                     const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);

    int button = -1;

    switch (event->type()) {
    case QEvent::MouseButtonRelease:
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

            for (int position = 0; position < 4; position++) {
                if (createButtonRect(option.rect, position).contains(mouseEvent->pos())) {
                    button = position;
                    break;
                }
            }

            if (mouseEvent->button() != Qt::LeftButton || button < 0) {
                return QStyledItemDelegate::editorEvent(event, model, option, index);
            }
        }
        break;

    default:
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    Q_ASSERT(button >= 0);
    QStyleOptionViewItem opt = option;
    opt.state |= QStyle::State_MouseOver;

    const auto source = sourceForIndex(index);
    auto actions = actionsForSource(source, true);
    if (actions.count() > button) {
        const Action a = actions.at(button);
        emit actionTriggered(source, a);
        return true;
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QSize DataSourceDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    // Make sure we got a constant height suitable for the buttons
    size.setHeight(DELEGATE_HEIGHT + 4);
    return size;
}
