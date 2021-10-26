/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
   SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
   */



#ifndef DATASOURCEDELEGATE_H
#define DATASOURCEDELEGATE_H

#include <QStyledItemDelegate>

#include "domain/datasource.h"

namespace Widgets
{

class DataSourceDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DataSourceDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}

#endif

