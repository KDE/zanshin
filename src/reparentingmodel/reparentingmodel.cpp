/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Christian Mollekopf <chrigi_1@fastmail.fm>

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


#include "reparentingmodel.h"
#include "todonode.h"
#include "globaldefs.h"
#include "todonodemanager.h"
#include <KLocalizedString>
#include <KIcon>
#include <QMimeData>

ReparentingModel::ReparentingModel(ReparentingStrategy* strategy, QObject* parent)
:   TodoProxyModelBase(TodoProxyModelBase::MultiMapping, parent),
    m_strategy(strategy)
{
    m_strategy->setModel(this);
}

ReparentingModel::~ReparentingModel()
{

}

void ReparentingModel::init()
{
    m_strategy->init();
}

TodoNode* ReparentingModel::createInbox() const
{
    return 0;
}

TodoNode *ReparentingModel::createNode(const Id &identifier, const IdList &parents, const QString &name, const QModelIndex &sourceIndex)
{
//     kDebug() << "add node" << name << identifier << parents << sourceIndex;

    if (m_parentMap.contains(identifier)) { //We already have this node, we only need to update it
        TodoNode *node = reparentNode(identifier, parents, sourceIndex);
        if (!sourceIndex.isValid() && !name.isEmpty() && node) {
            node->setData(name, 0, Qt::DisplayRole);
        }
        return node;
    }

    Id parentIdentifier = -1;
    if (!parents.isEmpty()) {
        parentIdentifier = parents.first();
    }
    TodoNode* parentNode = 0;
    if (parentIdentifier >= 0) {
        if (!m_parentMap.contains(parentIdentifier)) {
//             kDebug() << "creating missing parent: " << parentIdentifier;
            createNode(parentIdentifier, IdList(), "unknown");
        }
        Q_ASSERT(m_parentMap.contains(parentIdentifier));
        parentNode = m_parentMap.value(parentIdentifier);
    }

    int row = 0;
    if (parentNode) {
        row = parentNode->children().size();
    } else {
        row = m_manager->roots().size();
    }
    beginInsertRows(m_manager->indexForNode(parentNode, 0), row, row);
    TodoNode *node;
    if (sourceIndex.isValid()) {
        node = new TodoNode(sourceIndex, parentNode);
        //Don't set anything on real nodes
    } else { //For virtual nodes
        node = new TodoNode(parentNode);
        node->setData(name, 0, Qt::DisplayRole);
        node->setData(name, 0, Qt::EditRole);
        m_strategy->setData(node, identifier);
    }

    Q_ASSERT(node);
    m_parentMap[identifier] = node;
    m_manager->insertNode(node);
    endInsertRows();
    Q_ASSERT(!m_parentMap.values().contains(0));
    return node;
}

void ReparentingModel::removeNode(TodoNode *root, bool removeChildren)
{
//     kDebug() << "removeNode " << m_parentMap.key(root);

    if (removeChildren) {
        foreach(TodoNode *childNode, root->children()) {
            Q_ASSERT(m_parentMap.values().contains(childNode));
            Id childId = m_parentMap.key(childNode);
            if (m_strategy->reparentOnParentRemoval(childId)) {
//                 kDebug() << "child " << childId;
                IdList parents = m_strategy->getParents(childNode->rowSourceIndex(), IdList() << m_parentMap.key(root));//Don't try to re-add it to the current parent (which is not yet removed)
                reparentNode(childId, parents, childNode->rowSourceIndex());
            } else {
                removeNode(childNode, true);
            }
        }
    } else {
        foreach(TodoNode *childNode, root->children()) {
            childNode->setParent(0);
        }
    }

    QModelIndex proxyParentIndex = m_manager->indexForNode(root->parent(), 0);
    int row = 0;

    if (root->parent()) {
        row = root->parent()->children().indexOf(root);
    } else {
        row = m_manager->roots().indexOf(root);
    }

    beginRemoveRows(proxyParentIndex, row, row);
    Q_ASSERT(m_parentMap.values().contains(root));
    Id id = m_parentMap.key(root);
    m_strategy->onNodeRemoval(id);
    m_parentMap.remove(id);
    m_manager->removeNode(root);
    delete root;
    endRemoveRows();
}

