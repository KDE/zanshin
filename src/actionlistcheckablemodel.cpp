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

#include <actionlistcheckablemodel.h>
#include <QStringList>
#include <KDebug>

ActionListCheckableModel::ActionListCheckableModel(QObject *parent)
    : KCheckableProxyModel(parent)
{
}

QVariant ActionListCheckableModel::data(const QModelIndex& id, int role) const
{
    if (role ==  Qt::EditRole) {
        QStringList categories;
        QModelIndexList indexes = selectionModel()->selectedIndexes();
        foreach (const QModelIndex &index, indexes) {
            QString category = index.data(Qt::DisplayRole).toString();
            QStringList path = category.split(" / ");
            categories << path.last();
        }
        return categories.join(", ");
    }

    QVariant var = KCheckableProxyModel::data(id, role);
    return var;
}
