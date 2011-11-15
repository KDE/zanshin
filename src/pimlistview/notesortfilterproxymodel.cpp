/* This file is part of Zanshin Todo.

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

#include "notesortfilterproxymodel.h"

#include <QtGui>
#include <QModelIndex>
#include <QVariant>
#include <Akonadi/EntityTreeModel>
#include <KMime/KMimeMessage>
#include <QItemSelectionModel>

#include <Nepomuk/Query/Query>
#include <Nepomuk/Types/Class>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/GroupTerm>
#include <Nepomuk/Query/AndTerm>
#include <Nepomuk/Query/LiteralTerm>
#include <Nepomuk/Vocabulary/NIE>
#include <Nepomuk/Vocabulary/NCAL>
#include <aneo.h>

#include "notetakermodel.h"
#include "tagmanager.h"
#include "pimitem.h"


#include <KDebug>
#include <KCalCore/Incidence>
#include <Nepomuk/Vocabulary/NFO>
#include <nepomuk/orterm.h>
#include <nepomuk/comparisonterm.h>

#include "searchfiltercacheproxy.h"
#include <incidenceitem.h>

NoteSortFilterProxyModel::NoteSortFilterProxyModel(QObject *parent)
:   QSortFilterProxyModel(parent),
    m_itemTypes(AbstractPimItem::All),
    m_filterStrategy(NormalFilter),
    m_cache(new SearchFilterCache(this)),
    m_allTopicsSelected(true),
    m_noTopicSet(false)
{
    setFilterKeyColumn(0); //search title columns
}

NoteSortFilterProxyModel::~NoteSortFilterProxyModel()
{
}


void NoteSortFilterProxyModel::setFilterStrategy(NoteSortFilterProxyModel::FilterStrategy filterStrategy)
{
    m_filterStrategy = filterStrategy;
    invalidateFilter();
}


void NoteSortFilterProxyModel::setTopicFilter(const QList <KUrl> &topicFilterList, bool all, bool none)
{
    m_allTopicsSelected = all;
    m_noTopicSet = none;
    if (!all) {
        m_cache->setTopicFilter(topicFilterList, m_noTopicSet);
    }
    invalidateFilter();
}

void NoteSortFilterProxyModel::setFilterDate(const KDateTime& date)
{
    m_filterDate = date;
    invalidateFilter();
}

void NoteSortFilterProxyModel::setItemFilter(AbstractPimItem::ItemTypes itemTypes)
{
    m_itemTypes = itemTypes;
    kDebug() << itemTypes;
    invalidateFilter();
}

void NoteSortFilterProxyModel::setFilterString(const QString &string)
{
    kDebug() << string;
    m_filterString = string;
    m_cache->setFulltextSearch(string);
    setFilterRegExp(string);
    invalidateFilter();
}


bool NoteSortFilterProxyModel::itemIsSelected(const Akonadi::Item &item) const
{
    if (m_allTopicsSelected) {
        return true;
    }
    return m_cache->isTopicMatch(item);
}

void NoteSortFilterProxyModel::setSourceModel(QAbstractItemModel* s)
{
    Q_ASSERT(m_cache);
    m_cache->setSourceModel(s);
    QSortFilterProxyModel::setSourceModel(m_cache);
}


bool NoteSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    /*if (sourceParent.isValid()) { //check only toplevel items if they match, in case subtodos were not added to the topic
        return true;
    }*/

    const QModelIndex &index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    Q_ASSERT(index0.isValid());
    const Akonadi::Item item = index0.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    if (!item.isValid()) {
        return true;
    }
    //kDebug() << item.id();

    //generic things
    if (!(m_itemTypes & AbstractPimItem::itemType(item))) {
        //kDebug() << m_itemTypes << AbstractPimItem::itemType(item);
        if (AbstractPimItem::itemType(item)== AbstractPimItem::Unknown) {
            kWarning() << "not a pim item" << item.url();// << item.payloadData();
        }
        return false;
    }
