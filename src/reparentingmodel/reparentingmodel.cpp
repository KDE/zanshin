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

QList<TodoNode*> ReparentingModel::insertNode(const Id &identifier, const QString &name, QList<TodoNode*> parentNodes, const QModelIndex &sourceIndex)
{
    if (parentNodes.isEmpty()) {
        TodoNode *parentNode = 0;
        int row = m_manager->roots().size();
        beginInsertRows(m_manager->indexForNode(parentNode, 0), row, row);
        TodoNode *node;
        if (sourceIndex.isValid()) {
            node = new TodoNode(sourceIndex, parentNode);
            //Don't set anything on real nodes
        } else { //For virtual nodes
            node = new TodoNode(parentNode);
            node->setData(name, 0, Qt::DisplayRole);
            node->setData(name, 0, Qt::EditRole);
            m_strategy->setNodeData(node, identifier);
        }

        Q_ASSERT(node);
        m_parentMap.insertMulti(identifier, node);
        m_manager->insertNode(node);
        endInsertRows();
        return QList<TodoNode*>() << node;
    }


    QList<TodoNode*> nodes;
    foreach (TodoNode *parentNode, parentNodes) {
        int row = parentNode->children().size();
        beginInsertRows(m_manager->indexForNode(parentNode, 0), row, row);
        TodoNode *node;
        if (sourceIndex.isValid()) {
            node = new TodoNode(sourceIndex, parentNode);
            //Don't set anything on real nodes
        } else { //For virtual nodes
            node = new TodoNode(parentNode);
            node->setData(name, 0, Qt::DisplayRole);
            node->setData(name, 0, Qt::EditRole);
            m_strategy->setNodeData(node, identifier);
        }

        Q_ASSERT(node);
        m_parentMap.insertMulti(identifier, node);
        m_manager->insertNode(node);
        endInsertRows();
//         Q_ASSERT(!m_parentMap.values().contains(0));
        nodes.append(node);
    }
    return nodes;
}

QList<TodoNode*> ReparentingModel::createNode(const Id &identifier, const IdList &parents, const QString &name, const QModelIndex &sourceIndex)
{
    kDebug() << "add node" << name << identifier << parents << sourceIndex;

    if (m_parentMap.contains(identifier)) { //We already have this node, we only need to update the temporary node FIXME this check is probably broken with multiparent?
        QList<TodoNode*> nodes = reparentNode(identifier, parents, sourceIndex);
//        kDebug() << "updating temporary nodes: " << identifier << nodes; 
        if (!sourceIndex.isValid() && !name.isEmpty()) {
            foreach (TodoNode *node, nodes) {
                node->setData(name, 0, Qt::DisplayRole); //FIXME setting data after the signals have been emitted is probably evil
            }
        }
        return nodes;
    }

    if (parents.isEmpty()) {
//        kDebug() << "parents is empty";
        return QList<TodoNode*>() << insertNode(identifier, name, QList<TodoNode*>(), sourceIndex);
    }

    QList<TodoNode*> nodeList;
    foreach (Id parentIdentifier, parents) {
        Q_ASSERT(parentIdentifier >= 0);
        if (!m_parentMap.contains(parentIdentifier)) {
//            kDebug() << "creating missing parent: " << parentIdentifier;
            createNode(parentIdentifier, IdList(), "unknown");
        }
        Q_ASSERT(m_parentMap.contains(parentIdentifier));
        QList<TodoNode*> parentNodes = m_parentMap.values(parentIdentifier);
        Q_ASSERT(!parentNodes.isEmpty());
        nodeList.append(insertNode(identifier, name, parentNodes, sourceIndex));
        //TODO create children
    }
    return nodeList;
}

