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

ReparentingModel::ReparentingModel(ReparentingStrategy* strategy, QObject* parent)
:   TodoProxyModelBase(TodoProxyModelBase::MultiMapping, parent),
//     m_rootNode(0),
    m_strategy(strategy)
{

}

ReparentingModel::~ReparentingModel()
{

}

void ReparentingModel::init()
{
//     if (!m_rootNode) {
//         beginInsertRows(QModelIndex(), 1, 1);
// 
//         TodoNode *node = new TodoNode;
//         node->setData(i18n("Topics"), 0, Qt::DisplayRole);
//         node->setData(KIcon("document-multiple"), 0, Qt::DecorationRole);
//         node->setRowData(Zanshin::TopicRoot, Zanshin::ItemTypeRole);
// 
//         m_rootNode = node;
//         m_manager->insertNode(m_rootNode);
// 
//         endInsertRows();
//     }
//     m_strategy->init();
}

TodoNode* ReparentingModel::createInbox() const
{
//     TodoNode *node = new TodoNode;
// 
//     node->setData(i18n("No Topic"), 0, Qt::DisplayRole);
//     node->setData(KIcon("mail-folder-inbox"), 0, Qt::DecorationRole);
//     node->setRowData(Zanshin::Inbox, Zanshin::ItemTypeRole);
// 
//     return node;
    return 0;
}

TodoNode *ReparentingModel::createNode(const Id &identifier, const Id &parentIdentifier, const QString &name, const QModelIndex &sourceIndex)
{    
//     kDebug() << "add topic" << name << identifier;
    TodoNode* parentNode = 0;
    if (parentIdentifier >= 0) {
        if (!m_parentMap.contains(parentIdentifier)) {
            createNode(parentIdentifier, -1, "unknown");
        }
        Q_ASSERT(m_parentMap.contains(parentIdentifier));
        parentNode = m_parentMap[parentIdentifier];
    } /*else {
        parentNode = m_rootNode;
    }
    Q_ASSERT(parentNode);*/

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
    } else {
        node = new TodoNode(parentNode);
        node->setData(name, 0, Qt::DisplayRole);
        node->setData(name, 0, Qt::EditRole);
    }
    node->setRowData(identifier, IdRole);
    //TODO
//     m_strategy->setData(node, identifier);

    m_parentMap[identifier] = node;
    m_manager->insertNode(node);
    kDebug() << identifier << node;
    endInsertRows();
    return node;
}

void ReparentingModel::reparentParent(const Id& p, const IdList& parents, const QModelIndex &sourceIndex)
{
    if (p < 0) {
        kWarning() << "invalid item";
        return;
    }

    //TODO handle multiple parents
    TodoNode *node = m_parentMap[p];
    Q_ASSERT(node);
//     if (!node) { //TODO is this a valid case?
//         return;
//     }
    const QString &name = node->data(0, Qt::DisplayRole).toString();

    QList<TodoNode*> children = node->children();
    foreach (TodoNode* child, children) {
        child->setParent(0);
    }

    //TODO check before removing which parents stay, and only change the ones which actually change (sourceIndex must also be checked)

    //remove node from any current parent
    TodoNode *parentNode = node->parent();
    int oldRow;
    QModelIndex parentIndex;
    if (parentNode) {
        oldRow = parentNode->children().indexOf(node);
        parentIndex = m_manager->indexForNode(parentNode, 0);
    } else {
        oldRow = m_manager->roots().indexOf(node);
    }
    beginRemoveRows(parentIndex, oldRow, oldRow);//FIXME triggers multimapping warning, but there shouldn't be multiple instances of the same item under inbox
    Q_ASSERT(m_parentMap.values().contains(node));
    m_parentMap.remove(m_parentMap.key(node));
    m_manager->removeNode(node);
    delete node;
    endRemoveRows();

    TodoNode *newParent;
    if (parents.isEmpty()) {
        newParent = createNode(p, -1, name, sourceIndex);
    } else {
        newParent = createNode(p, parents.first(), name, sourceIndex);
    }
    
    foreach (TodoNode* child, children) {
        child->setParent(newParent);
    }
}

