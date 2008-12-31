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

#ifndef ZANSHIN_TODOFLATMODEL_H
#define ZANSHIN_TODOFLATMODEL_H

#include <akonadi/entity.h>
#include <QtGui/QSortFilterProxyModel>

namespace Akonadi
{
    class Collection;
    class Item;
    class ItemModel;
}

class TodoFlatModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum Column {
        Summary = 0,
        Categories,
        DueDate,
        FlagImportant,
        RemoteId,
        ParentRemoteId,
        LastColumn = ParentRemoteId
    };

    TodoFlatModel(QObject *parent = 0);
    virtual ~TodoFlatModel();

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    Akonadi::Item itemForIndex (const QModelIndex &index) const;
    QModelIndex indexForItem (const Akonadi::Item &item, const int column) const;

    void setCollection(const Akonadi::Collection &collection);
    Akonadi::Collection collection() const;

signals:
    void collectionChanged(const Akonadi::Collection &collection);

private slots:
    void onSourceInsertRows(const QModelIndex &sourceIndex, int begin, int end);
    void onSourceRemoveRows(const QModelIndex &sourceIndex, int begin, int end);

protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    enum TodoType
    {
        StandardTodo = 0,
        ProjectTodo,
        FolderTodo
    };

    void setSourceModel(QAbstractItemModel *sourceModel);
    Akonadi::ItemModel *itemModel() const;

    TodoType todoType(const QString &remoteId) const;
    bool isAncestorOf(Akonadi::Entity::Id child, Akonadi::Entity::Id parent);

    QHash<Akonadi::Entity::Id, Akonadi::Entity::Id> m_parentMap;
    QHash<QString, QStringList> m_childrenMap;
    QHash<QString, Akonadi::Entity::Id> m_remoteIdReverseMap;
};

#endif

