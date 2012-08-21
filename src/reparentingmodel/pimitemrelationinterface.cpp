/*
 * Copyright (C) 2012  Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pimitemrelationinterface.h"

#include <QtCore/QAbstractItemModel>

#include <KDE/Akonadi/ItemModifyJob>
#include <Akonadi/ItemCreateJob>
#include <KDE/KCalCore/Todo>
#include <KDE/KDebug>
#include <KDE/KGlobal>
#include <KDE/KLocale>
#include <KDE/KMessageBox>

#include "globaldefs.h"
#include "todohelpers.h"
#include <pimitem.h>
#include <note.h>

K_GLOBAL_STATIC(PimItemRelationInterface, s_contextManager)
K_GLOBAL_STATIC(PimItemRelationInterface, s_topicManager)
K_GLOBAL_STATIC(ProjectStructureInterface, s_projectManager)

PimNode PimItemStructureInterface::fromIndex(const QModelIndex &index)
{
     Zanshin::ItemType itemType = (Zanshin::ItemType)index.data(Zanshin::ItemTypeRole).toInt();
     switch (itemType) {
         case Zanshin::Collection: {
             PimNode node(PimNode::Collection);
             node.collection = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
             return node;
         }
         case Zanshin::ProjectTodo: {
             PimNode node (PimNode::Project);
             node.item = index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
             return node;
         }
         case Zanshin::Category:
         case Zanshin::CategoryRoot: {
             PimNode node (PimNode::Context);
             node.relationId = index.data(Zanshin::RelationIdRole).value<Id>();
             return node;
         }
         case Zanshin::Topic:
         case Zanshin::TopicRoot: {
             PimNode node (PimNode::Topic);
             node.relationId = index.data(Zanshin::RelationIdRole).value<Id>();
             return node;
         }
         case Zanshin::StandardTodo: {
             PimNode node (PimNode::PimItem);
             node.item = index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
             return node;
         }
     }
     Q_ASSERT(false);
     return PimNode(PimNode::Invalid);
}

// PimNode PimItemStructureInterface::projectNode(const Akonadi::Item& )
// {
// 
// }
// 
// PimNode PimItemStructureInterface::contextNode(Id )
// {
// 
// }
// 
// PimNode PimItemStructureInterface::topicNode(Id )
// {
// 
// }
// 
// PimNode PimItemStructureInterface::todoNode(const Akonadi::Item& )
// {
// 
// }
// 
// PimNode PimItemStructureInterface::noteNode(const Akonadi::Item& )
// {
// 
// }
// 
// PimNode PimItemStructureInterface::collectionNode(const Akonadi::Collection& )
// {
// 
// }
// 

void PimItemStructureInterface::create(PimNode::NodeType type, const QString& name, const QList< PimNode >& parents, const Akonadi::Collection& col)
{
    switch (type) {
        case PimNode::Project:
            TodoHelpers::addProject(name, col);
            break;
        case PimNode::Todo:
//             QString parent;
//             if (!parents.isEmpty()) {
//                 parent = parents.first().item;
//             }
            TodoHelpers::addTodo(name, QString(), QString(), col);
            break;
        case PimNode::Note: {
            Note note;
            note.setTitle(name);
            Akonadi::ItemCreateJob *itemCreateJob = new Akonadi::ItemCreateJob(note.getItem(), col);
            break;
        }
        case PimNode::Context:
            PimItemStructureInterface::contextInstance().add(name, parents);
            break;
        case PimNode::Topic:
            PimItemStructureInterface::topicInstance().add(name, parents);
            break;
        default:
            Q_ASSERT(0);
    }
}

void PimItemStructureInterface::remove(const PimNode& node, QWidget *)
{

}

void PimItemStructureInterface::remove(const QList< PimNode >& nodes, QWidget* )
{

}

void PimItemStructureInterface::moveTo(const PimNode& node, const PimNode& parent)
{

}

void PimItemStructureInterface::linkTo(const PimNode& node, const PimNode& parent)
{

}

void PimItemStructureInterface::unlink(const PimNode& node, const PimNode& parent)
{

}

void PimItemStructureInterface::rename(const PimNode& node, const QString& name)
{

}


PimItemStructureInterface &PimItemStructureInterface::contextInstance()
{
    Q_ASSERT(s_contextManager);
    return *s_contextManager;
}

PimItemStructureInterface &PimItemStructureInterface::topicInstance()
{
    Q_ASSERT(s_topicManager);
    return *s_topicManager;
}

ProjectStructureInterface& PimItemStructureInterface::projectInstance()
{
    Q_ASSERT(s_projectManager);
    return *s_projectManager;
}

void PimItemStructureInterface::setRelationsStructure(PimItemRelationCache *s)
{
    mStructure = s;
}




PimItemRelationInterface::PimItemRelationInterface()
{
}

PimItemRelationInterface::~PimItemRelationInterface()
{
}

static Id toId(const PimNode &index)
{
    return index.relationId;
}

static IdList toId(const QList<PimNode> &list)
{
    IdList parentIds;
    foreach (const PimNode &index, list) {
        parentIds << index.relationId;
    }
    return parentIds;
}

void PimItemRelationInterface::add(const QString& name, const QList<PimNode>& parents)
{
    //kDebug() << name << parentCategory;
    mStructure->addNode(name, toId(parents));
}

// bool PimItemRelationInterface::remove(QWidget* widget, const QModelIndexList& relations)
// {
//     IdList relationIds = toId(relations);
//     if (widget) {
//         QStringList categoryList;
//         foreach (Id category, relationIds) {
//             categoryList << mStructure->getName(category);
//         }
//         QString categoryName = categoryList.join(", ");
//         kDebug() << relationIds << categoryList;
//         QString title;
//         QString text;
//         //TODO adjust Context/Topic
//         if (relationIds.size() > 1) {
//             text = i18n("Do you really want to delete the context '%1'? All actions won't be associated to this context anymore.", categoryName);
//             title = i18n("Delete Context");
//         } else {
//             text = i18n("Do you really want to delete the contexts '%1'? All actions won't be associated to those contexts anymore.", categoryName);
//             title = i18n("Delete Contexts");
//         }
//         int button = KMessageBox::questionYesNo(widget, text, title);
//         bool canRemove = (button==KMessageBox::Yes);
// 
//         if (!canRemove) {
//             return false;
//         }
//     }
//     kDebug() << "remove " << relationIds;
//     foreach (Id id, relationIds) {
//         mStructure->removeNode(id);
//     }
//     return true;
// }
// 
// bool PimItemRelationInterface::remove(const Id& relation)
// {
//     kDebug() << relation;
//     mStructure->removeNode(relation);
//     return true;
// }
// 
// bool PimItemRelationInterface::moveTo(const QModelIndex& node, const QModelIndex& parent)
// {
//     Zanshin::ItemType parentType = (Zanshin::ItemType)parent.data(Zanshin::ItemTypeRole).toInt();
//     Id id = toId(node);
//     Id parentId = toId(parent);
//     kDebug() << id << parentId;
//     if (parentType!=Zanshin::Category && parentType!=Zanshin::CategoryRoot) { //TODO shouldn't be necessary
//         return false;
//     }
//     mStructure->moveNode(id, IdList() << parentId);
//     return true;
// }
// 
// bool PimItemRelationInterface::linkTo(const QModelIndex& node, const QModelIndex& parent)
// {
//     Id id = toId(node);
//     Id parentId = toId(parent);
//     kDebug() << id << parentId;
// //     if (parentType!=Zanshin::Category && parentType!=Zanshin::CategoryRoot) { //TODO shouldn't be necessary
// //         return false;
// //     }
//     IdList parents = mStructure->getParents(id);
//     parents << parentId;
//     mStructure->moveNode(id, parents);
//     return true;
// }
// 
// 
// bool PimItemRelationInterface::unlink(const Akonadi::Item& item, const QModelIndex& parent)
// {
//     Id parentId = toId(parent);
//     kDebug() << item.id() << parentId;
//     if (!item.isValid()) {
//         return false;
//     }
//     Id id = mStructure->getItemId(item);
//     IdList parents = mStructure->getParents(id);
//     parents.removeAll(parentId);
//     mStructure->moveNode(id, parents);
//     return true;
// }
// 
// bool PimItemRelationInterface::rename(const QModelIndex& node, const QString& name)
// {
//     mStructure->renameNode(toId(node), name);
//     return true;
// }
// 
// 
// // IdList PimItemRelationInterface::getParents(const Akonadi::Item& item) const
// // {
// //     if (!item.isValid()) {
// //         return IdList();
// //     }
// //     Id id = mStructure->getItemId(item);
// //     return mStructure->getParents(id);
// // }
// // 
// // IdList PimItemRelationInterface::getAncestors(Id id) const
// // {
// //     IdList ancestors;
// //     foreach (Id parent, mStructure->getParents(id)) {
// //         kDebug() << mStructure->getPath(parent) << parent;
// //         ancestors << parent;
// //         ancestors << getAncestors(parent);
// //     }
// //     return ancestors;
// // }
// // 
// // IdList PimItemRelationInterface::getAncestors(const Akonadi::Item& item) const
// // {
// //     if (!item.isValid()) {
// //         return IdList();
// //     }
// //     IdList ancestors;
// //     Id id = mStructure->getItemId(item);
// //     kDebug() << "ancestors of " << mStructure->getPath(id) << id;
// //     return getAncestors(id);
// // }
//
// const char *todoScheme = "Todo";
// static PimNode nodeUri(PimItemStructureInterface::NodeType type, const QString &uid)
// {
//     PimNode uri;
//     switch (type) {
//         case PimItemStructureInterface::Todo:
//             uri.setScheme(todoScheme);
//             break;
//     }
//     uri.setPath(uid);
//     return uri;
// }
// 
// static PimItemStructureInterface::NodeType getNodeType(const PimNode &uri)
// {
//     if (uri.scheme() == todoScheme) {
//         return PimItemStructureInterface::Todo;
//     }
//     return PimItemStructureInterface::Invalid;
// }

ProjectStructureInterface::ProjectStructureInterface()
{

}

bool ProjectStructureInterface::moveTo(const PimNode& node, const PimNode& parent)
{
    PimNode::NodeType nodeType = node.type;
    PimNode::NodeType parentType = parent.type;
//    const Akonadi::Item item = node.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();

    if (nodeType == PimNode::Invalid || parentType == PimNode::Invalid) {
        return false;
    }

    IdList parents;
    parents << mStructure->getId(parent.uid);
    Id nodeId = mStructure->getItemId(node.item);
    mStructure->moveNode(nodeId, parents);
    return true;

//     if ((itemType == Zanshin::StandardTodo && parentType == Zanshin::StandardTodo)
//      || (itemType == Zanshin::ProjectTodo && parentType == Zanshin::StandardTodo)
//      || (itemType == Zanshin::Collection && parentType == Zanshin::ProjectTodo)
//      || (itemType == Zanshin::Collection && parentType == Zanshin::StandardTodo)) {
//          return false;
//     }

//     const Akonadi::Item parentItem = parent.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
//     QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(parentItem));
//     Q_ASSERT(pimitem);
//     Akonadi::Collection parentCollection;
// 
//     return TodoHelpers::moveTodoToProject(item, pimitem->getUid(), parentType, parentCollection);
}




