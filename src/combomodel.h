/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>
   Copyright 2008, 2009 Mario Bensi <nef@ipsquad.net>

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

#ifndef ZANSHIN_COMBOMODEL_H
#define ZANSHIN_COMBOMODEL_H

#include <QtGui/QSortFilterProxyModel>

#include "todomodel.h"

class ComboModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    // Add role for completion
    enum Roles {
        LastPathPartRole = TodoModel::UserRole + 1,
        UserRole = TodoModel::UserRole + 2
    };

    ComboModel(bool isCheckable, QObject *parent = 0);
    virtual ~ComboModel();

    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);

    void setSelectedItems(const QStringList &selectedItems);
    QStringList selectedItems() const;

public slots:
    void checkItem(int row);
    void checkItem(const QModelIndex &index);

private:
    QStringList m_selectedItems;
    bool m_isCheckable;
};

#endif

