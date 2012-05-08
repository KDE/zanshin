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

#include <KDE/Akonadi/ItemModifyJob>
#include <abstractpimitem.h>
#include <pimitem.h>
#include <datestringbuilder.h>
#include <QBrush>
#include <incidenceitem.h>

PimItemModel::PimItemModel(Akonadi::ChangeRecorder *monitor, QObject *parent)
    : Akonadi::EntityTreeModel(monitor, parent)
{
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(onSourceInsertRows(QModelIndex,int,int)));
    connect(this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(onSourceRemoveRows(QModelIndex,int,int)));
    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(onSourceDataChanged(QModelIndex,QModelIndex)));

    onSourceInsertRows(QModelIndex(), 0, rowCount()-1);

    m_itemHeaders << i18n("Summary") << i18n("Project") << i18n("Contexts") << i18n("Date") << i18n("Collection");

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
    return Akonadi::EntityTreeModel::flags(index) | Qt::ItemIsEditable;
}

int PimItemModel::entityColumnCount(HeaderGroup headerGroup) const
{
    if (headerGroup == CollectionTreeHeaders) {
        return 1;
    } else {
        return ColumnCount;
    }
}

QVariant PimItemModel::entityHeaderData(int section, Qt::Orientation orientation, int role, HeaderGroup headerGroup) const
{
    if (orientation == Qt::Vertical) {
        return EntityTreeModel::entityHeaderData(section, orientation, role, headerGroup);
    }

    if (headerGroup == CollectionTreeHeaders) {
        return i18n("Summary");
    } else if (role == Qt::DisplayRole) {
        if (role == Qt::DisplayRole) {
            return m_itemHeaders.value(section);
        }
    }

    return EntityTreeModel::entityHeaderData(section, orientation, role, headerGroup);
}

