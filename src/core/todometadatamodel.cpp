/* This file is part of Zanshin Todo.

   Copyright 2008-2011 Mario Bensi <nef@ipsquad.net>

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

#include "todometadatamodel.h"

#include <KDebug>

#include "globaldefs.h"
#include "core/pimitem.h"
#include "core/incidenceitem.h"
#include "pimitemfactory.h"
#include "pimitemrelations.h"

QStringList getParentProjects(const QList<PimItemRelation> &relations)
{
    QStringList parents;
    foreach (const PimItemRelation &rel, relations) {
        if (rel.type == PimItemRelation::Project) {
            foreach(const PimItemTreeNode &node, rel.parentNodes) {
                parents << node.uid;
            }
        }
    }
    return parents;
}


TodoMetadataModel::TodoMetadataModel(QObject *parent)
    : KIdentityProxyModel(parent)
{
}

TodoMetadataModel::~TodoMetadataModel()
{
}

Qt::ItemFlags TodoMetadataModel::flags(const QModelIndex &index) const
{
    if (!sourceModel()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = Qt::NoItemFlags;

    if (index.isValid()) {
        flags = sourceModel()->flags(mapToSource(index));
        Zanshin::ItemType type = (Zanshin::ItemType)index.data(Zanshin::ItemTypeRole).toInt();
        if (index.column()==0) {
            Akonadi::Item item = index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
            if (item.isValid() && type==Zanshin::StandardTodo) {
                flags|= Qt::ItemIsUserCheckable;
            }
        } else if (index.column()==4) {
            flags&= ~Qt::ItemIsEditable;
        }

        if (type==Zanshin::Collection) {
            flags&= ~Qt::ItemIsEditable;
            flags&= ~Qt::ItemIsDragEnabled;
        }
    }

    return flags;
}

QVariant TodoMetadataModel::data(const QModelIndex &index, int role) const
{
    if (!sourceModel()) {
        return QVariant();
    }

    const Akonadi::Item &item = sourceModel()->data(mapToSource(index), Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    if (!item.isValid()) {
        if ((role == Zanshin::ItemTypeRole) && sourceModel()->data(mapToSource(index), Akonadi::EntityTreeModel::CollectionRole).isValid()) {
            return Zanshin::Collection;
        }
        return KIdentityProxyModel::data(index, role);
    }
    PimItem::Ptr pimitem(PimItemFactory::getItem(item));
    Q_ASSERT(pimitem);
    switch (role) {
    case Qt::CheckStateRole:
        if ((pimitem->itemType() == PimItem::Todo) && index.column()==0 && !pimitem.staticCast<IncidenceItem>()->isProject()) {
            return (pimitem->status() == PimItem::Complete) ? Qt::Checked : Qt::Unchecked;
        } else {
            return QVariant();
        }
    case Zanshin::UidRole:
        return pimitem->uid();
    case Zanshin::ParentUidRole:
        return getParentProjects(pimitem->relations());
    case Zanshin::ItemTypeRole:
        if ((pimitem->itemType() == PimItem::Project)) {
            return Zanshin::ProjectTodo;
        }
        return Zanshin::StandardTodo;
    default:
        return KIdentityProxyModel::data(index, role);
    }
    return KIdentityProxyModel::data(index, role);
}

