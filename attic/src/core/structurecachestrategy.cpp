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

#include "structurecachestrategy.h"
#include <KIcon>
#include <KUrl>
#include <KLocalizedString>
#include <QMimeData>

#include "reparentingmodel/reparentingmodel.h"
#include "reparentingmodel/todonode.h"
#include "core/pimitemservices.h"
#include "core/pimitemstructurecache.h"
#include "todohelpers.h"

StructureCacheStrategy::StructureCacheStrategy(PimItemRelation::Type type)
:   ReparentingStrategy(),
    mInbox(1),
    mRoot(2),
    mRelations(new PimItemStructureCache(type)),
    mType(type)
{
    switch (type) {
        case PimItemRelation::Context:
            mReparentOnRemoval = true;
            PimItemServices::contextInstance().setRelationsStructure(static_cast<PimItemStructureCache*>(mRelations.data()));
            break;
        case PimItemRelation::Topic:
            mReparentOnRemoval = true;
            PimItemServices::topicInstance().setRelationsStructure(static_cast<PimItemStructureCache*>(mRelations.data()));
            break;
        default:
            qWarning() << "unhandled type: " << type;
            Q_ASSERT_X( false, "PimItemRelationStrategy constructor", "Unknown 'type' argument" );
    }
    connect(mRelations.data(), SIGNAL(virtualNodeAdded(Id, IdList, QString)), this, SLOT(createVirtualNode(Id, IdList, QString)));
    connect(mRelations.data(), SIGNAL(nodeRemoved(Id)), this, SLOT(doRemoveNode(Id)));
    connect(mRelations.data(), SIGNAL(parentsChanged(Id,IdList)), this, SLOT(doChangeParents(Id, IdList)));
    connect(mRelations.data(), SIGNAL(virtualNodeRenamed(Id,QString)), this, SLOT(doRenameParent(Id, QString)));
    connect(mRelations.data(), SIGNAL(updateItems(IdList)), this, SLOT(doUpdateItems(IdList)));
}