void ReparentingModel::removeNodeById(Id id)
{
    Q_ASSERT(m_parentMap.contains(id));
    removeNode(m_parentMap.value(id), true);
}


TodoNode *ReparentingModel::reparentNode(const Id& p, const IdList& parents, const QModelIndex &sourceIndex)
{
    if (p < 0) {
        kWarning() << "invalid item";
        return 0;
    }
//     kDebug() << p << parents << sourceIndex;
    //TODO handle multiple parents
    Q_ASSERT(m_parentMap.contains(p));
    TodoNode *node = m_parentMap.value(p);
    Q_ASSERT(node);

    //We want to reparent to same parent to update sourceIndex
    if (node->rowSourceIndex() == sourceIndex) {
        //Reparent only if parent has changed
        if ((parents.isEmpty() && !node->parent()) || (!parents.isEmpty() && (node->parent() == m_parentMap.value(parents.first())))) {
//             kDebug() << "nothing changed";
            return 0;
        }
    }

    //TODO move instead of copy
    //Store what we need to copy to the next item
    const QString &name = node->data(0, Qt::DisplayRole).toString();
    QList<TodoNode*> children = node->children();
    QModelIndex index = sourceIndex;
    if (!sourceIndex.isValid() && node->rowSourceIndex().isValid()) {
        index = node->rowSourceIndex();
    }

    //remove node from any current parent
    removeNode(node, false);

    TodoNode *newNode = createNode(p, parents, name, index);
    Q_ASSERT(newNode);
    foreach (TodoNode* child, children) {
        child->setParent(newNode);
    }
    Q_ASSERT(!m_parentMap.values().contains(0));
    return newNode;
}

void ReparentingModel::renameNode(const Id& identifier, const QString& name)
{
//     kDebug() << "renamed " << identifier << " to " << name;
    Q_ASSERT(m_parentMap.contains(identifier));
    TodoNode *node = m_parentMap.value(identifier);
    node->setData(name, 0, Qt::DisplayRole);
    node->setData(name, 0, Qt::EditRole);
    const QModelIndex &begin = m_manager->indexForNode(node, 0);
    const QModelIndex &end = m_manager->indexForNode(node, 0);
    emit dataChanged(begin, end);
}

void ReparentingModel::onSourceInsertRows(const QModelIndex& sourceIndex, int begin, int end)
{
//     kDebug() << sourceIndex << begin << end;
    for (int i = begin; i <= end; i++) {
        const QModelIndex &sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);
        
        if (!sourceChildIndex.isValid()) {
            kWarning() << "invalid sourceIndex";
            continue;
        }

        Id id = m_strategy->getId(sourceChildIndex);
        bool replace = false;
        if (id > 0) {
            IdList parents = m_strategy->getParents(sourceChildIndex);
            replace = m_parentMap.contains(id);
            createNode(id, parents, QString(), sourceChildIndex);
        }

        //Insert children too
        if (!replace && sourceModel()->hasChildren(sourceChildIndex)) {
//             kDebug() << id << " has children";
            onSourceInsertRows(sourceChildIndex, 0, sourceModel()->rowCount(sourceChildIndex)-1);
        }
    }

    Q_ASSERT(!m_parentMap.values().contains(0));
}

