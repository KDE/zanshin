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
class Node
{
public:
    Node(Domain::Task::Ptr task, Node *parent, TaskTreeModel *model,
         Domain::TaskRepository *repository,
         const TaskTreeModel::QueryGenerator &queryGenerator);
    ~Node();

    Qt::ItemFlags flags() const;
    QVariant data(int role) const;
    bool setData(const QVariant &value, int role);

    int row();
    Node *parent() const;
    Node *child(int row) const;
    void insertChild(int row, Node *node);
    int childCount() const;

    Domain::Task::Ptr task() const;
    Domain::Task::Ptr childTask(int row) const;

private:
    Domain::Task::Ptr m_task;
    Node *m_parent;
    TaskTreeModel::TaskList::Ptr m_taskChildren;
    QList<Node*> m_childNode;
    TaskTreeModel *m_model;
    Domain::TaskRepository *m_repository;
};
}

using namespace Presentation;

Node::Node(Domain::Task::Ptr task, Node *parent, TaskTreeModel *model,
           Domain::TaskRepository *repository,
           const TaskTreeModel::QueryGenerator &queryGenerator)
    : m_task(task),
      m_parent(parent),
      m_model(model),
      m_repository(repository)
{
    m_taskChildren = queryGenerator(task);
    for (auto task : m_taskChildren->data()) {
        Node *node = new Node(task, this, model, m_repository, queryGenerator);
        m_childNode.append(node);
    }

    m_taskChildren->addPreInsertHandler([this, model](const Domain::Task::Ptr &, int index) {
        QModelIndex parentIndex = m_parent ? model->createIndex(row(), 0, this) : QModelIndex();
        model->beginInsertRows(parentIndex, index, index);
    });
    m_taskChildren->addPostInsertHandler([this, model, queryGenerator](const Domain::Task::Ptr &task, int index) {
        Node *node = new Node(task, this, model, m_repository, queryGenerator);
        insertChild(index, node);
        model->endInsertRows();
    });
    m_taskChildren->addPreRemoveHandler([this, model](const Domain::Task::Ptr &, int index) {
        QModelIndex parentIndex = m_parent ? model->createIndex(row(), 0, this) : QModelIndex();
        model->beginRemoveRows(parentIndex, index, index);
    });
    m_taskChildren->addPostRemoveHandler([this, model](const Domain::Task::Ptr &, int index) {
        m_childNode.removeAt(index);
        model->endRemoveRows();
    });
    m_taskChildren->addPostReplaceHandler([this, model](const Domain::Task::Ptr &, int idx) {
        QModelIndex parentIndex = m_parent ? model->createIndex(row(), 0, this) : QModelIndex();
        emit model->dataChanged(model->index(idx, 0, parentIndex), model->index(idx, 0, parentIndex));
    });
}

Node::~Node()
{
    qDeleteAll(m_childNode);
}

Qt::ItemFlags Node::flags() const
{
    return Qt::ItemIsSelectable
         | Qt::ItemIsEnabled
         | Qt::ItemIsEditable
         | Qt::ItemIsUserCheckable;
}

QVariant Node::data(int role) const
{
    if (role != Qt::DisplayRole && role != Qt::CheckStateRole) {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
        return m_task->title();
    else
        return m_task->isDone() ? Qt::Checked : Qt::Unchecked;
}

bool Node::setData(const QVariant &value, int role)
{
    if (role != Qt::EditRole && role != Qt::CheckStateRole) {
        return false;
    }

    if (role == Qt::EditRole) {
        m_task->setTitle(value.toString());
    } else {
        m_task->setDone(value.toInt() == Qt::Checked);
    }

    m_repository->save(m_task);
    return true;
}

int Node::row()
{
    return m_parent ? m_parent->m_childNode.indexOf(this) : -1;
}

Node *Node::parent() const
{
    return m_parent;
}

Node *Node::child(int row) const
{
    if (row >= 0 && row < m_childNode.size())
        return m_childNode.value(row);
    else
        return 0;
}

void Node::insertChild(int row, Node *node)
{
    m_childNode.insert(row, node);
}

int Node::childCount() const
{
    if (m_taskChildren)
        return m_taskChildren->data().size();
    else
        return 0;
}

Domain::Task::Ptr Node::task() const
{
    return m_task;
}

Domain::Task::Ptr Node::childTask(int row) const
{
    if (m_taskChildren)
        return m_taskChildren->data().at(row);
    else
        return Domain::Task::Ptr();
}

TaskTreeModel::TaskTreeModel(const QueryGenerator &queryGenerator, Domain::TaskRepository *repository, QObject *parent)
    : QAbstractItemModel(parent),
      m_rootNode(new Node(Domain::Task::Ptr(), 0, this, repository, queryGenerator))
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

    const Node *parentNode = nodeFromIndex(parent);

    if (row < parentNode->childCount()) {
        Node *node = parentNode->child(row);
        return createIndex(row, column, node);
    } else {
        return QModelIndex();
    }
}

QModelIndex TaskTreeModel::parent(const QModelIndex &index) const
{
    Node *node = nodeFromIndex(index);
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

Node *TaskTreeModel::nodeFromIndex(const QModelIndex &index) const
{
    return index.isValid() ? static_cast<Node*>(index.internalPointer()) : m_rootNode;
}

bool TaskTreeModel::isModelIndexValid(const QModelIndex &index) const
{
    bool valid = index.isValid()
        && index.column() == 0
        && index.row() >= 0;

    if (!valid)
        return false;

    const Node *parentNode = nodeFromIndex(index.parent());
    const int count = parentNode->childCount();
    return index.row() < count;
}

