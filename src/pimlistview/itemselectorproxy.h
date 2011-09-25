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


#ifndef ITEMSELECTORPROXY_H
#define ITEMSELECTORPROXY_H

#include <QObject>

#include <Akonadi/Item>

namespace Akonadi {
class EntityTreeView;
}

class QModelIndex;
class QItemSelectionModel;
class QTimer;
class QAbstractItemModel;

/**
 * Class for selecting new items
 * Works only on top of an EntityTreeModel
 */
class ItemSelectorProxy : public QObject
{
    Q_OBJECT
public:
    ItemSelectorProxy(QObject* parent = 0);

    void setView(Akonadi::EntityTreeView *view);

signals:
    //connect to this signal to be notified if an item has been selected
    void itemSelected(const Akonadi::Item &);

public slots:
    /**
     * Tries to select the item immediately,
     * otherwise connects to newRow and modelReset, and retries with every new item
     */
    void selectItem(const Akonadi::Item &);
private slots:
    bool selectNewIndex();
    void timeout();
    //void currentIndexChanged(const QModelIndex &, const QModelIndex &);
private:
    void selectIndex(const QModelIndex &index);
    QItemSelectionModel *m_selectionModel;
    QAbstractItemModel *m_sourceModel;
    Akonadi::Item m_itemToSelect;
    Akonadi::EntityTreeView *m_view;
    QTimer *m_selectionTimer;
};

#endif // ITEMSELECTORPROXY_H