void ReparentingModel::onSourceDataChanged(const QModelIndex& begin, const QModelIndex& end)
{
//     kDebug() << begin << end;
    for (int row = begin.row(); row <= end.row(); row++) {
        const QModelIndex &index = sourceModel()->index(row, 0, begin.parent());
        Id id = m_strategy->getId(index);
        if (id < 0) {
            continue;
        }
        const IdList &parents = m_strategy->getParents(index);
//         kDebug() << index.data(Qt::DisplayRole).toString() << id << parents;

        if (m_parentMap.contains(id)) { //In case we get updates for indexes which haven't been inserted yet
            reparentNode(id, parents, index);
        } else {
//             kWarning() << id << " is missing";
        }

        const QModelIndexList &list = mapFromSourceAll(index);
        foreach (const QModelIndex &proxyIndex, list) {
            dataChanged(proxyIndex, proxyIndex);
        }
    }
    Q_ASSERT(!m_parentMap.values().contains(0));
}


void ReparentingModel::onSourceRemoveRows(const QModelIndex& sourceIndex, int begin, int end)
{
//     kDebug() << begin << end;
    for (int i = begin; i <= end; ++i) {
        QModelIndex sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);
        if (!sourceChildIndex.isValid()) {
//             kDebug() << "invalid source";
            continue;
        }
        //The structure we have is not necessarily the same as the one of the sourceModel, we need to remove the children of the source model in case we don't get a removed signal for them.
        //TODO write test
        if (sourceModel()->hasChildren(sourceChildIndex)) {
            onSourceRemoveRows(sourceChildIndex, 0, sourceModel()->rowCount(sourceChildIndex)-1);
        }
        QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(sourceChildIndex);
        foreach (TodoNode *node, nodes) {
            removeNode(node, true);
        }
    }
}

void ReparentingModel::resetInternalData()
{
    m_parentMap.clear();
    m_strategy->reset();
    TodoProxyModelBase::resetInternalData();
}

Qt::ItemFlags ReparentingModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return m_strategy->flags(index, TodoProxyModelBase::flags(index));
}

Qt::DropActions ReparentingModel::supportedDropActions() const
{
    Qt::DropActions actions = m_strategy->supportedDropActions();
    if (!sourceModel()) {
        return actions|Qt::IgnoreAction;
    }
    return actions|sourceModel()->supportedDropActions();
}

//TODO move to todoproxymodelbase
QStringList ReparentingModel::mimeTypes() const
{
    QStringList list = m_strategy->mimeTypes();
    if (sourceModel()) {
        list.append(sourceModel()->mimeTypes());
        return list;
    }
    list.append(QAbstractItemModel::mimeTypes());
    return list;
}

bool ReparentingModel::dropMimeData(const QMimeData* mimeData, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
//     kDebug() << row << column << parent;
    TodoNode *target;
    if (row < 0 || column < 0) {
        target = m_manager->nodeForIndex(parent);
    } else {
        target = m_manager->nodeForIndex(index(row, column, parent));
    }
    Q_ASSERT(target && m_parentMap.values().contains(target));
    if (m_strategy->onDropMimeData(m_parentMap.key(target), mimeData, action)) {
        return true;
    }
    return TodoProxyModelBase::dropMimeData(mimeData, action, row, column, parent);
}

//TODO move to todoproxymodelbase
QMimeData *ReparentingModel::mimeData(const QModelIndexList &indexes) const
{
    QModelIndexList sourceIndexes;
    foreach (const QModelIndex &proxyIndex, indexes) {
        const QModelIndex &sourceIndex = mapToSource(proxyIndex);
        if (sourceIndex.isValid() && !sourceIndexes.contains(sourceIndex)) {
            sourceIndexes << mapToSource(proxyIndex);
        }
    }
    if (!sourceIndexes.isEmpty()) {
        return sourceModel()->mimeData(sourceIndexes);
    }
    
    return m_strategy->mimeData(indexes);
}

bool ReparentingModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role!=Qt::EditRole || !index.isValid()) {
        return TodoProxyModelBase::setData(index, value, role);
    }
    TodoNode *node = m_manager->nodeForIndex(index);
    Q_ASSERT(node && m_parentMap.values().contains(node));
    if (m_strategy->onSetData(m_parentMap.key(node), value, role)) {
        return true;
    }
    return TodoProxyModelBase::setData(index, value, role);
}
