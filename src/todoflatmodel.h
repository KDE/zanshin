/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>
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
    Q_ENUMS(ItemType Column)
    Q_FLAGS(ItemTypes)

public:
    enum ItemType
    {
        StandardTodo = 0,
        ProjectTodo,
        FolderTodo,
        Category
    };

    Q_DECLARE_FLAGS(ItemTypes, ItemType)

    enum Column {
        Summary = 0,
        Categories,
        ParentSummary,
        DueDate,
        RowType,
        RemoteId,
        ParentRemoteId,
        LastColumn = ParentRemoteId
    };

    TodoFlatModel(QObject *parent = 0);
    virtual ~TodoFlatModel();

    virtual QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;
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
    void onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end);

protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    void setSourceModel(QAbstractItemModel *sourceModel);
    Akonadi::ItemModel *itemModel() const;

    ItemType todoType(const QString &remoteId, bool examinateSiblings = true) const;
    bool isAncestorOf(const QString &ancestor, const QString &child);

    QHash<QString, QString> m_parentMap;
    QHash<QString, QStringList> m_childrenMap;
    QHash<Akonadi::Entity::Id, QString> m_remoteIdMap;
    QHash<QString, Akonadi::Entity::Id> m_reverseRemoteIdMap;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TodoFlatModel::ItemTypes)

#endif