void ReparentingModel::renameParent(const Id& identifier, const QString& name)
{
    kDebug() << "renamed " << identifier << " to " << name;
    TodoNode *node = m_parentMap[identifier];
    node->setData(name, 0, Qt::DisplayRole);
    node->setData(name, 0, Qt::EditRole);
    const QModelIndex &begin = m_manager->indexForNode(node, 0);
    const QModelIndex &end = m_manager->indexForNode(node, 0);
    emit dataChanged(begin, end);
}

void ReparentingModel::onSourceInsertRows(const QModelIndex& sourceIndex, int begin, int end)
{
    for (int i = begin; i <= end; i++) {
        const QModelIndex &sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);
        
        if (!sourceChildIndex.isValid()) {
            kWarning() << "invalid sourceIndex";
            continue;
        }

        Id id = m_strategy->getId(sourceChildIndex);
        IdList parents = m_strategy->onSourceInsertRow(sourceChildIndex);
        kDebug() << "adding node: " << id;
        //The item has already been inserted before, update and reparent
        if (m_parentMap.contains(id)) {
            kDebug() << "update parent";
            //m_parentMap[id]->setRowSourceIndex(sourceChildIndex);
//             m_manager->insertNode(m_parentMap[id]); //Update sourceChildIndex cache
            // We need to convert a node without corresponding sourceIndex to one with a sourceIndex
            // 
            reparentParent(id, parents, sourceChildIndex);
            return;
        }

        //Insert new item
        if (parents.isEmpty()) {
            m_parentMap[id] = addChildNode(sourceChildIndex, 0);
        } else {
            foreach (const Id &p, parents) {
                kDebug() << "added node to parent: " << p;
                TodoNode *parent = m_parentMap[p];
                if (!parent) { //if the item is before the parent, the parent may not be existing yet.
                    kDebug() << "creating parent";
                    createNode(p, -1, "unknown");
                    Q_ASSERT(m_parentMap.contains(p));
                    parent = m_parentMap[p];
                }
                Q_ASSERT(parent);
                m_parentMap[id] = addChildNode(sourceChildIndex, parent); //TODO set data
                //TODO parent map is a multimap actually, act accordingly (this applies to all places where parent map is used). This because a node can be in multiple places at the same time.
            }
        }

        //Insert children too
        if (sourceModel()->hasChildren(sourceChildIndex)) {
            kDebug() << id << " has childred";
            onSourceInsertRows(sourceChildIndex, 0, sourceModel()->rowCount(sourceChildIndex)-1);
        }
        
    }
}

void ReparentingModel::onSourceDataChanged(const QModelIndex& begin, const QModelIndex& end)
{
    kDebug() << begin << end;
    for (int row = begin.row(); row <= end.row(); row++) {
        const QModelIndex &index = sourceModel()->index(row, 0, begin.parent());
        Id id = m_strategy->getId(index);
        const IdList &parents = m_strategy->onSourceDataChanged(index);
        if (m_parentMap.contains(id)) { //In case we get updates for indexes which haven't been inserted yet
            reparentParent(id, parents);
        }

        const QModelIndexList &list = mapFromSourceAll(index);
        foreach (const QModelIndex &proxyIndex, list) {
            dataChanged(proxyIndex, proxyIndex);
        }
    }
}

void ReparentingModel::onSourceRemoveRows(const QModelIndex& sourceIndex, int begin, int end)
{
    kDebug() << begin << end;
    for (int i = begin; i <= end; ++i) {
        QModelIndex sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);
        QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(sourceChildIndex);
        foreach (TodoNode *node, nodes) {
            Id id = m_parentMap.key(node);
            m_strategy->onNodeRemoval(id);
            foreach(TodoNode *childNode, node->children()) {
                Id childId = m_parentMap.key(childNode);
                reparentParent(childId, m_strategy->getParents(childId), childNode->rowSourceIndex());
            }
            if (TodoNode *parentNode = node->parent()) {
                kDebug() << "removed node";
                int oldRow = parentNode->children().indexOf(node);
                beginRemoveRows(m_manager->indexForNode(parentNode, 0), oldRow, oldRow);
                m_manager->removeNode(node);
                delete node;
                endRemoveRows();
            } //TODO handle toplevel removals
        }
    }
}

void ReparentingModel::resetInternalData()
{
    m_parentMap.clear();
    TodoProxyModelBase::resetInternalData();
}