void ReparentingModel::removeNode(Id id, bool removeChildren, bool cleanupStrategy)
{
    if (!m_parentMap.contains(id)) {
        kDebug() << "no such item " << id;
        return;
    }
    const QList<TodoNode*> &rootNodes = m_parentMap.values(id);
    foreach (TodoNode *root, rootNodes) {
//        kDebug() << "removeNode " << id;

        if (removeChildren) {
            foreach(TodoNode *childNode, root->children()) {
                Q_ASSERT(m_parentMap.values().contains(childNode));
                Id childId = m_parentMap.key(childNode);
                if (m_strategy->reparentOnParentRemoval(childId)) {
//                    kDebug() << "child " << childId;
                    IdList parents = m_strategy->getParents(childNode->rowSourceIndex(), IdList() << id);//Don't try to re-add it to the current parent (which is not yet removed)
                    reparentNode(childId, parents, childNode->rowSourceIndex());
                } else {
                    removeNode(childId, true);
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
        m_manager->removeNode(root);
        delete root;
        endRemoveRows();
    }
    Q_ASSERT(m_parentMap.contains(id));
    if (cleanupStrategy) {
        m_strategy->onNodeRemoval(id);
    }
    //This removes all in one go
    m_parentMap.remove(id);

}

void ReparentingModel::removeNodeById(Id id) //TODO remove
{
    if (!m_parentMap.contains(id)) {
        kDebug() << "no such item " << id;
        return;
    }
    removeNode(id, true);
}

void ReparentingModel::removeNodeFromParents(TodoNode *node)
{
    QMap<Id, TodoNode*>::iterator i = m_parentMap.begin();
    while (i != m_parentMap.end()) {
        if (i.value() == node) {
            i = m_parentMap.erase(i);
            return;
        } else {
            ++i;
        }
    }
}

void ReparentingModel::removeChildren(const QList<TodoNode*> &children)
{
    foreach(TodoNode *child, children) {
        removeChildren(child->children());
        m_manager->removeNode(child);
        removeNodeFromParents(child); //TODO check if this is really required
        delete child;
    }
}

QList<TodoNode*> ReparentingModel::cloneChildren(const QList<TodoNode*> &children)
{
    QList<TodoNode*> list;
    foreach (TodoNode *node, children) {
        Q_ASSERT(node);
        Id id = m_parentMap.key(node);
        TodoNode *newNode;
        if (node->rowSourceIndex().isValid()) {
            newNode = new TodoNode(node->rowSourceIndex());
        } else {
            newNode = new TodoNode();
            newNode->setData(node->data(0, Qt::DisplayRole), 0, Qt::DisplayRole);
            newNode->setData(node->data(0, Qt::EditRole), 0, Qt::EditRole);
            m_strategy->setNodeData(newNode, id);
        }
//         newNode->setData(newNode->data()); //TODO copy all data we need

        QList<TodoNode*> newChildren = cloneChildren(node->children());
        foreach (TodoNode *newChild, newChildren) {
            newChild->setParent(newNode);
            //Never add to the node manager before a parent is set, otherwise the nodes end up in the root-set as well
            m_manager->insertNode(newChild);
        }
        m_parentMap.insertMulti(id, newNode);
        list.append(newNode);
    }
    return list;
}

QList<TodoNode*> ReparentingModel::reparentNode(const Id& p, const IdList& parents, const QModelIndex &sourceIndex)
{
    Q_ASSERT(p >= 0); //Not sure if this assert is correct
    if (p < 0) {
        kWarning() << "invalid item";
        return QList<TodoNode*>();
    }
    kDebug() << p << parents << sourceIndex;
    QList<TodoNode*> oldNodes = m_parentMap.values(p);
    Q_ASSERT(!oldNodes.isEmpty());

    //First check if we need to update anything
    //We need this check because i.e. onSourceDataChanged simply calls this function, which would result in a complete removal of the node and readding
    QSet<Id> oldParents;
    bool updateSourceIndex = false;
    foreach (TodoNode *node, oldNodes) {
        if (node->rowSourceIndex() != sourceIndex) {
            kDebug() << "updating the sourceindex";
            updateSourceIndex = true;
        }
        if (m_parentMap.values().contains(node->parent())) {
            Id id = m_parentMap.key(node->parent());
            if (!oldParents.contains(id)) {
                oldParents << id;
            }
        }
    }

    bool parentsChanged = false;
    foreach (Id id, parents) {
        if(!oldParents.remove(id)) {
            parentsChanged = true;
        }
    }
    if (!oldParents.isEmpty()) {
        parentsChanged = true;
    }
    if (!parentsChanged && !updateSourceIndex) {
//        kDebug() << "nothing to do";
        return QList<TodoNode*>();
    }

    //TODO when updating the sourceIndex, we should avoid emitting signals (respectively only emit changed signals for the updated name)


    //Store what we need to create the new nodes
    //could be obtained from the strategy instead
    const QString &name = oldNodes.first()->data(0, Qt::DisplayRole).toString();
    QModelIndex index = sourceIndex; //The sourceindex must be the same for all (if it is the same id it always maps to one sourceIndex)
    if (!sourceIndex.isValid() && oldNodes.first()->rowSourceIndex().isValid()) {
        index = oldNodes.first()->rowSourceIndex();
    }
    //We have to store the child structure, to be able to rebuild it later on when the new nodes have been created
    QList< QList< TodoNode* > > children;
    foreach (TodoNode *oldNode, oldNodes) {
        children.append(oldNode->children());
    }

    /*
     * remove node from any current parent, don't touch the subtree (it moves along)
     * and don't remove the node from the strategy (it already has the updated parent information)
     */
    removeNode(p, false, false);

    //Create new nodes (one per parent
    QList<TodoNode*> newNodes = createNode(p, parents, name, index);

    //Now assign or copy a set of the stored children to each created node
    int count = 0;
    foreach (TodoNode *newNode, newNodes) {
//        kDebug() << newNode->data(0, Qt::DisplayRole) << count << newNode;
        if (count <= (children.size()-1)) {
            foreach (TodoNode* child, children.at(count)) {
                child->setParent(newNode);
            }
        } else {
            const QList<TodoNode *> clones = cloneChildren(children.at(0)); //It doesn't matter which children we copy
            foreach (TodoNode* child, clones) {
                child->setParent(newNode);
                m_manager->insertNode(child);
            }
        }
        count++;
    }
    //Dispose the unused children trees
    for (int i = count; i < children.size(); i++) {
//         kDebug() << "remove superfluous children " << i;
        removeChildren(children.at(i));
    }

    return newNodes;
}

void ReparentingModel::renameNode(const Id& identifier, const QString& name)
{
//     kDebug() << "renamed " << identifier << " to " << name;
    Q_ASSERT(m_parentMap.contains(identifier));
    QList<TodoNode*> nodes = m_parentMap.values(identifier);
    foreach(TodoNode *node, nodes) {
        node->setData(name, 0, Qt::DisplayRole);
        node->setData(name, 0, Qt::EditRole);
        const QModelIndex &begin = m_manager->indexForNode(node, 0);
        const QModelIndex &end = m_manager->indexForNode(node, 0);
        emit dataChanged(begin, end);
    }
}

void ReparentingModel::onSourceInsertRows(const QModelIndex& sourceIndex, int begin, int end)
{
    kDebug() << sourceIndex << begin << end;
    for (int i = begin; i <= end; i++) {
        const QModelIndex &sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);

        if (!sourceChildIndex.isValid()) {
            kWarning() << "invalid sourceIndex";
            continue;
        }

        Id id = m_strategy->getId(sourceChildIndex);
        bool replaceTemporaryNode = false;
        if (id > 0) {
            IdList parents = m_strategy->getParents(sourceChildIndex);
            replaceTemporaryNode = m_parentMap.contains(id);
            kDebug() << "replacing existing node " << replaceTemporaryNode << id;
            createNode(id, parents, QString(), sourceChildIndex);
        } else {
            kDebug() << "ingore node";
        }

        //Insert children too
        if (!replaceTemporaryNode && sourceModel()->hasChildren(sourceChildIndex)) {
             kDebug() << id << " has children";
            onSourceInsertRows(sourceChildIndex, 0, sourceModel()->rowCount(sourceChildIndex)-1);
        }
    }
}

void ReparentingModel::onSourceDataChanged(const QModelIndex& begin, const QModelIndex& end)
{
//     kDebug() << begin << end;
    for (int row = begin.row(); row <= end.row(); row++) {
        const QModelIndex &index = sourceModel()->index(row, 0, begin.parent());
        Id id = m_strategy->getId(index);
        if (id < 0) {
            //TODO write a test for the case that an item becomes hidden after being already in the model
            QList<TodoNode *>nodes = m_manager->nodesForSourceIndex(index);
            if (!nodes.isEmpty()) {
                Id oldId = m_parentMap.key(nodes.first());
                removeNode(oldId, true); //remove if the sourceindex was in this model but is now hidden
            }
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
//     Q_ASSERT(!m_parentMap.values().contains(0));
}


void ReparentingModel::onSourceRemoveRows(const QModelIndex& sourceIndex, int begin, int end)
{
//    kDebug() << begin << end;
    for (int i = begin; i <= end; ++i) {
        QModelIndex sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);
        if (!sourceChildIndex.isValid()) {
            kDebug() << "invalid source";
            continue;
        }
        //The structure we have is not necessarily the same as the one of the sourceModel, we need to remove the children of the source model in case we don't get a removed signal for them.
        //TODO write test
        if (sourceModel()->hasChildren(sourceChildIndex)) {
            onSourceRemoveRows(sourceChildIndex, 0, sourceModel()->rowCount(sourceChildIndex)-1);
        }
        Id id = m_strategy->getId(sourceChildIndex);
        removeNode(id, true);
        //TODO if we wanted to delete children of a collection we could do so here, but IMO it doesn't make sense and is signaled through ETM anyways (the removal of the child items)
        //actually I'm not sure if we're supposed ot remove child nodes as well.... Maybe we never get a removed signal for the childred
        //if it is correct, write test for it in reparentingmodelspec
        //also check behaviour if only some of the children are filtered

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
    return actions/*|sourceModel()->supportedDropActions()*/;
}

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
//    kDebug() << row << column << parent;
    TodoNode *target;
    if (row < 0 || column < 0) {
        target = m_manager->nodeForIndex(parent);
    } else {
        target = m_manager->nodeForIndex(index(row, column, parent));
    }
    Q_ASSERT(target);
    Q_ASSERT(m_parentMap.values().contains(target));
    if (m_strategy->onDropMimeData(m_parentMap.key(target), mimeData, action)) {
        return true;
    }
    //Calling the QAbstractItemModel implementation results in a crash, maybe add an implementation to todoproxymodelbase?
    return false;
}

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
//     kDebug() << value << (role==Qt::EditRole) << index.isValid();
    if (role!=Qt::EditRole || !index.isValid()) {
        return TodoProxyModelBase::setData(index, value, role);
    }
    TodoNode *node = m_manager->nodeForIndex(index);
    Q_ASSERT(node && m_parentMap.values().contains(node));
    if (m_strategy->onSetData(m_parentMap.key(node), value, role, index.column())) {
        return true;
    }
    return TodoProxyModelBase::setData(index, value, role);
}

QVariant ReparentingModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    TodoNode *node = m_manager->nodeForIndex(index);
    if (!node) {
        return QVariant();
    }
    Q_ASSERT(node && m_parentMap.values().contains(node));
    const QVariant &var = m_strategy->data(m_parentMap.key(node), role);
    if (var.isValid()) {
        return var;
    }
    return node->data(index.column(), role);
}