QVariant PimItemModel::entityData(const Akonadi::Item &item, int column, int role) const
{
    if (!item.isValid()) {
        kWarning() << "invalid item" << column << role;
        return QVariant();
    }
    QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
    if (pimitem.isNull()) {
        return QVariant();
    }
    switch(role) {
        case Qt::DisplayRole: {
            switch (column) {
                case Summary:
                    return pimitem->getTitle();
                case Project:
                    if ((pimitem->itemType() & AbstractPimItem::Todo) && todoFromItem(item)) {
                        return m_summaryMap[todoFromItem(item)->relatedTo()];
                    }
                    return QString();
                case Contexts:
                    if ((pimitem->itemType() & AbstractPimItem::Todo) && todoFromItem(item)) {
                        return todoFromItem(item)->categories().join(", ");
                    }
                    return QString();
                case Date:
                    return DateStringBuilder::getShortDate(pimitem->getPrimaryDate());
                case Collection:
                    return modelIndexForCollection(this, item.parentCollection()).data();
                case Status:
                    switch (pimitem->getStatus()) {
                        case AbstractPimItem::Now:
                            return QBrush(Qt::green);
                        case AbstractPimItem::Later:
                            return QBrush(Qt::yellow);
                        case AbstractPimItem::Complete:
                            return QBrush(Qt::lightGray);
                        case AbstractPimItem::Attention:
                            return QBrush(Qt::red);
                    }
                    kDebug() << "unhandled status" << item.id() << pimitem->getStatus();
                    break;
            }
            break;
        }
        case Qt::EditRole:
            switch (column) {
                case Summary:
                    return pimitem->getTitle();
                case Project:
                    if ((pimitem->itemType() & AbstractPimItem::Todo) && todoFromItem(item)) {
                        return m_summaryMap[todoFromItem(item)->relatedTo()];
                    }
                    break;
                case Contexts:
                    if ((pimitem->itemType() & AbstractPimItem::Todo) && todoFromItem(item)) {
                        return todoFromItem(item)->categories();
                    }
                    break;
                case Date:
                    return pimitem->getPrimaryDate().dateTime();
                case Collection:
                    return modelIndexForCollection(this, item.parentCollection()).data();
                case Status: //TODO status editor?
                    switch (pimitem->getStatus()) {
                        case AbstractPimItem::Now:
                            return QBrush(Qt::green);
                        case AbstractPimItem::Later:
                            return QBrush(Qt::yellow);
                        case AbstractPimItem::Complete:
                            return QBrush(Qt::lightGray);
                        case AbstractPimItem::Attention:
                            return QBrush(Qt::red);
                    }
                    break;
            }
            break;
        case Qt::ToolTipRole: {
            QString d;
            d.append(QString::fromLatin1("Subject: %1\n").arg(pimitem->getTitle()));
            //kDebug() << pimitem->getCreationDate().dateTime() << pimitem->getLastModifiedDate().dateTime();
            d.append(QString::fromLatin1("Created: %1\n").arg(DateStringBuilder::getFullDateTime(pimitem->getCreationDate())));
            d.append(QString::fromLatin1("Modified: %1\n").arg(DateStringBuilder::getFullDateTime(pimitem->getLastModifiedDate())));
            if (pimitem->itemType()&AbstractPimItem::Todo && static_cast<IncidenceItem*>(pimitem.data())->hasDueDate()) {
                d.append(QString::fromLatin1("Due: %1\n").arg(DateStringBuilder::getFullDateTime(pimitem->getPrimaryDate())));
            }
            d.append(QString::fromLatin1("Akonadi: %1\n").arg(item.url().url()));
//             d.append(QString::fromLatin1("Nepomuk Resource: %1\n").arg(PimItemUtils::getResource(item).resourceUri().toString()));
//             d.append(QString::fromLatin1("Nepomuk Thing: %1\n").arg(PimItemUtils::getThing(item).resourceUri().toString()));
            d.append(QString::fromLatin1("Akonadi Collection: %1\n").arg(item.parentCollection().id()));
            return d;
        }
        case Qt::DecorationRole: { 
            if (column==Collection) {
                return modelIndexForCollection(this, item.parentCollection()).data(Qt::DecorationRole);
            }
            //only needed because the calendar doesnt set the display attribute properly, so we cant rely on it
//             if (column==Summary) {
//                 return SmallIcon(pimitem->getIconName());
//             }
//             return EntityTreeModel::entityData(item, column, role);
            return QVariant();
        }
        /*case Qt::BackgroundRole: {
            if (pimitem->itemType() & AbstractPimItem::Todo) {
                IncidenceItem *inc = static_cast<IncidenceItem*>(pimitem);
                if (inc->getTodoStatus() == IncidenceItem::Now) {
                    return QBrush(Qt::green);
                } else if (inc->getTodoStatus() == IncidenceItem::Later) {
                    return QBrush(Qt::yellow);
                } else if (inc->getTodoStatus() == IncidenceItem::Complete) {
                    return QBrush(Qt::lightGray);
                }
            }
            break;
        }*/
        case SortRole: {
            switch( column ) {
                case Summary:
                    return pimitem->getTitle();
                case Date:
                    return pimitem->getPrimaryDate().dateTime();
                case Status: {
                        //kDebug() << "status: " <<inc->getTodoStatus();
                    switch (pimitem->getStatus()) {
                        case IncidenceItem::Attention:
                            return 0;
                        case IncidenceItem::Now:
                            return 1;
                        case IncidenceItem::Later:
                            return 2;
                        case IncidenceItem::Complete:
                            return 3;
                    }
                }
                default:
                    return QVariant();
            }
        }
        case TitleRole:
            return pimitem->getTitle();
        case DateRole:
            return pimitem->getPrimaryDate().dateTime().toString("ddd, hh:mm:ss");
        case ItemTypeRole:
            return pimitem->itemType();
        default:
            return QVariant();
    }

    //kWarning() << "Not a message" << item.id() << item.remoteId() << item.mimeType();

    return Akonadi::EntityTreeModel::entityData(item, column, role);

}

