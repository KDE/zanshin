/* This file is part of Zanshin

   Copyright 2014 Mario Bensi <mbensi@ipsquad.net>

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


#include "tasktreemodel.h"

#include "domain/taskrepository.h"
#include "domain/taskqueries.h"

#include <QDebug>


namespace Presentation {
class NodeBase
{
public:
    NodeBase(NodeBase *parent, TaskTreeModel *model);
    virtual ~NodeBase();

    virtual Qt::ItemFlags flags() const = 0;
    virtual QVariant data(int role) const = 0;
    virtual bool setData(const QVariant &value, int role) = 0;

    int row();
    NodeBase *parent() const;
    NodeBase *child(int row) const;
    void insertChild(int row, NodeBase *node);
    void appendChild(NodeBase *node);
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
    NodeBase *m_parent;
    QList<NodeBase*> m_childNode;
    TaskTreeModel *m_model;
};

template<typename ItemType>
class Node : public NodeBase
{
public:
    typedef typename ItemType::Ptr ItemTypePtr;
    typedef Domain::QueryResult<ItemTypePtr> ItemQuery;
    typedef typename ItemQuery::Ptr ItemQueryPtr;

    typedef std::function<ItemQueryPtr(const ItemTypePtr &)> QueryGenerator;
    typedef std::function<Qt::ItemFlags(const ItemTypePtr &)> FlagsFunction;
    typedef std::function<QVariant(const ItemTypePtr &, int)> DataFunction;
    typedef std::function<bool(const ItemTypePtr &, const QVariant &, int)> SetDataFunction;

    Node(const ItemTypePtr &item, NodeBase *parentNode, TaskTreeModel *model,
         const QueryGenerator &queryGenerator,
         const FlagsFunction &flagsFunction,
         const DataFunction &dataFunction,
         const SetDataFunction &setDataFunction)
        : NodeBase(parentNode, model),
          m_item(item),
          m_flagsFunction(flagsFunction),
          m_dataFunction(dataFunction),
          m_setDataFunction(setDataFunction)
    {
        m_children = queryGenerator(m_item);
        for (auto child : m_children->data()) {
            NodeBase *node = new Node<ItemType>(child, this, model, queryGenerator, m_flagsFunction, m_dataFunction, m_setDataFunction);
            appendChild(node);
        }

        m_children->addPreInsertHandler([this](const ItemTypePtr &, int index) {
            QModelIndex parentIndex = parent() ? createIndex(row(), 0, this) : QModelIndex();
            beginInsertRows(parentIndex, index, index);
        });
        m_children->addPostInsertHandler([this, model, queryGenerator](const ItemTypePtr &item, int index) {
            NodeBase *node = new Node<ItemType>(item, this, model, queryGenerator, m_flagsFunction, m_dataFunction, m_setDataFunction);
            insertChild(index, node);
            endInsertRows();
        });
        m_children->addPreRemoveHandler([this](const ItemTypePtr &, int index) {
            QModelIndex parentIndex = parent() ? createIndex(row(), 0, this) : QModelIndex();
            beginRemoveRows(parentIndex, index, index);
        });
        m_children->addPostRemoveHandler([this](const ItemTypePtr &, int index) {
            removeChildAt(index);
            endRemoveRows();
        });
        m_children->addPostReplaceHandler([this](const ItemTypePtr &, int idx) {
            QModelIndex parentIndex = parent() ? createIndex(row(), 0, this) : QModelIndex();
            emitDataChanged(index(idx, 0, parentIndex), index(idx, 0, parentIndex));
        });
    }

    Qt::ItemFlags flags() const { return m_flagsFunction(m_item); }
    QVariant data(int role) const { return m_dataFunction(m_item, role); }
    bool setData(const QVariant &value, int role) { return m_setDataFunction(m_item, value, role); }

private:
    ItemTypePtr m_item;
    ItemQueryPtr m_children;

    FlagsFunction m_flagsFunction;
    DataFunction m_dataFunction;
    SetDataFunction m_setDataFunction;
};
}

using namespace Presentation;

NodeBase::NodeBase(NodeBase *parent, TaskTreeModel *model)
    : m_parent(parent),
      m_model(model)
{
}

NodeBase::~NodeBase()
{
    qDeleteAll(m_childNode);
}

int NodeBase::row()
{
    return m_parent ? m_parent->m_childNode.indexOf(this) : -1;
}

NodeBase *NodeBase::parent() const
{
    return m_parent;
}

NodeBase *NodeBase::child(int row) const
{
    if (row >= 0 && row < m_childNode.size())
        return m_childNode.value(row);
    else
        return 0;
}

void NodeBase::insertChild(int row, NodeBase *node)
{
    m_childNode.insert(row, node);
}

void NodeBase::appendChild(NodeBase *node)
{
    m_childNode.append(node);
}

void NodeBase::removeChildAt(int row)
{
    delete m_childNode.takeAt(row);
}

int NodeBase::childCount() const
{
    return m_childNode.size();
}

QModelIndex NodeBase::index(int row, int column, const QModelIndex &parent) const
{
    return m_model->index(row, column, parent);
}

QModelIndex NodeBase::createIndex(int row, int column, void *data) const
{
    return m_model->createIndex(row, column, data);
}

void NodeBase::beginInsertRows(const QModelIndex &parent, int first, int last)
{
    m_model->beginInsertRows(parent, first, last);
}

void NodeBase::endInsertRows()
{
    m_model->endInsertRows();
}

void NodeBase::beginRemoveRows(const QModelIndex &parent, int first, int last)
{
    m_model->beginRemoveRows(parent, first, last);
}

void NodeBase::endRemoveRows()
{
    m_model->endRemoveRows();
}

void NodeBase::emitDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    emit m_model->dataChanged(topLeft, bottomRight);
}

TaskTreeModel::TaskTreeModel(const QueryGenerator &queryGenerator, const FlagsFunction &flagsFunction, const DataFunction &dataFunction, const SetDataFunction &setDataFunction, QObject *parent)
    : QAbstractItemModel(parent),
      m_rootNode(new Node<Domain::Task>(Domain::Task::Ptr(), 0, this, queryGenerator, flagsFunction, dataFunction, setDataFunction))
{
}

TaskTreeModel::~TaskTreeModel()
{
    delete m_rootNode;
}

Qt::ItemFlags TaskTreeModel::flags(const QModelIndex &index) const
{
    if (!isModelIndexValid(index)) {
        return Qt::NoItemFlags;
    }

    return nodeFromIndex(index)->flags();
}

QModelIndex TaskTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0)
        return QModelIndex();

    const NodeBase *parentNode = nodeFromIndex(parent);

    if (row < parentNode->childCount()) {
        NodeBase *node = parentNode->child(row);
        return createIndex(row, column, node);
    } else {
        return QModelIndex();
    }
}

QModelIndex TaskTreeModel::parent(const QModelIndex &index) const
{
    NodeBase *node = nodeFromIndex(index);
    if (!node->parent() || node->parent() == m_rootNode)
        return QModelIndex();
    else
        return createIndex(node->parent()->row(), 0, node->parent());
}

int TaskTreeModel::rowCount(const QModelIndex &index) const
{
    return nodeFromIndex(index)->childCount();
}

int TaskTreeModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant TaskTreeModel::data(const QModelIndex &index, int role) const
{
    if (!isModelIndexValid(index)) {
        return QVariant();
    }

    return nodeFromIndex(index)->data(role);
}

bool TaskTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!isModelIndexValid(index)) {
        return false;
    }

    return nodeFromIndex(index)->setData(value, role);
}

NodeBase *TaskTreeModel::nodeFromIndex(const QModelIndex &index) const
{
    return index.isValid() ? static_cast<NodeBase*>(index.internalPointer()) : m_rootNode;
}

bool TaskTreeModel::isModelIndexValid(const QModelIndex &index) const
{
    bool valid = index.isValid()
        && index.column() == 0
        && index.row() >= 0;

    if (!valid)
        return false;

    const NodeBase *parentNode = nodeFromIndex(index.parent());
    const int count = parentNode->childCount();
    return index.row() < count;
}

