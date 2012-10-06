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
#include <KIcon>
#include <KLocale>

#include "globaldefs.h"
#include <abstractpimitem.h>
#include <pimitem.h>
#include <incidenceitem.h>

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

    QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
    if (pimitem.isNull()) {
        if (role==Zanshin::ItemTypeRole) {
            Akonadi::Collection collection = sourceModel()->data(mapToSource(index), Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
            if (collection.isValid()) {
                return Zanshin::Collection;
            }
        }
        return KIdentityProxyModel::data(index, role);
    }
    switch (role) {
    case Qt::CheckStateRole:
        if ((pimitem->itemType() == AbstractPimItem::Todo) && index.column()==0 && itemTypeFromItem(item)==Zanshin::StandardTodo) {
            return (pimitem->getStatus() == AbstractPimItem::Complete) ? Qt::Checked : Qt::Unchecked;
        } else {
            return QVariant();
        }
    case Zanshin::UidRole:
        return pimitem->getUid();
    case Zanshin::ParentUidRole:
        return getParentProjects(pimitem->getRelations());
    case Zanshin::ItemTypeRole:
        return itemTypeFromItem(item);
    case Zanshin::DataTypeRole:
        switch (index.column()) {
            case 1 :
                return Zanshin::ProjectType;
            case 2 :
                return Zanshin::CategoryType;
            default:
                return Zanshin::StandardType;
        }
    default:
        return KIdentityProxyModel::data(index, role);
    }
    return KIdentityProxyModel::data(index, role);
}

Zanshin::ItemType TodoMetadataModel::itemTypeFromItem(const Akonadi::Item &item) const
{
    QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
    if (pimitem.isNull() || (pimitem->itemType() != AbstractPimItem::Todo)) {
        return Zanshin::StandardTodo;
    }

    const QString uid = pimitem->getUid();

    if (static_cast<IncidenceItem*>(pimitem.data())->isProject()) {
        return Zanshin::ProjectTodo;
    } else {
        return Zanshin::StandardTodo;
    }
}


void TodoMetadataModel::setSourceModel(QAbstractItemModel *model)
{
    KIdentityProxyModel::setSourceModel(model);
}
