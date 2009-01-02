/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/

#ifndef ZANSHIN_TODOTREEMODEL_H
#define ZANSHIN_TODOTREEMODEL_H

#include <QtGui/QAbstractProxyModel>

#include <akonadi/entity.h>

namespace Akonadi
{
    class Item;
    class Collection;
}

class TodoFlatModel;

class TodoTreeModel : public QAbstractProxyModel
{
    Q_OBJECT

public:
    TodoTreeModel(QObject *parent = 0);
    virtual ~TodoTreeModel();

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;

    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;

    virtual void setSourceModel(QAbstractItemModel *sourceModel);

    Akonadi::Item itemForIndex (const QModelIndex &index) const;
    QModelIndex indexForItem (const Akonadi::Item &item, const int column = 0) const;

    void setCollection(const Akonadi::Collection &collection);
    Akonadi::Collection collection() const;

signals:
    void collectionChanged(const Akonadi::Collection &collection);

private slots:
    void onSourceInsertRows(const QModelIndex &sourceIndex, int begin, int end);
    void onSourceRemoveRows(const QModelIndex &sourceIndex, int begin, int end);
    void onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end);
    void onSourceCollectionChanged(const Akonadi::Collection &collection);

private:
    TodoFlatModel *flatModel() const;
    Akonadi::Entity::Id idForIndex(const QModelIndex &index) const;
    QModelIndex indexForId(Akonadi::Entity::Id id, int column = 0) const;

    QHash<Akonadi::Entity::Id, QList<Akonadi::Entity::Id> > m_childrenMap;
    QHash<Akonadi::Entity::Id, Akonadi::Entity::Id> m_parentMap;

    QHash<Akonadi::Entity::Id, QString> m_remoteIdMap;
    QHash<QString, Akonadi::Entity::Id> m_remoteIdReverseMap;
};

#endif

