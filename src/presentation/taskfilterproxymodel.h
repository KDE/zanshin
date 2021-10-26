/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef PRESENTATION_TASKFILTERPROXYMODEL_H
#define PRESENTATION_TASKFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace Presentation {

class TaskFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum SortType {
        TitleSort = 0,
        DateSort
    };
    Q_ENUM(SortType)

    explicit TaskFilterProxyModel(QObject *parent = nullptr);

    SortType sortType() const;
    void setSortType(SortType type);

    void setSortOrder(Qt::SortOrder order);

    bool showDoneTasks() const;
    void setShowDoneTasks(bool show);

    bool showFutureTasks() const;
    void setShowFutureTasks(bool show);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    SortType m_sortType;
    bool m_showDone;
    bool m_showFuture;
};

}

#endif // PRESENTATION_TASKFILTERPROXYMODEL_H
