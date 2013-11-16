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

#include "pimitemservices.h"

#include <QtCore/QAbstractItemModel>

#include <KDE/Akonadi/ItemModifyJob>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/TransactionSequence>
#include <Akonadi/ItemDeleteJob>
#include <KDE/KCalCore/Todo>
#include <KDE/KDebug>
#include <KDE/KGlobal>
#include <KDE/KLocale>
#include <KDE/KMessageBox>
#include <KUrl>

#include "datastoreinterface.h"
#include "todohelpers.h"
#include "noteitem.h"
#include "settings.h"
#include "pimitemstructurecache.h"

K_GLOBAL_STATIC(PimItemRelationInterface, s_contextManager)
K_GLOBAL_STATIC(PimItemRelationInterface, s_topicManager)
K_GLOBAL_STATIC(ProjectStructureInterface, s_projectManager)

PimItemIndex PimItemServices::fromIndex(const QModelIndex &index)
{
     Zanshin::ItemType itemType = (Zanshin::ItemType)index.data(Zanshin::ItemTypeRole).toInt();
     kDebug() << index << itemType;
     switch (itemType) {
         case Zanshin::Collection: {
             PimItemIndex node(PimItem::Collection);
             node.collection = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
             return node;
         }
         case Zanshin::ProjectTodo: {
             PimItemIndex node (PimItem::Project);
             node.item = index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
             node.uid = index.data(Zanshin::UidRole).toString();
             return node;
         }
         case Zanshin::Context: {
             PimItemIndex node (PimItem::Context);
             node.relationId = index.data(Zanshin::RelationIdRole).value<Id>();
             node.uid = index.data(Zanshin::UidRole).toString();
             return node;
         }
         case Zanshin::Topic: {
             PimItemIndex node (PimItem::Topic);
             node.relationId = index.data(Zanshin::RelationIdRole).value<Id>();
             node.uid = index.data(Zanshin::UidRole).toString();
             if (node.uid.isEmpty()) {
                 kWarning() << "empty uid";
             }
             return node;
         }
         case Zanshin::StandardTodo: {
             PimItemIndex node (PimItem::Todo);
             node.item = index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
             node.uid = index.data(Zanshin::UidRole).toString();
             return node;
         }
         case Zanshin::Inbox:
         case Zanshin::TopicRoot:
         case Zanshin::ContextRoot:
             return PimItemIndex(PimItem::NoType);
         default:
             kWarning() << "unhandled type: " << itemType;
     }
     Q_ASSERT(false);
     return PimItemIndex(PimItem::NoType);
}

// PimNode PimItemServices::projectNode(const Akonadi::Item& )
// {
// 
// }
// 
// PimNode PimItemServices::contextNode(Id )
// {
// 
// }
// 
// PimNode PimItemServices::topicNode(Id )
// {
// 
// }
// 
// PimNode PimItemServices::todoNode(const Akonadi::Item& )
// {
// 
// }
// 
// PimNode PimItemServices::noteNode(const Akonadi::Item& )
// {
// 
// }
// 
// PimNode PimItemServices::collectionNode(const Akonadi::Collection& )
// {
// 
// }
// 

PimItemTreeNode PimItemServices::getNode(const PimItemIndex &node) const
{
    if( node.relationId >= 0 ) {
        return mStructure->getNode(node.relationId);
    }
    Q_ASSERT(!node.uid.isEmpty());
    return PimItemTreeNode(node.uid.toLatin1());
}

PimItemServices &PimItemServices::getInstance(PimItemRelation::Type type) {
    switch (type) {
        case PimItemRelation::Project:
            return projectInstance();
        case PimItemRelation::Context:
            return contextInstance();
        case PimItemRelation::Topic:
            return topicInstance();
        case PimItemRelation::Invalid:
            kWarning() << "invalid relationtype";
    }
    Q_ASSERT(false);
    return projectInstance();
}

static PimItemRelation::Type getRelationType(PimItem::ItemType parentType)
{
    switch (parentType) {
        case PimItem::Project:
            return PimItemRelation::Project;
        case PimItem::Context:
            return PimItemRelation::Context;
        case PimItem::Topic:
            return PimItemRelation::Topic;
        default:
            return PimItemRelation::Invalid;
    }
}

static bool isVirtualType(PimItem::ItemType type)
{
    return (type == PimItem::Context || type == PimItem::Topic);
}

