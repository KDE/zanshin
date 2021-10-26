/*
 * SPDX-FileCopyrightText: 2014 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "availablepagessortfilterproxymodel.h"

using namespace Presentation;

AvailablePagesSortFilterProxyModel::AvailablePagesSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    sort(0, Qt::AscendingOrder);
}

bool AvailablePagesSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if ( !left.parent().isValid() ) {
        // Toplevel items: no sorting
        return left.row() < right.row();
    }
    return QSortFilterProxyModel::lessThan(left, right);
}
