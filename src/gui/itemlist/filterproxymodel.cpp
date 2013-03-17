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

#include "filterproxymodel.h"

#include <QtGui>
#include <QModelIndex>
#include <QVariant>
#include <Akonadi/EntityTreeModel>
#include <QItemSelectionModel>

#include <KDebug>
#include <KCalCore/Incidence>

#include "searchfiltercacheproxy.h"
#include "core/incidenceitem.h"
#include "core/pimitemmodel.h"
#include <core/pimitemfactory.h>

FilterProxyModel::FilterProxyModel(QObject *parent)
:   QSortFilterProxyModel(parent),
    m_cache(new SearchFilterCache(this))
{
    setFilterKeyColumn(0); //search title columns
    setDynamicSortFilter(true);
}

FilterProxyModel::~FilterProxyModel()
{
}

void FilterProxyModel::setFilterString(const QString &string)
{
    kDebug() << string;
    m_filterString = string;
    m_cache->setFulltextSearch(string);
    setFilterRegExp(string);
    invalidateFilter();
}

void FilterProxyModel::setSourceModel(QAbstractItemModel* s)
{
    Q_ASSERT(m_cache);
    m_cache->setSourceModel(s);
    QSortFilterProxyModel::setSourceModel(m_cache);
}

bool FilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex &index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    Q_ASSERT(index0.isValid());
    //Ensures that parent todos in the project view are not filtered, alternatively we could use a krecursivefilterproxy
    if (index0.data(Zanshin::ItemTypeRole).toInt() == Zanshin::ProjectTodo) {
        return true;
    }
    const Akonadi::Item item = index0.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    if (!item.isValid()) {
//         kWarning() << "not an item " << sourceRow;
        return true;
    }

    //generic things
    PimItem::Ptr pimItem(PimItemFactory::getItem(item));
    Q_ASSERT(!pimItem.isNull());

    //search trough title
    //TODO could be replaced by the corresponding nepomuk searches
    //though the nepomuk serach behaves a little different (explicit regex needed, i.e. Bub does not match Bubikon)
    if (m_filterString.isEmpty() || m_cache->isFulltextMatch(item) || pimItem->getTitle().contains(filterRegExp())) {
        return true;
    }
    return false;
}

