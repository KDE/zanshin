/* This file is part of Zanshin Todo.

   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>

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

#include "parentstructuremodel.h"
#include "todonode.h"
#include "globaldefs.h"
#include <KIcon>
#include <KLocalizedString>
#include "todonodemanager.h"
#include <tagmanager.h>
#include <QMimeData>
#include <pimitemmodel.h>
#include <abstractpimitem.h>
#include <queries.h>
#include <pimitem.h>

ParentStructureModel::ParentStructureModel(ParentStructureStrategy *adapter, QObject* parent)
: TodoProxyModelBase(MultiMapping, parent), m_rootNode(0), m_nepomukAdapter(adapter)
{
    adapter->setModel(this);
}

ParentStructureModel::~ParentStructureModel()
{

}

TodoNode* ParentStructureModel::createInbox() const
{
    TodoNode *node = new TodoNode;

    node->setData(i18n("No Topic"), 0, Qt::DisplayRole);
    node->setData(KIcon("mail-folder-inbox"), 0, Qt::DecorationRole);
    node->setRowData(Zanshin::Inbox, Zanshin::ItemTypeRole);

    return node;
}

void ParentStructureModel::init()
{
//     kDebug();
    TodoProxyModelBase::init();
    
    if (!m_rootNode) {
        beginInsertRows(QModelIndex(), 1, 1);

        TodoNode *node = new TodoNode;
        node->setData(i18n("Topics"), 0, Qt::DisplayRole);
        node->setData(KIcon("document-multiple"), 0, Qt::DecorationRole);
        node->setRowData(Zanshin::TopicRoot, Zanshin::ItemTypeRole);

        m_rootNode = node;
        m_manager->insertNode(m_rootNode);

        endInsertRows();
    }
    m_nepomukAdapter->init();
}




void ParentStructureModel::itemParentsChanged(const QModelIndex& sourceIndex, const IdList& parents)
{
    kDebug() << sourceIndex << parents;
    if (!sourceIndex.isValid()) {
        kWarning() << "invalid item";
        return;
    }
    QList<TodoNode*> parentNodes;
    foreach(const Id &p, parents) {
        TodoNode *pa = m_resourceMap[p];
        Q_ASSERT(pa);
        kDebug() << "newparent : " << pa;
        parentNodes.append(pa);
    }
    bool backToInbox = parentNodes.isEmpty();
    
    //remove node from any current parent
    QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(sourceIndex);
    Q_ASSERT(!nodes.empty());
    foreach (TodoNode *node, nodes) {
        TodoNode *parentNode = node->parent();
        kDebug() << "remove " << node << node->parent();
        if (parentNode) {
            //Don't remove/readd for parents which remain
            if (parentNodes.removeOne(parentNode)) {
                continue;
            }
            //Don't remove/readd for parents which remain
            if (backToInbox && (parentNode == m_inboxNode)) {
                backToInbox = false;
                continue;
            }
            int oldRow = parentNode->children().indexOf(node);
            beginRemoveRows(m_manager->indexForNode(parentNode, 0), oldRow, oldRow); //FIXME triggers multimapping warning, but there shouldn't be multiple instances of the same item under inbox
            m_manager->removeNode(node);
            delete node;
            endRemoveRows();
        } else {
            kDebug() << "why is there no parent?";
        }
    }
    
    //If no parents are available, back to inbox
    if (backToInbox) {
        addChildNode(sourceIndex, m_inboxNode);
        kDebug() << "add to inbox";
        return;
    }
    
    foreach(TodoNode *p, parentNodes) {
        addChildNode(sourceIndex, p);
    }
}

void ParentStructureModel::reparentParent(const Id& p, const Id& parent)
{
    if (p < 0) {
        kWarning() << "invalid item";
        return;
    }
    
    TodoNode *node = m_resourceMap[p];
    Q_ASSERT(node);
    const QString &name = node->data(0, Qt::DisplayRole).toString();
    
    QList<TodoNode*> children = node->children();
    foreach (TodoNode* child, children) {
        child->setParent(0);
    }

    //remove node from any current parent
    TodoNode *parentNode = node->parent();
    Q_ASSERT(parentNode);
    int oldRow = parentNode->children().indexOf(node);
    beginRemoveRows(m_manager->indexForNode(parentNode, 0), oldRow, oldRow); //FIXME triggers multimapping warning, but there shouldn't be multiple instances of the same item under inbox
    Q_ASSERT(m_resourceMap.values().contains(node));
    m_resourceMap.remove(m_resourceMap.key(node));
    m_manager->removeNode(node);
    delete node;
    endRemoveRows();
    
    TodoNode *newParent = createNode(p, parent, name);
    foreach (TodoNode* child, children) {
        child->setParent(newParent);
    }
}

void ParentStructureModel::renameParent(const Id& identifier, const QString& name)
{
    kDebug() << "renamed " << identifier << " to " << name;
    TodoNode *node = m_resourceMap[identifier];
    node->setData(name, 0, Qt::DisplayRole);
    node->setData(name, 0, Qt::EditRole);
    const QModelIndex &begin = m_manager->indexForNode(node, 0);
    const QModelIndex &end = m_manager->indexForNode(node, 0);
    emit dataChanged(begin, end);
}

void ParentStructureModel::createOrUpdateParent(const Id& identifier, const Id& parentIdentifier, const QString& name)
{
    if (!m_resourceMap.contains(identifier)) {
        createNode(identifier, parentIdentifier, name);
        return;
    }
    //if the node was already created we have to rename it now
    renameParent(identifier, name);
    reparentParent(identifier, parentIdentifier);
}


TodoNode *ParentStructureModel::createNode(const Id &identifier, const Id &parentIdentifier, const QString &name)
{
    kDebug() << "add topic" << name << identifier;
    TodoNode* parentNode = 0;
    if (parentIdentifier >= 0) {
        if (!m_resourceMap.contains(parentIdentifier)) {
            createNode(parentIdentifier, -1, "unknown");
        }
        Q_ASSERT(m_resourceMap.contains(parentIdentifier));
        parentNode = m_resourceMap[parentIdentifier];
    } else {
        parentNode = m_rootNode; 
    }
    Q_ASSERT(parentNode);

    int row = parentNode->children().size();
    beginInsertRows(m_manager->indexForNode(parentNode, 0), row, row);
    TodoNode *node = new TodoNode(parentNode);
    node->setData(name, 0, Qt::DisplayRole);
    node->setData(name, 0, Qt::EditRole);
    node->setRowData(identifier, IdRole);
    m_nepomukAdapter->setData(node, identifier);
    
    m_resourceMap[identifier] = node;
    m_manager->insertNode(node);
    kDebug() << identifier << node;
    endInsertRows();
    return node;
}

void ParentStructureModel::removeNode(const Id &identifier)
{
    kDebug() << identifier;
    if (!m_resourceMap.contains(identifier)) {
        return;
    }

    TodoNode *node = m_resourceMap[identifier];
    m_nepomukAdapter->onNodeRemoval(identifier);
    QList<TodoNode*> children = node->children();
    foreach (TodoNode* child, children) {
        
        QModelIndex childIndex = m_manager->indexForNode(child, 0);
        if (childIndex.data(IdRole).canConvert<Id>()) {
            Id id = childIndex.data(IdRole).value<Id>();
            removeNode(id); //Recursive removal of child parent nodes
        } else {
            child->setParent(m_inboxNode);
        }
    }

    QModelIndex index = m_manager->indexForNode(node, 0);
    beginRemoveRows(index.parent(), index.row(), index.row());
    m_manager->removeNode(node);
    m_resourceMap.remove(identifier);
    delete node;
    endRemoveRows();
}

void ParentStructureModel::onSourceInsertRows(const QModelIndex& sourceIndex, int begin, int end)
{
//     kDebug() << begin << end << sourceIndex;
//     kDebug() << sourceModel()->rowCount();
    for (int i = begin; i <= end; i++) {
        QModelIndex sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);

        if (!sourceChildIndex.isValid()) {
            kDebug() << "invalid sourceIndex";
            continue;
        }
        
        IdList parents = m_nepomukAdapter->onSourceInsertRow(sourceChildIndex);
        
        TodoNode *node;
        if (parents.isEmpty()) {
            kDebug() << "add node to inbox";
            node = addChildNode(sourceChildIndex, m_inboxNode);
        } else {
            foreach (const Id &res, parents) {
                kDebug() << "added node to topic: " << res;
                TodoNode *parent = m_resourceMap[res];
                if (!parent) { //if the item is before the parent, the parent may not be existing yet.
                    createNode(res, -1, "unknown");
                    Q_ASSERT(m_resourceMap.contains(res));
                    parent = m_resourceMap[res];
                }
                Q_ASSERT(parent);
                node = addChildNode(sourceChildIndex, parent);
            }
        }
    }
}

void ParentStructureModel::onSourceRemoveRows(const QModelIndex& sourceIndex, int begin, int end)
{
    for (int i = begin; i <= end; ++i) {
        QModelIndex sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);
        QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(sourceChildIndex);
        foreach (TodoNode *node, nodes) {
            TodoNode *parentNode = node->parent();
            if (parentNode) {
                kDebug() << "removed node";
                int oldRow = parentNode->children().indexOf(node);
                beginRemoveRows(m_manager->indexForNode(parentNode, 0), oldRow, oldRow);
                m_manager->removeNode(node);
                delete node;
                endRemoveRows();
            }
        }
    }
}

void ParentStructureModel::onSourceDataChanged(const QModelIndex& begin, const QModelIndex& end)
{
    kDebug() << begin << end;
    for (int row = begin.row(); row <= end.row(); row++) {
        const QModelIndex &index = sourceModel()->index(row, 0, begin.parent());
        const IdList &parents = m_nepomukAdapter->onSourceDataChanged(index);
        
        itemParentsChanged(index, parents);

        const QModelIndexList &list = mapFromSourceAll(index);
        
        foreach (const QModelIndex &proxyIndex, list) {
            dataChanged(proxyIndex, proxyIndex);
        }
    }
}

QStringList ParentStructureModel::mimeTypes() const
{
    QStringList list = QAbstractItemModel::mimeTypes();
    list.append("text/uri-list");
    list.append("text/plain");
    return list;
}

Qt::ItemFlags ParentStructureModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Zanshin::ItemType type = (Zanshin::ItemType) index.data(Zanshin::ItemTypeRole).toInt();
    if (type == Zanshin::Inbox || type == Zanshin::TopicRoot) {
        return Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    }
    return TodoProxyModelBase::flags(index) | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
}



Qt::DropActions ParentStructureModel::supportedDropActions() const
{
    if (!sourceModel()) {
        return Qt::IgnoreAction;
    }
    return sourceModel()->supportedDropActions();
}

bool ParentStructureModel::dropMimeData(const QMimeData* mimeData, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    //kDebug() << mimeData->formats();
    //kDebug() << mimeData->text();
    if (parent.data(IdRole).canConvert<Id>()) {
        Id id = parent.data(IdRole).value<Id>();
        //TODO make use of row/column to find correct child instead? Seems to work....
        return m_nepomukAdapter->onDropMimeData(mimeData, action, id);
    } else {
        m_nepomukAdapter->onDropMimeData(mimeData, action, -1);
    }
    
    return false;
}

bool ParentStructureModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role!=Qt::EditRole || !index.isValid()) {
        return TodoProxyModelBase::setData(index, value, role);
    }
    
    if (index.data(IdRole).canConvert<Id>() && m_nepomukAdapter->onSetData(index.data(IdRole).value<Id>(), value, role)) {
        return true;
    }

    return TodoProxyModelBase::setData(index, value, role);
}
