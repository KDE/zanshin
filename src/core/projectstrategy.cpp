/*
    This file is part of Zanshin Todo.

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


#include "globaldefs.h"
#include "core/projectstrategy.h"
#include "core/incidenceitem.h"
#include "core/pimitemmodel.h"
#include "core/pimitemservices.h"
#include "reparentingmodel/todonode.h"
#include "reparentingmodel/reparentingmodel.h"
#include "todohelpers.h"
#include <KLocalizedString>
#include <KIcon>
#include <QMimeData>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <KUrl>

ProjectStrategy::ProjectStrategy(ProjectStructureCache *structure)
:   ReparentingStrategy(),
    mInbox(1),
    mRelations(structure)
{
    PimItemServices::projectInstance().setRelationsStructure(mRelations.data());
    mReparentOnRemoval = false;
    connect(mRelations.data(), SIGNAL(nodeRemoved(Id)), this, SLOT(doRemoveNode(Id)));
    connect(mRelations.data(), SIGNAL(parentsChanged(Id,IdList)), this, SLOT(doChangeParents(Id, IdList)));
}

void ProjectStrategy::init()
{
    ReparentingStrategy::init();
    //FIXME we should be setting this earlier, here the inserted signals have already been emitted

    QList<TodoNode*> nodes = createNode(mInbox, IdList(), "Inbox");
    Q_ASSERT(nodes.size() == 1);
    TodoNode *node = nodes.first();
    node->setData(i18n("Inbox"), 0, Qt::DisplayRole);
    node->setData(KIcon("mail-folder-inbox"), 0, Qt::DecorationRole);
    node->setRowData(Zanshin::Inbox, Zanshin::ItemTypeRole);
}

static Id translateFrom(Id id)
{
    //TODO proper id mapping
    return id+10;
}

static Id translateTo(Id id)
{
    //TODO proper id mapping
    return id-10;
}

static IdList translateFrom(IdList l)
{
    IdList list;
    foreach (Id id, l) {
        list << translateFrom(id);
    }
    return list;
}

void ProjectStrategy::doRemoveNode(Id id)
{
    kDebug() << id;
    //FIXME
//     ReparentingStrategy::removeNode(translateFrom(id));
}

void ProjectStrategy::doChangeParents(Id id, IdList parents)
{
    IdList p;
    if (parents.isEmpty()) {
        bool isProject = mRelations->hasChildren(id);
        if (isProject) {
            const Akonadi::Collection col = getParentCollection(translateFrom(id));
            if (col.isValid()) {
                p << translateFrom(mRelations->addCollection(col));
            }
        } else {
            p << mInbox;
        }
    } else {
        p << translateFrom(parents);
    }
    ReparentingStrategy::updateParents(translateFrom(id), p);
}

Id ProjectStrategy::getId(const QModelIndex &sourceChildIndex)
{
    Zanshin::ItemType type = (Zanshin::ItemType) sourceChildIndex.data(Zanshin::ItemTypeRole).toInt();
    if (type==Zanshin::Collection) {
        return translateFrom(mRelations->addCollection(sourceChildIndex.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>()));
    }
//     kDebug() << sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>().url() << sourceChildIndex << type;
    const Akonadi::Item &item = sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    Q_ASSERT(item.isValid());
    Id id = mRelations->addItem(item);
    if (id < 0) {
        return -1;
    }
    return translateFrom(id);
}

bool ProjectStrategy::isProject(Id id, Zanshin::ItemType itemType) const
{
    if (itemType == Zanshin::ProjectTodo
    || ((itemType == Zanshin::StandardTodo) && mRelations->hasChildren(translateTo(id)))) {
        return true;
    }
    return false;
}

IdList ProjectStrategy::getParents(const QModelIndex &sourceChildIndex, const IdList &ignore)
{
    Q_ASSERT(sourceChildIndex.isValid());
//     kDebug() << sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>().url();
//     kDebug() << sourceChildIndex.data().toString();
//     mRelations->printCache();
    IdList parents;
    Zanshin::ItemType type = (Zanshin::ItemType) sourceChildIndex.data(Zanshin::ItemTypeRole).toInt();
    if (type==Zanshin::Collection) {
        const QModelIndex &parent = sourceChildIndex.parent();
        if (parent.isValid()) {
            return IdList() << getId(parent);
        }
        return IdList();
    }

    const Akonadi::Item &item = sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    Id id = mRelations->getItemId(item);
    parents = translateFrom(mRelations->getParents(id));
    bool isNote = (sourceChildIndex.data(PimItemModel::ItemTypeRole).toInt() == PimItem::Note);
    if (parents.isEmpty() || isNote) {
        if (!isProject(translateFrom(id), type) && !isNote) {
            return IdList() << mInbox;
        }
        const QModelIndex &parent = sourceChildIndex.parent();
        if (parent.isValid()) {
            parents << getId(parent);
        }
    }
    Q_ASSERT(!parents.contains(translateFrom(id)));

    foreach(Id i, ignore) {
        parents.removeAll(i);
    }
//     kDebug() << id << parents;
    checkParents(parents);
    return parents;
}

void ProjectStrategy::onNodeRemoval(const Id& changed)
{
    IdList parents = translateFrom(mRelations->getParents(translateTo(changed)));
//     kDebug() << changed << parents;
    mRelations->removeNode(translateTo(changed));
    checkParents(parents);
}

void ProjectStrategy::checkParents(const IdList &parentsToCheck)
{
    foreach(Id id, parentsToCheck) {
        //Because we may have just removed a project
        ReparentingStrategy::updateParents(id);
    }
}

void ProjectStrategy::reset()
{
    ReparentingStrategy::reset();
}

Qt::ItemFlags ProjectStrategy::flags(const QModelIndex& index, Qt::ItemFlags existingFlags)
{
    Zanshin::ItemType type = (Zanshin::ItemType)index.data(Zanshin::ItemTypeRole).toInt();

    if (type == Zanshin::Inbox) {
        return Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    }

    Akonadi::Collection collection;
    if (type==Zanshin::Collection) {
        collection = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    } else if (type == Zanshin::ProjectTodo) { //isProject(getId(index), type) FIXME for some reason the item is invalid in getid
        // We use ParentCollectionRole instead of Akonadi::Item::parentCollection() because the
        // information about the rights is not valid on retrieved items.
        collection = index.data(Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
    }

    if (!(collection.rights() & Akonadi::Collection::CanCreateItem)) {
        existingFlags &= ~Qt::ItemIsDropEnabled;
    } else {
        existingFlags |= Qt::ItemIsDropEnabled;
    }

    return existingFlags;
}

Qt::DropActions ProjectStrategy::supportedDropActions() const
{
    return Qt::MoveAction/*|Qt::LinkAction*/;
}