void PimItemServices::create(PimItem::ItemType type, const QString& name, const QList< PimItemIndex >& parents, const Akonadi::Collection& col)
{
    Akonadi::Collection collection = col;
    if (!collection.isValid() && !isVirtualType(type)) {
        switch (type) {
            case PimItem::Project:
            case PimItem::Todo:
                collection = Settings::instance().defaultTodoCollection();
                break;
            case PimItem::Note:
                collection = Settings::instance().defaultNoteCollection();
                break;
            default:
                kWarning() << "unhandled type: " << type;
        }
        if (!collection.isValid()) {
            kWarning() << "no valid collection to create item";
            return;
        }
    }
    QList<PimItemRelation> relations;
    if (!parents.isEmpty()) {
        const PimItemIndex parent = parents.first();
        const PimItemRelation::Type relationType = getRelationType(parent.type);
        if (relationType != PimItemRelation::Invalid) {
            relations << PimItemRelation(relationType, QList<PimItemTreeNode>() << getInstance(relationType).getNode(parent));
        }
    }
 
    switch (type) {
        case PimItem::Project:
            kDebug() << "adding project: " << name << collection.url().url();
            TodoHelpers::addTodo(name, relations, collection, true);
            break;
        case PimItem::Todo: {
            kDebug() << "adding todo: " << name << collection.url().url();
            TodoHelpers::addTodo(name, relations, collection, false);
            break;
        }
        case PimItem::Note: {
            NoteItem note;
            note.setTitle(name);
            note.setRelations(relations);
            new Akonadi::ItemCreateJob(note.getItem(), collection);
            break;
        }
        case PimItem::Context:
            PimItemServices::contextInstance().add(name, parents);
            break;
        case PimItem::Topic:
            PimItemServices::topicInstance().add(name, parents);
            break;
        default:
            Q_ASSERT(0);
    }
}

void PimItemServices::remove(const PimItemIndex& node, QWidget *parent)
{
    switch (node.type) {
        case PimItem::Project:
            PimItemServices::projectInstance().remove(node, parent);
            break;
        case PimItem::Todo:
        case PimItem::Note:
            new Akonadi::ItemDeleteJob(node.item);
            break;
        case PimItem::Context:
            PimItemServices::contextInstance().remove(node, parent);
            break;
        case PimItem::Topic:
            PimItemServices::topicInstance().remove(node, parent);
            break;
        default:
            Q_ASSERT(0);
    }
}

void PimItemServices::remove(const QList< PimItemIndex >& nodes, QWidget *parent)
{
    foreach(const PimItemIndex &node, nodes) {
        remove(node, parent);
    }
}

void PimItemServices::moveTo(const PimItemIndex &node, const PimItemIndex &parent)
{
    switch (parent.type) {
    case PimItem::Project:
        switch (node.type) {
        case PimItem::Project:
        case PimItem::Todo:
        case PimItem::Note:
            projectInstance().moveTo(node, parent);
            break;
        default:
            qFatal("Unsupported move operation");
            break;
        }
        break;
    default:
        qFatal("Unsupported move operation");
        break;
    }
}

void PimItemServices::linkTo(const PimItemIndex& /*node*/, const PimItemIndex& /*parent*/)
{

}

void PimItemServices::unlink(const PimItemIndex& /*node*/, const PimItemIndex& /*parent*/)
{

}

void PimItemServices::rename(const PimItemIndex& /*node*/, const QString& /*name*/)
{

}


PimItemRelationInterface &PimItemServices::contextInstance()
{
    Q_ASSERT(s_contextManager);
    return *s_contextManager;
}

PimItemRelationInterface &PimItemServices::topicInstance()
{
    Q_ASSERT(s_topicManager);
    return *s_topicManager;
}

ProjectStructureInterface& PimItemServices::projectInstance()
{
    Q_ASSERT(s_projectManager);
    return *s_projectManager;
}

void PimItemServices::setRelationsStructure(PimItemRelationCache *s)
{
    mStructure = s;
}




PimItemRelationInterface::PimItemRelationInterface()
{
}

PimItemRelationInterface::~PimItemRelationInterface()
{
}

// static Id toId(const PimNode &index)
// {
//     return index.relationId;
// }

static IdList toId(const QList<PimItemIndex> &list)
{
    IdList parentIds;
    foreach (const PimItemIndex &index, list) {
        parentIds << index.relationId;
    }
    return parentIds;
}

void PimItemRelationInterface::add(const QString& name, const QList<PimItemIndex>& parents)
{
    kDebug() << name << toId(parents);
    mStructure->addNode(name, toId(parents));
}

