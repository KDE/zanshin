/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

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

#ifndef ZANSHIN_ACTIONLISTMODEL_H
#define ZANSHIN_ACTIONLISTMODEL_H

#include <QtGui/QSortFilterProxyModel>

namespace Akonadi
{
    class Item;
}

class ActionListModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum Mode {
        StandardMode = 0,
        ProjectMode,
        ContextMode,
        NoProjectMode,
        NoContextMode
    };

    ActionListModel(QObject *parent = 0);
    virtual ~ActionListModel();

    Akonadi::Item itemForIndex(const QModelIndex &index) const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void setMode(Mode mode);
    void setSourceFocusIndex(const QModelIndex &sourceIndex);
    QModelIndex sourceFocusIndex() const;

    bool isInFocus(const QModelIndex &index) const;

protected:
    virtual bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const;
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:
    QPersistentModelIndex m_sourceFocusIndex;
    Mode m_mode;
};

#endif

