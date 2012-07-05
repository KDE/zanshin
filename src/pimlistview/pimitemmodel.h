/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>
   Copyright 2008, 2009 Mario Bensi <nef@ipsquad.net>
   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>

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

#ifndef ZANSHIN_PIMITEMMODEL_H
#define ZANSHIN_PIMITEMMODEL_H

#include <QtGui/QSortFilterProxyModel>

#include <KDE/Akonadi/EntityTreeModel>
#include "globaldefs.h"

class PimItemModel : public Akonadi::EntityTreeModel
{
    Q_OBJECT
    Q_ENUMS(Zanshin::ItemType Roles)

public:
    PimItemModel(Akonadi::ChangeRecorder *monitor, QObject *parent = 0);
    virtual ~PimItemModel();

    enum Column {
        Summary=0,
        Date,
        Collection,
        Status,
        ColumnCount
    };
    
    enum CustomRoles {
        SortRole=Zanshin::UserRole,
        TitleRole,
        DateRole,
        ItemTypeRole,
        UserRole
    };

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual int entityColumnCount(HeaderGroup headerGroup) const;
    virtual QVariant entityHeaderData(int section, Qt::Orientation orientation, int role, HeaderGroup headerGroup) const;
    virtual QVariant entityData(const Akonadi::Item &item, int column, int role) const;
    using EntityTreeModel::entityData;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    virtual Qt::DropActions supportedDropActions() const;

private:
    QStringList m_itemHeaders;
};

#endif