/*
    if (!itemIsSelected(item)) {
        return false;
    }

    if (m_filterDate.isValid()) {
        if (m_filterDate.date() != item.modificationTime().date()) {
            return false;
        }
    }*/

    QScopedPointer<AbstractPimItem> pimItem(PimItemUtils::getItem(item));
    Q_ASSERT(!pimItem.isNull());
    //TODO add exception for newly created items
    /*switch (m_filterStrategy) {
        case NotCompleteFilter:
            if (pimItem->getStatus() == AbstractPimItem::Complete) {
                return false;
            }
            break;
        case UpcomingFilter:
            if (pimItem->itemType() & AbstractPimItem::Todo) {
                IncidenceItem *incidence = static_cast<IncidenceItem*>(pimItem.data());
                Q_ASSERT(incidence);
                if (pimItem->getStatus() == AbstractPimItem::Complete) {
                    return false;
                }
                if (incidence->hasDueDate()) { //allow only todos with a due date set
                    break;
                }
            } else if (pimItem->itemType() & AbstractPimItem::Event) {
                IncidenceItem *incidence = static_cast<IncidenceItem*>(pimItem.data());
                Q_ASSERT(incidence);
                if (!incidence->hasStartDate() || (incidence->getEventStart() >= KDateTime(QDateTime::currentDateTime().addDays(-1)))) { //allow only events with a date in the future & last 24 hours
                    break;
                }
            }
            return false;
    }*/

    //search trough title
    //TODO could be replaced by the corresponding nepomuk searches
    //though the nepomuk serach behaves a little different (explicit regex needed, i.e. Bub does not match Bubikon)
    if (m_filterString.isEmpty() || m_cache->isFulltextMatch(item) || pimItem->getTitle().contains(filterRegExp())) {
        return true;
    }
    return false;
}

void NoteSortFilterProxyModel::sort(int column, Qt::SortOrder order)
{
    //kDebug() << column << order;
    if (column < 0) {
        m_customSorting = true;
        //We don't care about the column in the custom sortingstrategies
        //and calling sort with -1 wouldn't trigger the sorting
        return QSortFilterProxyModel::sort(0, Qt::DescendingOrder); 
    }
    m_customSorting = false;
    QSortFilterProxyModel::sort(column, order);
}

void NoteSortFilterProxyModel::setSorting(NoteSortFilterProxyModel::SortRoles sortRole)
{
    QSortFilterProxyModel::setSortRole(sortRole);
    invalidate();
}


bool NoteSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    //kDebug() << sortColumn() << sortRole() << m_customSorting;
    if (m_customSorting) {
        //TODO implement priority sorting, which sorts according to relevancy of fulltext search matches
        if (sortRole() == StatusDateSorting || sortRole() == UpcomingSorting || sortRole() == PrioritySorting) {
            //StatusDateSorting: Primary by status, then by date
            //Upcoming:
            //Todos overdue on top (Status Warning)
            //Everyting with a date
            //Todos/Events without date at end TODO: this is not explicitly handled as also a valid date is returned for items without due/event-date. 
            //But since the modification time is never in the future, it works implicitly correct.
            int leftStatus = left.sibling(left.row(), NotetakerModel::Status).data(NotetakerModel::SortRole).value<int>();
            int rightStatus = right.sibling(right.row(), NotetakerModel::Status).data(NotetakerModel::SortRole).value<int>();
            //TODO items with duedate (or duedate within 1 week) have higher priority than todos without due date
            if (leftStatus == rightStatus) {
                return left.sibling(left.row(), NotetakerModel::Date).data(NotetakerModel::SortRole).value<QDateTime>() < right.sibling(right.row(), NotetakerModel::Date).data(NotetakerModel::SortRole).value<QDateTime>();
            }
            return leftStatus > rightStatus;
        }
    }
    switch (sortColumn()) {
        case NotetakerModel::Date: { //sort by last primary date
            return left.data(NotetakerModel::SortRole).value<QDateTime>() < right.data(NotetakerModel::SortRole).value<QDateTime>();
        }
        case NotetakerModel::Status: { //sort by status
            return left.data(NotetakerModel::SortRole).value<int>() < right.data(NotetakerModel::SortRole).value<int>();
        }
        default: {
            if (left.data(Qt::DisplayRole).value<QString>().compare(right.data(Qt::DisplayRole).value<QString>(), Qt::CaseInsensitive) < 0) {
                return true;
            }
            return false;
        }
    }
    return true;
}

