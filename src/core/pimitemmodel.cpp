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

#include "pimitemmodel.h"

#include <KDE/KCalCore/Todo>

#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <KUrl>
#include <QBrush>
#include <KDE/Akonadi/ItemModifyJob>

#include "core/pimitem.h"
#include "core/incidenceitem.h"
#include "pimitemfactory.h"
#include "utils/datestringbuilder.h"

PimItemModel::PimItemModel(Akonadi::ChangeRecorder *monitor, QObject *parent)
    : Akonadi::EntityTreeModel(monitor, parent)
{
    m_itemHeaders << i18n("Summary") << i18n("Date");

    //For QML
//     QHash<int, QByteArray> roles = EntityTreeModel::roleNames();
//     roles[Summary] = "title";
//     roles[Date] = "date";
//     setRoleNames(roles);
}

PimItemModel::~PimItemModel()
{
}

Qt::ItemFlags PimItemModel::flags(const QModelIndex &index) const
{
    const PimItem::Ptr pimitem(index.data(Zanshin::PimItemRole).value<PimItem::Ptr>());
    const bool isEditable = index.column() == 0 || (pimitem && pimitem->itemType() == PimItemIndex::Todo );
    const Qt::ItemFlags extra = isEditable ? Qt::ItemIsEditable : Qt::NoItemFlags;
    return Akonadi::EntityTreeModel::flags(index) | extra;
}

int PimItemModel::entityColumnCount(HeaderGroup headerGroup) const
{
    if (headerGroup == CollectionTreeHeaders) {
        return 1;
    } else {
        return m_itemHeaders.size();
    }
}

QVariant PimItemModel::entityHeaderData(int section, Qt::Orientation orientation, int role, HeaderGroup headerGroup) const
{
    if (orientation == Qt::Vertical || role != Qt::DisplayRole) {
        return EntityTreeModel::entityHeaderData(section, orientation, role, headerGroup);
    }

    if (headerGroup == CollectionTreeHeaders) {
        return i18n("Summary");
    } else if (section > -1 && section < m_itemHeaders.size()){
        return m_itemHeaders.at(section);
    }

    return QVariant();
}

QVariant PimItemModel::entityData(const Akonadi::Item &item, int column, int role) const
{
    if (!item.isValid()) {
        kWarning() << "invalid item" << column << role;
        return QVariant();
    }
    PimItem::Ptr pimitem(PimItemFactory::getItem(item));
    if (!pimitem || column < 0 || column > 1) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (column == 0)
            return pimitem->title();
        else
            return pimitem->primaryDate().toString();

    case Zanshin::PimItemRole:
        return QVariant::fromValue(pimitem);

    default:
        return QVariant();
    }

    return Akonadi::EntityTreeModel::entityData(item, column, role);
}

bool PimItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if ((role!=Qt::EditRole && role!=Qt::CheckStateRole)) {
        //The ETM makes some checks which don't work with the multiparenting proxies
        if (role == EntityTreeModel::ItemRole && value.canConvert<Akonadi::Item>()) {
            new Akonadi::ItemModifyJob(value.value<Akonadi::Item>());
            return true;
        }
        return EntityTreeModel::setData(index, value, role);
    }

    // We use ParentCollectionRole instead of Akonadi::Item::parentCollection() because the
    // information about the rights is not valid on retrieved items.
    Akonadi::Collection collection = data(index, Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
    if (!(collection.rights() & Akonadi::Collection::CanChangeItem)) {
        return false;
    }

    Akonadi::Item item = data(index, Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    PimItem::Ptr pimitem(PimItemFactory::getItem(item));
    if (pimitem.isNull()) {
        return false;
    }

    if (index.column() < 0 || index.column() > 1)
        return false;

    if (role == Qt::EditRole) {
        if (index.column() == 0)
            pimitem->setTitle(value.toString());
        else if (pimitem->itemType() == PimItemIndex::Todo)
            static_cast<IncidenceItem*>(pimitem.data())->setDueDate(KDateTime(value.toDate()));
    } else if (role==Qt::CheckStateRole && pimitem->itemType() == PimItemIndex::Todo) {
        if (value.toInt() == Qt::Checked) {
            static_cast<IncidenceItem*>(pimitem.data())->setTodoStatus(PimItem::Complete);
        } else {
            static_cast<IncidenceItem*>(pimitem.data())->setTodoStatus(PimItem::NotComplete);
        }
    }

    pimitem->saveItem();
    return true;
}

#include "pimitemmodel.moc"
