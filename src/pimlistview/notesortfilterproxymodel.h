/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) <year>  <name of author>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef NOTESORTFILTERPROXYMODEL_H
#define NOTESORTFILTERPROXYMODEL_H

#include <QDate>
#include <QString>
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <kdatetime.h>
#include "abstractpimitem.h"
#include "notetakermodel.h"

namespace Akonadi {
class ItemSearchJob;
}

/**
 * Filtering of items
 * Selection of new Items in the set selection model
 */
class NoteSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit NoteSortFilterProxyModel(QObject *parent = 0);
    virtual ~NoteSortFilterProxyModel();

    void setFilterDate(const KDateTime& date);

    /**
     * Only the types set here will pass the filter
     */
    void setItemFilter(AbstractPimItem::ItemTypes);
    /**
     * Only the topics set here will pass the filter
     *
     * if @p all is set, all item will pass. @p all overrides other filters
     * if @p none is set, only items with no topic will pass
     */
    void setTopicFilter(const QList <KUrl> &, bool all, bool none);
    

    enum FilterStrategy {
        NormalFilter,
        NotCompleteFilter, ///Only items with status != done will pass
        UpcomingFilter, ///Only incomplete todos with a due date and events will pass
        SearchFilter ///only search matches will pass (empty by default)
    };
    /**
     * Set a special filter strategy
     */
    void setFilterStrategy(FilterStrategy);

    enum SortRoles {
        ColumnSorting = NotetakerModel::UserRole, //Normal sorting by column, sorts by column 0 by default
        StatusDateSorting, //First by status, then by date, used by work view
        UpcomingSorting, //Used by the upcoming view
        PrioritySorting //used by the search view
    };

    void setSorting(SortRoles);

    void setFilterString(const QString &);
    
    virtual void setSourceModel(QAbstractItemModel* sourceModel);

protected:
    /**
     * Sort filter criterias, according to how expensive the operation is
     */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

private:

    QString m_filterString;
    KDateTime m_filterDate;
    AbstractPimItem::ItemTypes m_itemTypes;

    QList <KUrl> m_topicFilter;

    bool m_allTopicsSelected;
    bool m_noTopicSet;

    /**
     * Returns true if the item has one of the selected topics
     */
    bool itemIsSelected(const Akonadi::Item &item) const;
    bool itemIsTagged(const Akonadi::Item &item) const;
    
    bool m_customSorting;
    FilterStrategy m_filterStrategy;
    
    class SearchFilterCache *m_cache;

};

#endif // NOTESORTFILTERPROXYMODEL_H