void StructureCacheStrategy::init()
{
    ReparentingStrategy::init();

    QString noRelation, noRelationTranslated, relation, relationTranslated;
    int rootType;

    if (mType == PimItemRelation::Context) {
        noRelation = "No Context";
        noRelationTranslated = i18n("No Context");
        relation = "Contexts";
        relationTranslated = i18n("Contexts");
        rootType = Zanshin::ContextRoot;
    } else {
        Q_ASSERT(mType == PimItemRelation::Topic);

        noRelation = "No Topic";
        noRelationTranslated = i18n("No Topic");
        relation = "Topics";
        relationTranslated = i18n("Topics");
        rootType = Zanshin::TopicRoot;
    }

    QList<TodoNode*> nodes = createNode(mInbox, IdList(), noRelation);
    Q_ASSERT(nodes.size() == 1);
    TodoNode *node = nodes.first();
    node->setData(noRelationTranslated, 0, Qt::DisplayRole);
    node->setData(KIcon("mail-folder-inbox"), 0, Qt::DecorationRole);
    node->setRowData(Zanshin::Inbox, Zanshin::ItemTypeRole);

    QList<TodoNode*> nodes2 = createNode(mRoot, IdList(), relation);
    Q_ASSERT(nodes2.size() == 1);
    TodoNode *node2 = nodes2.first();
    node2->setData(relationTranslated, 0, Qt::DisplayRole);
    node2->setData(KIcon("document-multiple"), 0, Qt::DecorationRole);
    node2->setRowData(rootType, Zanshin::ItemTypeRole);
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

bool isIgnored(const QModelIndex &sourceChildIndex)
{
    Zanshin::ItemType type = (Zanshin::ItemType) sourceChildIndex.data(Zanshin::ItemTypeRole).toInt();
    if (type!=Zanshin::StandardTodo /*&& type!=Zanshin::ProjectTodo*/) { //Filter all other items
        return true;
    }
    return false;
}

Id StructureCacheStrategy::getId(const QModelIndex &sourceChildIndex)
{
//     kDebug() << sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>().id();
    if (isIgnored(sourceChildIndex)) { //Filter all other items
        return -1;
    }
    Id id = mRelations->addItem(sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>());
    if (id < 0) {
        return -1;
    }
    return translateFrom(id);
}

IdList StructureCacheStrategy::getParents(const QModelIndex &sourceChildIndex, const IdList& ignore)
{
//     kDebug() << sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>().id();
    if (isIgnored(sourceChildIndex)) {
        return IdList();
    }
    Q_ASSERT(sourceChildIndex.data(Zanshin::ItemTypeRole).toInt()==Zanshin::StandardTodo /*|| sourceChildIndex.data(Zanshin::ItemTypeRole).toInt()==Zanshin::ProjectTodo*/);

    IdList parents = translateFrom(mRelations->getParents(mRelations->getItemId(sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>())));

    foreach(Id i, ignore) {
        parents.removeAll(i);
    }
    if (parents.isEmpty()) {
        return IdList() << mInbox;
    }
    return parents;
}

void StructureCacheStrategy::createVirtualNode(Id id, IdList parents, const QString& name)
{
    IdList p = translateFrom(parents);
    if (p.isEmpty()) {
        p << mRoot;
    }
    createNode(translateFrom(id), p, name);
}

void StructureCacheStrategy::doRemoveNode(Id id)
{
    kDebug() << id;
    ReparentingStrategy::removeNode(translateFrom(id));
}

void StructureCacheStrategy::doChangeParents(Id id, IdList parents)
{
    ReparentingStrategy::updateParents(translateFrom(id), translateFrom(parents));
}

void StructureCacheStrategy::doRenameParent(Id id, const QString& name)
{
    ReparentingStrategy::renameNode(translateFrom(id), name);
}

void StructureCacheStrategy::doUpdateItems(const IdList &itemsToUpdate)
{
    kDebug() << itemsToUpdate;
    foreach (Id id , itemsToUpdate) {
        Akonadi::Item item = getData(translateFrom(id), Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
        kDebug() << id << item.id();
        if (!item.isValid()) {
            kWarning() << "could not find item " << id;
            continue;
        }
        Q_ASSERT(item.isValid());
        mRelations->updateRelationTree(item);
        //TODO Transaction style updating for all items, but while making sure it's never being executed during testing
        setData(id, QVariant::fromValue<Akonadi::Item>(item), Akonadi::EntityTreeModel::ItemRole);
    }
}

void StructureCacheStrategy::setNodeData(TodoNode* node, Id id)
{
//     kDebug() << id;
    if (id == mInbox || id == mRoot) {
        return;
    }

    const QString &contextName = mRelations->getName(translateTo(id));
    node->setData(contextName, 0, Qt::DisplayRole);
    node->setData(contextName, 0, Qt::EditRole);
    node->setData(KIcon("view-pim-notes"), 0, Qt::DecorationRole);
    if (mType == PimItemRelation::Context) {
        node->setRowData(Zanshin::Context, Zanshin::ItemTypeRole); //TODO relation role
    } else {
        Q_ASSERT(mType == PimItemRelation::Topic);
        node->setRowData(Zanshin::Topic, Zanshin::ItemTypeRole);
    }
}

bool StructureCacheStrategy::reparentOnParentRemoval(Id child) const
{
//     kDebug() << child;
    if (mRelations->isVirtual(translateTo(child))) {
        return false;
    }
    return true;
}

void StructureCacheStrategy::onNodeRemoval(const Id& changed)
{
    kDebug() << changed;
    mRelations->removeNode(translateTo(changed));
}

QMimeData* StructureCacheStrategy::mimeData(const QModelIndexList& indexes) const
{
    QStringList ids;
    foreach (const QModelIndex &proxyIndex, indexes) {
        ids << QString::number(proxyIndex.data(Zanshin::RelationIdRole).toLongLong());
    }

    if (!ids.isEmpty()) {
        QMimeData *mimeData = new QMimeData();
        QByteArray contexts = ids.join(",").toUtf8();
        mimeData->setData("application/x-vnd.zanshin.relationid", contexts);
        return mimeData;
    }
    return 0;
}

QStringList StructureCacheStrategy::mimeTypes()
{
    return QStringList() << "application/x-vnd.zanshin.relationid";
}

Qt::DropActions StructureCacheStrategy::supportedDropActions() const
{
    return /*Qt::MoveAction|*/Qt::LinkAction;
}

Qt::ItemFlags StructureCacheStrategy::flags(const QModelIndex& index, Qt::ItemFlags flags)
{
    Zanshin::ItemType type = (Zanshin::ItemType) index.data(Zanshin::ItemTypeRole).toInt();
    if (type == Zanshin::Inbox || type == Zanshin::ContextRoot) {
        return Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    }
    return flags | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
}

bool StructureCacheStrategy::onDropMimeData(Id id, const QMimeData *mimeData, Qt::DropAction action)
{
    if (!KUrl::List::canDecode(mimeData) && !mimeData->hasFormat("application/x-vnd.zanshin.relationid")) {
        kWarning() << "invalid drop " << KUrl::List::canDecode(mimeData);
        return false;
    }

    Zanshin::ItemType parentType = (Zanshin::ItemType)getData(id, Zanshin::ItemTypeRole).toInt();
    if (parentType!=Zanshin::Context && parentType!=Zanshin::ContextRoot &&
        parentType!=Zanshin::Topic && parentType!=Zanshin::TopicRoot
    ) { //TODO Check if virtual instead (so it works also for topics)
        kWarning() << "not a context";
        return false;
    }
    if (action != Qt::MoveAction && action != Qt::LinkAction) {
        kWarning() << "action not supported";
        return false;
    }
    IdList sourceItems;
    if (KUrl::List::canDecode(mimeData)) {
        KUrl::List urls = KUrl::List::fromMimeData(mimeData);
//         kDebug() << urls;
        foreach (const KUrl &url, urls) {
            const Akonadi::Item urlItem = Akonadi::Item::fromUrl(url);
            if (!urlItem.isValid()) {
                qWarning() << "invalid item";
                continue;
            }
            sourceItems << mRelations->getItemId(urlItem);
        }
    } else {
        const QStringList sItems = QString(mimeData->data("application/x-vnd.zanshin.relationid")).split(",");
        foreach (const QString &source, sItems) {
            sourceItems << source.toLongLong();
        }
        //TODO if multiple nodes of a hierarchy are dropped, all nodes are added as direct child (the hierarchy is destroyed).
        //TODO move only dragged node and not all (remove a single context and add the new one from the parent list)
    }
    foreach (const Id &s, sourceItems) {
        if (action == Qt::MoveAction) {
            mRelations->moveNode(s, IdList() << translateTo(id));
        } else if (action == Qt::LinkAction) {
            mRelations->moveNode(s, IdList() << translateTo(id) << mRelations->getParents(s));
        }
    }
    return true;
}

bool StructureCacheStrategy::onSetData(Id id, const QVariant& value, int /*role*/, int /*column*/)
{
    Zanshin::ItemType type = (Zanshin::ItemType) getData(id, Zanshin::ItemTypeRole).toInt();
    kWarning() << id << type;
    if (type==Zanshin::Context) {
        kDebug() << value.toString();
        mRelations->renameNode(translateTo(id), value.toString());
        return true;
    }
    return false;
}

void StructureCacheStrategy::reset()
{
    ReparentingStrategy::reset();
}

QVariant StructureCacheStrategy::data(Id id, int /*column*/, int role, bool &/*forward*/) const
{
    const Id translatedId = translateTo(id);
    if (role == Zanshin::RelationIdRole) {
        return translatedId;
    }
    if (role == Zanshin::UidRole) {
        return QString(mRelations->getUid(translatedId));
    }
    return QVariant();
}
