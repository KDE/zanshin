/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
}

void DataSourceDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());

    const auto isDefault = index.data(Presentation::QueryTreeModelBase::IsDefaultRole).toBool();

    QStyleOptionViewItem option = opt;
    initStyleOption(&option, index);
    option.font.setBold(isDefault);

    QStyledItemDelegate::paint(painter, option, index);
}

QSize DataSourceDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    // Make sure we got a constant height
    size.setHeight(DELEGATE_HEIGHT + 4);
    return size;
}

#include "moc_datasourcedelegate.cpp"