// bool PimItemRelationInterface::remove(QWidget* widget, const QModelIndexList& relations)
// {
//     IdList relationIds = toId(relations);
//     if (widget) {
//         QStringList contextList;
//         foreach (Id context, relationIds) {
//             contextList << mStructure->getName(context);
//         }
//         QString contextName = contextList.join(", ");
//         kDebug() << relationIds << contextList;
//         QString title;
//         QString text;
//         //TODO adjust Context/Topic
//         if (relationIds.size() > 1) {
//             text = i18n("Do you really want to delete the context '%1'? All actions won't be associated to this context anymore.", contextName);
//             title = i18n("Delete Context");
//         } else {
//             text = i18n("Do you really want to delete the contexts '%1'? All actions won't be associated to those contexts anymore.", contextName);
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
//     if (parentType!=Zanshin::Context && parentType!=Zanshin::ContextRoot) { //TODO shouldn't be necessary
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
// //     if (parentType!=Zanshin::Context && parentType!=Zanshin::ContextRoot) { //TODO shouldn't be necessary
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
// static PimNode nodeUri(PimItemServices::NodeType type, const QString &uid)
// {
//     PimNode uri;
//     switch (type) {
//         case PimItemServices::Todo:
//             uri.setScheme(todoScheme);
//             break;
//     }
//     uri.setPath(uid);
//     return uri;
// }
// 
// static PimItemServices::NodeType getNodeType(const PimNode &uri)
// {
//     if (uri.scheme() == todoScheme) {
//         return PimItemServices::Todo;
//     }
//     return PimItemServices::Invalid;
// }

ProjectStructureInterface::ProjectStructureInterface()
{

}

bool ProjectStructureInterface::hasChildren(const QString &uid)
{
    if (!mStructure) return false;
    Id nodeId = mStructure->getId(uid.toLatin1());
    return static_cast<ProjectStructureCache*>(mStructure.data())->hasChildren(nodeId);
}

bool ProjectStructureInterface::moveTo(const PimItemIndex& node, const PimItemIndex& parent)
{
    PimItem::ItemType nodeType = node.type;
    PimItem::ItemType parentType = parent.type;
//    const Akonadi::Item item = node.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();

    if (nodeType == PimItem::NoType || parentType == PimItem::NoType) {
        return false;
    }

    DataStoreInterface::instance().moveTodoToProject(node, parent);

    IdList parents;
    if (parentType != PimItem::Inbox) {
        parents << mStructure->getId(parent.uid.toLatin1());
    }
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
//     QScopedPointer<PimItem> pimitem(PimItemUtils::getItem(parentItem));
//     Q_ASSERT(pimitem);
//     Akonadi::Collection parentCollection;
// 
//     return TodoHelpers::moveTodoToProject(item, pimitem->getUid(), parentType, parentCollection);
}

void ProjectStructureInterface::remove(const QList< PimItemIndex >& nodes, QWidget *parent)
{
    Q_ASSERT(mStructure);
    if (nodes.isEmpty()) {
        return;
    }

    bool canRemove = true;
    QString summary;
    IdList projectList;
    if (nodes.size() > 1) {
//         QStringList projectList;
//         foreach (QModelIndex project, projects) {
//             projectList << project.data().toString();
//         }
//         summary = projectList.join(", ");
    } else {
//         QModelIndexList children = projects[0].data(Zanshin::ChildIndexesRole).value<QModelIndexList>();
//         if (!children.isEmpty()) {
//             summary = projects[0].data().toString();
//         }
    }

    if (!summary.isEmpty()) {
        QString title;
        QString text;

//         if (projects.size() > 1) {
//             title = i18n("Delete Projects");
//             text = i18n("Do you really want to delete the projects '%1', with all its actions?", summary);
//         } else {
            title = i18n("Delete Project");
            text = i18n("Do you really want to delete the project '%1', with all its actions?", summary);
//         }

        int button = KMessageBox::questionYesNo(parent, text, title);
        canRemove = (button==KMessageBox::Yes);
    }

    if (!canRemove) return;

    Akonadi::TransactionSequence *sequence = new Akonadi::TransactionSequence();
    foreach (const PimItemIndex &node, nodes) {
        Q_ASSERT(node.item.isValid());
        Id id = mStructure->getItemId(node.item);
        IdList children = static_cast<ProjectStructureCache*>(mStructure.data())->getChildren(id);
        foreach (Id child, children) {
            Akonadi::Item item(static_cast<ProjectStructureCache*>(mStructure.data())->itemId(child));
            kDebug() << "remove " << item.id();
            new Akonadi::ItemDeleteJob(item, sequence);
        }
        kDebug() << "remove " << node.item.id();
        new Akonadi::ItemDeleteJob(node.item, sequence);
    }
    sequence->start();
}

void ProjectStructureInterface::remove(const PimItemIndex& node, QWidget *parent)
{
    QList<PimItemIndex> projects;
    projects << node;
    return remove(projects, parent);
}
