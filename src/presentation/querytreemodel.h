/* This file is part of Zanshin

   Copyright 2014 Mario Bensi <mbensi@ipsquad.net>
   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#ifndef PRESENTATION_QUERYTREEMODEL_H
#define PRESENTATION_QUERYTREEMODEL_H

#include <functional>

#include <QAbstractItemModel>

#include "domain/queryresult.h"
#include "domain/queryresultprovider.h"
#include "domain/task.h"

namespace Domain
{
    class TaskRepository;
    class TaskQueries;
}

namespace Presentation {

class QueryTreeModelBase;

class QueryTreeNodeBase
{
public:
    QueryTreeNodeBase(QueryTreeNodeBase *parent, QueryTreeModelBase *model);
    virtual ~QueryTreeNodeBase();

    virtual Qt::ItemFlags flags() const = 0;
    virtual QVariant data(int role) const = 0;
    virtual bool setData(const QVariant &value, int role) = 0;

    int row();
    QueryTreeNodeBase *parent() const;
    QueryTreeNodeBase *child(int row) const;
    void insertChild(int row, QueryTreeNodeBase *node);
    void appendChild(QueryTreeNodeBase *node);
    void removeChildAt(int row);
    int childCount() const;

protected:
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex createIndex(int row, int column, void *data) const;
    void beginInsertRows(const QModelIndex &parent, int first, int last);
    void endInsertRows();
    void beginRemoveRows(const QModelIndex &parent, int first, int last);
    void endRemoveRows();
    void emitDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    QueryTreeNodeBase *m_parent;
    QList<QueryTreeNodeBase*> m_childNode;
    QueryTreeModelBase *m_model;
};

class QueryTreeModelBase : public QAbstractItemModel
{
    Q_OBJECT
public:
    ~QueryTreeModelBase();

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

protected:
    explicit QueryTreeModelBase(QueryTreeNodeBase *rootNode,
                                QObject *parent = 0);

private:
    friend class QueryTreeNodeBase;
    QueryTreeNodeBase *nodeFromIndex(const QModelIndex &index) const;
    bool isModelIndexValid(const QModelIndex &index) const;

    QueryTreeNodeBase *m_rootNode;
};

template<typename ItemType>
class QueryTreeNode : public QueryTreeNodeBase
{
public:
    typedef Domain::QueryResult<ItemType> ItemQuery;
    typedef typename ItemQuery::Ptr ItemQueryPtr;

    typedef std::function<ItemQueryPtr(const ItemType &)> QueryGenerator;
    typedef std::function<Qt::ItemFlags(const ItemType &)> FlagsFunction;
    typedef std::function<QVariant(const ItemType &, int)> DataFunction;
    typedef std::function<bool(const ItemType &, const QVariant &, int)> SetDataFunction;

    QueryTreeNode(const ItemType &item, QueryTreeNodeBase *parentNode, QueryTreeModelBase *model,
         const QueryGenerator &queryGenerator,
         const FlagsFunction &flagsFunction,
         const DataFunction &dataFunction,
         const SetDataFunction &setDataFunction)
        : QueryTreeNodeBase(parentNode, model),
          m_item(item),
          m_flagsFunction(flagsFunction),
          m_dataFunction(dataFunction),
          m_setDataFunction(setDataFunction)
    {
        m_children = queryGenerator(m_item);
        for (auto child : m_children->data()) {
            QueryTreeNodeBase *node = new QueryTreeNode<ItemType>(child, this, model, queryGenerator, m_flagsFunction, m_dataFunction, m_setDataFunction);
            appendChild(node);
        }

        m_children->addPreInsertHandler([this](const ItemType &, int index) {
            QModelIndex parentIndex = parent() ? createIndex(row(), 0, this) : QModelIndex();
            beginInsertRows(parentIndex, index, index);
        });
        m_children->addPostInsertHandler([this, model, queryGenerator](const ItemType &item, int index) {
            QueryTreeNodeBase *node = new QueryTreeNode<ItemType>(item, this, model, queryGenerator, m_flagsFunction, m_dataFunction, m_setDataFunction);
            insertChild(index, node);
            endInsertRows();
        });
        m_children->addPreRemoveHandler([this](const ItemType &, int index) {
            QModelIndex parentIndex = parent() ? createIndex(row(), 0, this) : QModelIndex();
            beginRemoveRows(parentIndex, index, index);
        });
        m_children->addPostRemoveHandler([this](const ItemType &, int index) {
            removeChildAt(index);
            endRemoveRows();
        });
        m_children->addPostReplaceHandler([this](const ItemType &, int idx) {
            QModelIndex parentIndex = parent() ? createIndex(row(), 0, this) : QModelIndex();
            emitDataChanged(index(idx, 0, parentIndex), index(idx, 0, parentIndex));
        });
    }

    Qt::ItemFlags flags() const { return m_flagsFunction(m_item); }
    QVariant data(int role) const { return m_dataFunction(m_item, role); }
    bool setData(const QVariant &value, int role) { return m_setDataFunction(m_item, value, role); }

private:
    ItemType m_item;
    ItemQueryPtr m_children;

    FlagsFunction m_flagsFunction;
    DataFunction m_dataFunction;
    SetDataFunction m_setDataFunction;
};

template<typename ItemType>
class QueryTreeModel : public QueryTreeModelBase
{
public:
    typedef typename QueryTreeNode<ItemType>::QueryGenerator QueryGenerator;
    typedef typename QueryTreeNode<ItemType>::FlagsFunction FlagsFunction;
    typedef typename QueryTreeNode<ItemType>::DataFunction DataFunction;
    typedef typename QueryTreeNode<ItemType>::SetDataFunction SetDataFunction;

    explicit QueryTreeModel(const QueryGenerator &queryGenerator,
                            const FlagsFunction &flagsFunction,
                            const DataFunction &dataFunction,
                            const SetDataFunction &setDataFunction,
                            QObject *parent = 0)
        : QueryTreeModelBase(new QueryTreeNode<ItemType>(ItemType(), 0, this, queryGenerator, flagsFunction, dataFunction, setDataFunction), parent)
    {
    }
};

}

#endif // PRESENTATION_QUERYTREEMODEL_H
