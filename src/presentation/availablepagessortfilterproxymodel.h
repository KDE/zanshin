/*
 * SPDX-FileCopyrightText: 2014 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef PRESENTATION_AVAILABLEPAGESSORTFILTERPROXYMODEL_H
#define PRESENTATION_AVAILABLEPAGESSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace Presentation {

class AvailablePagesSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit AvailablePagesSortFilterProxyModel(QObject *parent = 0);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

}

#endif // PRESENTATION_AVAILABLEPAGESSORTFILTERPROXYMODEL_H