bool ProjectStrategy::onDropMimeData(Id id, const QMimeData* mimeData, Qt::DropAction action)
{
    if (action != Qt::MoveAction || !KUrl::List::canDecode(mimeData)) {
        return false;
    }

    KUrl::List urls = KUrl::List::fromMimeData(mimeData);

    Akonadi::Collection collection;
    Zanshin::ItemType parentType = (Zanshin::ItemType)getData(id, Zanshin::ItemTypeRole).toInt();
    if (parentType == Zanshin::Collection) {
        collection = getData(id, Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    } else {
        const Akonadi::Item parentItem = getData(id, Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
        collection = parentItem.parentCollection();
    }

    QString parentUid = getData(id, Zanshin::UidRole).toString();

    foreach (const KUrl &url, urls) {
        const Akonadi::Item urlItem = Akonadi::Item::fromUrl(url);
        //TODO make sure we never get here during testing (although we normally shouldn't anyways
        if (urlItem.isValid()) {
            //TODO replace by getData/setData?
            Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(urlItem);
            job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
            job->fetchScope().fetchFullPayload();
            if ( !job->exec() ) {
                continue;
            }
            Q_ASSERT(job->items().size()==1);
            Akonadi::Item item = job->items().first();
            Q_ASSERT(item.isValid());
            if (PimItem::itemType(item) == PimItem::Todo) {
                return TodoHelpers::moveTodoToProject(item, parentUid, parentType, collection);
            } else if (PimItem::itemType(item) == PimItem::Note) {
                TodoHelpers::moveToProject(item, parentUid);
                setData(id, QVariant::fromValue<Akonadi::Item>(item), Akonadi::EntityTreeModel::ItemRole);
                return true;
            }
        }
    }

    return false;
}

QVariant ProjectStrategy::data(Id id, int column, int role, bool &forward) const
{
    //We simply override the todometadatamodel data for todos with children (which are also projects)
    const Zanshin::ItemType itemType = static_cast<Zanshin::ItemType>(getData(id, Zanshin::ItemTypeRole).toInt());
    bool project = isProject(id, itemType);

    switch (role) {
        case Zanshin::ItemTypeRole: {
            if (id == mInbox) {
                return Zanshin::Inbox;
            }
            if (project) {
                return Zanshin::ProjectTodo;
            }
            return itemType;
        }
        case Qt::CheckStateRole:
            if (project) {
                forward = false;
                return QVariant();
            }
            break;
        case Qt::DecorationRole:
            if (project && !column) {
                return KIcon("view-pim-tasks");
            }
            break;
    }
    return QVariant();
}
