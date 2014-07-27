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
class Node : public QObject
{
public:
    typedef QSharedPointer<Node> Ptr;
    typedef Domain::QueryResult<Domain::Task::Ptr> TaskList;

    Node(Domain::Task::Ptr task, Node *parent, Domain::TaskQueries *queries, TaskTreeModel *model);
    ~Node();

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
    TaskList::Ptr m_taskChildren;
    QList<Node*> m_childNode;
    TaskTreeModel *m_model;
};
}

using namespace Presentation;

Node::Node(Domain::Task::Ptr task, Node *parent, Domain::TaskQueries *queries, TaskTreeModel *model)
    : m_task(task),
      m_parent(parent),
      m_model(model)
{
    if (queries) {
        m_taskChildren = queries->findChildren(task);
        for (auto task : m_taskChildren->data()) {
            Node *node = new Node(task, this, queries, model);
            m_childNode.append(node);
        }

        m_taskChildren->addPreInsertHandler([this, queries, model](const Domain::Task::Ptr &task, int index) {
                                        Node *node = new Node(task, this, queries, model);
                                        insertChild(index, node);
                                        model->beginInsertRows(model->createIndex(row(), 0, this), index, index);
                                    });
        m_taskChildren->addPostInsertHandler([this, model](const Domain::Task::Ptr &, int) {
                                        model->endInsertRows();
                                    });
        m_taskChildren->addPreRemoveHandler([this, model](const Domain::Task::Ptr &, int index) {
                                        model->beginRemoveRows(model->createIndex(row(), 0, this), index, index);
                                    });
        m_taskChildren->addPostRemoveHandler([this, model](const Domain::Task::Ptr &, int index) {
                                         m_childNode.removeAt(index);
                                         model->endRemoveRows();
                                    });
        m_taskChildren->addPostReplaceHandler([this, model](const Domain::Task::Ptr &, int idx) {
                                         QModelIndex parentIndex = model->createIndex(row(), 0, this);
                                         emit model->dataChanged(model->index(idx, 0, parentIndex), model->index(idx, 0, parentIndex));
                                    });
    }
}

Node::~Node()
{
    qDeleteAll(m_childNode);
}

int Node::row()
{
    if (m_parent)
        return m_parent->m_childNode.indexOf(this);
    else
        return m_model->m_rootNodes.indexOf(this);
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

TaskTreeModel::TaskTreeModel(const std::function<TaskList::Ptr()> &rootQuery, Domain::TaskQueries *queries, Domain::TaskRepository *repository, QObject *parent)
    : QAbstractItemModel(parent),
      m_repository(repository),
      m_queries(queries)
{
    m_taskList = rootQuery();

    for (auto task : m_taskList->data()) {
        Node *node = new Node(task, 0, m_queries, this);
        m_rootNodes.append(node);
    }

    m_taskList->addPreInsertHandler([this](const Domain::Task::Ptr &task, int index) {
                                        Node *node = new Node(task, 0, m_queries, this);
                                        m_rootNodes.insert(index, node);
                                        beginInsertRows(QModelIndex(), index, index);
                                    });
    m_taskList->addPostInsertHandler([this](const Domain::Task::Ptr &, int) {
                                        endInsertRows();
                                    });
    m_taskList->addPreRemoveHandler([this](const Domain::Task::Ptr &, int index) {
                                        beginRemoveRows(QModelIndex(), index, index);
                                    });
    m_taskList->addPostRemoveHandler([this](const Domain::Task::Ptr &, int index) {
                                        Node *node = m_rootNodes.at(index);
                                        m_rootNodes.removeAt(index);
                                        delete node;
                                        endRemoveRows();
                                    });
    m_taskList->addPostReplaceHandler([this](const Domain::Task::Ptr &, int idx) {
                                        emit dataChanged(index(idx, 0), index(idx, 0));
                                    });
}

TaskTreeModel::~TaskTreeModel()
{
    qDeleteAll(m_rootNodes);
}

Qt::ItemFlags TaskTreeModel::flags(const QModelIndex &index) const
{
    if (!isModelIndexValid(index)) {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
}

QModelIndex TaskTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0)
        return QModelIndex();

    Node *node = 0;
    if (!parent.isValid()) {
        if (row < m_taskList->data().size()
         && !m_taskList->data().isEmpty()
         && row < m_rootNodes.size()) {
            node = m_rootNodes.value(row);
        }
    } else {
        Node *parentNode = static_cast<Node*>(parent.internalPointer());
        if (row < parentNode->childCount())
            node = parentNode->child(row);
    }

    if (node)
        return createIndex(row, column, node);
    else
        return QModelIndex();
}

QModelIndex TaskTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    Node *node = static_cast<Node*>(index.internalPointer());
    if (!node->parent())
        return QModelIndex();
    else
        return createIndex(node->parent()->row(), 0, node->parent());
}

int TaskTreeModel::rowCount(const QModelIndex &index) const
{
    if (!index.isValid())
        return m_taskList->data().size();

    if (index.parent().isValid()) {
        Node *parentNode = static_cast<Node*>(index.parent().internalPointer());
        if (parentNode) {
            Node *node = parentNode->child(index.row());
            if (node)
                return node->childCount();
        }
    }

    if (index.isValid()) {
        Node *node = static_cast<Node*>(index.internalPointer());
        if (node)
            return node->childCount();
    }

    return 0;
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

    if (role != Qt::DisplayRole && role != Qt::CheckStateRole) {
        return QVariant();
    }

    const auto task = taskForIndex(index);
    if (role == Qt::DisplayRole)
        return task->title();
    else
        return task->isDone() ? Qt::Checked : Qt::Unchecked;
}

bool TaskTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!isModelIndexValid(index)) {
        return false;
    }

    if (role != Qt::EditRole && role != Qt::CheckStateRole) {
        return false;
    }

    auto task = taskForIndex(index);
    if (role == Qt::EditRole) {
        task->setTitle(value.toString());
    } else {
        task->setDone(value.toInt() == Qt::Checked);
    }

    m_repository->save(task);
    return true;
}

Domain::Task::Ptr TaskTreeModel::taskForIndex(const QModelIndex &index) const
{
    if (index.parent().isValid()) {
        Node *parent = static_cast<Node*>(index.parent().internalPointer());
        return parent->childTask(index.row());
    } else
        return m_taskList->data().at(index.row());
}

bool TaskTreeModel::isModelIndexValid(const QModelIndex &index) const
{
    bool valid = index.isValid()
        && index.column() == 0
        && index.row() >= 0;

    if (!valid)
        return false;

    int count = index.parent().isValid() ? static_cast<Node*>(index.parent().internalPointer())->childCount()
                                         : m_taskList->data().size();
    return index.row() < count;
}

