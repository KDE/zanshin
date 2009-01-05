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

#ifndef ZANSHIN_TODOCATEGORIESMODEL_H
#define ZANSHIN_TODOCATEGORIESMODEL_H

#include <QtGui/QAbstractProxyModel>

#include <akonadi/collection.h>
#include <akonadi/entity.h>

namespace Akonadi
{
    class Item;
}

class TodoFlatModel;
class TodoCategoryTreeNode;

class TodoCategoriesModel : public QAbstractProxyModel
{
    Q_OBJECT

public:
    TodoCategoriesModel(QObject *parent = 0);
    virtual ~TodoCategoriesModel();

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    QList<QModelIndex> mapFromSourceAll(const QModelIndex &sourceIndex) const;

    virtual void setSourceModel(QAbstractItemModel *sourceModel);

    Akonadi::Item itemForIndex (const QModelIndex &index) const;
    QList<QModelIndex> indexesForItem (const Akonadi::Item &item, int column = 0) const;

    QString categoryForIndex(const QModelIndex &index) const;
    QModelIndex indexForCategory(const QString &category, int column = 0) const;

    void setCollection(const Akonadi::Collection &collection);
    Akonadi::Collection collection() const;

signals:
    void collectionChanged(const Akonadi::Collection &collection);

private slots:
    void onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end);
    void onSourceInsertRows(const QModelIndex &sourceIndex, int begin, int end);
    void onSourceRemoveRows(const QModelIndex &sourceIndex, int begin, int end);
    void onSourceCollectionChanged(const Akonadi::Collection &collection);

private:
    TodoFlatModel *flatModel() const;

    virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;

    void loadDefaultCategories();
    void loadCategory(TodoCategoryTreeNode *node, TodoCategoryTreeNode *parent = 0);

    void serializeCategories();
    void deserializeCategories();

    Akonadi::Collection m_collection;

    TodoCategoryTreeNode *nodeForIndex(const QModelIndex &index) const;
    QModelIndex indexForNode(TodoCategoryTreeNode *node, int column = 0) const;

    QList<TodoCategoryTreeNode*> m_roots;
    QHash<QString, TodoCategoryTreeNode*> m_categoryMap;
    QHash<Akonadi::Entity::Id, QList<TodoCategoryTreeNode*> > m_itemMap;
};

#endif

