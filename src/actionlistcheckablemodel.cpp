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
#include <globaldefs.h>
#include <pimitemrelations.h>
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
            QString category = index.data(Zanshin::CategoryPathRole).toString();
            categories << category;
        }
        return categories.join(", ");
    } else if (role==Qt::DisplayRole) {
        QModelIndex sourceChild = sourceModel()->index(id.row(), 0, id.parent());
        QString category = sourceChild.data().toString();
        category = category.mid(category.indexOf(" / ") + 3);
        return category;
    }

    QVariant var = KCheckableProxyModel::data(id, role);
    return var;
}

Qt::ItemFlags ActionListCheckableModel::flags(const QModelIndex &index) const
{
    if (!sourceModel()) {
        return Qt::NoItemFlags;
    }

    Id category = index.data(Zanshin::RelationIdRole).toLongLong();
    Qt::ItemFlags flags = KCheckableProxyModel::flags(index);
    if (m_disabledCategories.contains(category)) {
        flags&= ~Qt::ItemIsEnabled;
        return flags;
    }
    return flags;
}

void ActionListCheckableModel::setDisabledCategories(const IdList categories)
{
    m_disabledCategories = categories;
}

const IdList ActionListCheckableModel::disabledCategories()
{
    return m_disabledCategories;
}
