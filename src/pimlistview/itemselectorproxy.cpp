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


#include "itemselectorproxy.h"
#include "todoproxymodelbase.h"

#include <Akonadi/EntityTreeModel>
#include <QModelIndex>
#include <QModelIndexList>
#include <QTimer>
#include <QItemSelectionModel>
#include <QAbstractItemModel>
#include <KUrl>
#include <Akonadi/EntityTreeView>


ItemSelectorProxy::ItemSelectorProxy(QObject* parent)
:   QObject(parent),
    m_selectionModel(0),
    m_sourceModel(0),
    m_view(0)
{
    m_selectionTimer = new QTimer(this);
    m_selectionTimer->setSingleShot(true);
    connect(m_selectionTimer, SIGNAL(timeout()), this, SLOT(timeout()));

}

void ItemSelectorProxy::setView(Akonadi::EntityTreeView* view)
{
    if (m_view) {
        disconnect(m_view, SIGNAL(currentChanged(const Akonadi::Item &)), this, SLOT(selectItem(const Akonadi::Item &)));
    }
    m_sourceModel = view->model();
    m_selectionModel = view->selectionModel();
    connect(view, SIGNAL(currentChanged(const Akonadi::Item &)), this, SLOT(selectItem(const Akonadi::Item &)));
    m_view = view;
}


void ItemSelectorProxy::selectIndex(const QModelIndex &index)
{
    //TODO this would be needed to sync breadcrumbs with listview, it doesn't really work though if we select items again which have been selected in the listview. In case the item is twice in the list (different topics) theres a chance that we select the other instance than the one that was clicked.
    //m_selectionModel->setCurrentIndex(index, QItemSelectionModel::Rows|QItemSelectionModel::ClearAndSelect); 
    Akonadi::Item item = m_sourceModel->data(index, Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    kDebug() << "selecting new item" << index << item.url().url();
    Q_ASSERT(item.isValid());
    emit itemSelected(item);
}

bool ItemSelectorProxy::selectNewIndex()
{
    if (!m_selectionModel || !m_sourceModel) {
        kWarning() << "no selection model set";
        return false;
    }
    QModelIndexList idx = TodoProxyModelBase::modelIndexesForItem(m_sourceModel, m_itemToSelect);
    if (!idx.isEmpty()) {
        selectIndex(idx.first());
        disconnect(m_sourceModel, 0, this, 0);
        m_selectionTimer->stop();
        return true;
    }
    return false;
}

void ItemSelectorProxy::timeout()
{
    kWarning() << "selection timed out";
    disconnect(m_sourceModel, 0, this, 0);
}

void ItemSelectorProxy::selectItem(const Akonadi::Item &item)
{
    if (!m_sourceModel) {
        kWarning() << "no model set";
        return;
    }
    if (!item.isValid()) {
        kWarning() << "invalid item";
    }
    m_itemToSelect = item;
    if (!selectNewIndex()) {
        m_selectionTimer->start(3000); //timeout timer, in case the item is filtered before, or for some other reason never appears
        connect(m_sourceModel, SIGNAL(rowsInserted(const QModelIndex&,int,int)), this, SLOT(selectNewIndex()));
        connect(m_sourceModel, SIGNAL(modelReset()), this, SLOT(selectNewIndex()));
    }
}
