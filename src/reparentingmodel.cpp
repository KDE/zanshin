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

TodoNode *ReparentingModel::createNode(const Id &identifier, const Id &parentIdentifier, const QString &name)
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
    TodoNode *node = new TodoNode(parentNode);
    node->setData(name, 0, Qt::DisplayRole);
    node->setData(name, 0, Qt::EditRole);
    node->setRowData(identifier, IdRole);
    //TODO
//     m_strategy->setData(node, identifier);

    m_parentMap[identifier] = node;
    m_manager->insertNode(node);
    kDebug() << identifier << node;
    endInsertRows();
    return node;
}

void ReparentingModel::reparentParent(const Id& p, const Id& parent)
{
    if (p < 0) {
        kWarning() << "invalid item";
        return;
    }

    TodoNode *node = m_parentMap[p];
    Q_ASSERT(node);
    const QString &name = node->data(0, Qt::DisplayRole).toString();

    QList<TodoNode*> children = node->children();
    foreach (TodoNode* child, children) {
        child->setParent(0);
    }

    //remove node from any current parent
    TodoNode *parentNode = node->parent();
    if (parentNode) {
        int oldRow = parentNode->children().indexOf(node);
        beginRemoveRows(m_manager->indexForNode(parentNode, 0), oldRow, oldRow); //FIXME triggers multimapping warning, but there shouldn't be multiple instances of the same item under inbox
        Q_ASSERT(m_parentMap.values().contains(node));
        m_parentMap.remove(m_parentMap.key(node));
        m_manager->removeNode(node);
        delete node;
        endRemoveRows();
    }
    //FIXME remove from root here?

    TodoNode *newParent = createNode(p, parent, name);
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

void ReparentingModel::createOrUpdateParent(const Id& identifier, const Id& parentIdentifier, const QString& name)
{
    if (!m_parentMap.contains(identifier)) {
        createNode(identifier, parentIdentifier, name);
        return;
    }
    //if the node was already created we have to rename it now
    renameParent(identifier, name);
    reparentParent(identifier, parentIdentifier);
}

void ReparentingModel::onSourceInsertRows(const QModelIndex& sourceIndex, int begin, int end)
{
    for (int i = begin; i <= end; i++) {
        QModelIndex sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);
        
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
            createOrUpdateParent(id, -1, "unknown"); //TODO get name
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
                //TODO parent map is a multimap actually, act accordingly
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

}

void ReparentingModel::onSourceRemoveRows(const QModelIndex& sourceIndex, int begin, int end)
{

}

void ReparentingModel::resetInternalData()
{
    TodoProxyModelBase::resetInternalData();
}