bool PimItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if ((role!=Qt::EditRole && role!=Qt::CheckStateRole) || !index.isValid()) {
        return EntityTreeModel::setData(index, value, role);
    }

    // We use ParentCollectionRole instead of Akonadi::Item::parentCollection() because the
    // information about the rights is not valid on retrieved items.
    Akonadi::Collection collection = data(index, Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
    if (!(collection.rights() & Akonadi::Collection::CanChangeItem)) {
        return false;
    }

    Akonadi::Item item = data(index, ItemRole).value<Akonadi::Item>();

    if (!item.isValid() || !item.hasPayload<KCalCore::Todo::Ptr>()) {
        return EntityTreeModel::setData(index, value, role);
    }

    bool shouldModifyItem = false;

    KCalCore::Todo::Ptr todo = todoFromItem(item);
    if (!todo) {
        kWarning() << "not a todo cannot modify";
        return false;
    }

    switch (index.column()) {
    case Summary:
        if (role==Qt::EditRole) {
            todo->setSummary(value.toString());
            shouldModifyItem = true;
        } else if (role==Qt::CheckStateRole) {
            todo->setCompleted(value.toInt()==Qt::Checked);
            shouldModifyItem = true;
        }
        break;
    case Project:
        todo->setRelatedTo(value.toString());
        shouldModifyItem = true;
        break;
    case Contexts:
        todo->setCategories(value.toStringList());
        shouldModifyItem = true;
        break;
    case Date:
        todo->setDtDue(KDateTime(value.toDate()));
        todo->setHasDueDate(true);
        todo->setAllDay(true);
        shouldModifyItem = true;
        break;
    case Collection:
        break;
    }

    if (shouldModifyItem) {
        Akonadi::ItemModifyJob *itemModifyJob = new Akonadi::ItemModifyJob( item, this );
        connect(itemModifyJob, SIGNAL(result(KJob*)),
                 this, SLOT(updateJobDone(KJob*)));
    }

    return false;
}

void PimItemModel::onSourceInsertRows(const QModelIndex &parent, int begin, int end)
{
    for (int i = begin; i <= end; i++) {
        QModelIndex child = index(i, 0, parent);
        onSourceInsertRows(child, 0, rowCount(child)-1);

        const Akonadi::Item &item = data(child, ItemRole).value<Akonadi::Item>();
        KCalCore::Todo::Ptr todo = todoFromItem(item); //only required for todos because we need the related-to name
        if (todo) {
            m_summaryMap[todo->uid()] = todo->summary();
        }
    }
}

void PimItemModel::onSourceRemoveRows(const QModelIndex &parent, int begin, int end)
{
    for (int i = begin; i <= end; ++i) {
        const QModelIndex &child = index(i, 0, parent);
        KCalCore::Todo::Ptr todo = todoFromIndex(child);
        if (todo) {
            m_summaryMap.remove(todo->uid());;
        }
    }
}

void PimItemModel::onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end)
{
    for (int row = begin.row(); row <= end.row(); ++row) {
        for (int column = begin.column(); column <= end.column(); ++column) {
            KCalCore::Todo::Ptr todo = todoFromIndex( index(row, column, begin.parent()) );
            if (todo) {
                m_summaryMap[todo->uid()] = todo->summary();
            }
        }
    }
}

KCalCore::Todo::Ptr PimItemModel::todoFromIndex(const QModelIndex &index) const
{
    const Akonadi::Item &item = data(index, ItemRole).value<Akonadi::Item>();
    return todoFromItem(item);
}

KCalCore::Todo::Ptr PimItemModel::todoFromItem(const Akonadi::Item &item) const
{
    if (!item.isValid() || !item.hasPayload<KCalCore::Todo::Ptr>()) {
        return KCalCore::Todo::Ptr();
    } else {
        return item.payload<KCalCore::Todo::Ptr>();
    }
}

Qt::DropActions PimItemModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

#include "pimitemmodel.moc"