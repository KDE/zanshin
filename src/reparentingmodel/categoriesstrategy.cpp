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


#include "categoriesstrategy.h"
#include <categorymanager.h>
#include <todohelpers.h>
#include <todonode.h>
#include <KIcon>
#include <KUrl>
#include <KLocalizedString>
#include <QMimeData>

#include "reparentingmodel.h"
#include <Akonadi/ItemModifyJob>

CategoriesStrategy::CategoriesStrategy()
:   ReparentingStrategy(),
    mInbox(1),
    mRoot(2),
    mRelations(new CategoriesStructure())
{
    CategoryManager::instance().setCategoriesStructure(static_cast<CategoriesStructure*>(mRelations.data()));
    mReparentOnRemoval = true;
    connect(mRelations.data(), SIGNAL(virtualNodeAdded(Id, IdList, QString)), this, SLOT(createVirtualNode(Id, IdList, QString)));
    connect(mRelations.data(), SIGNAL(nodeRemoved(Id)), this, SLOT(doRemoveNode(Id)));
    connect(mRelations.data(), SIGNAL(parentsChanged(Id,IdList)), this, SLOT(doChangeParents(Id, IdList)));
    connect(mRelations.data(), SIGNAL(virtualNodeRenamed(Id,QString)), this, SLOT(doRenameParent(Id, QString)));
    connect(mRelations.data(), SIGNAL(updateItems(IdList)), this, SLOT(doUpdateItems(IdList)));
}

void CategoriesStrategy::init()
{
    ReparentingStrategy::init();
    TodoNode *node = createNode(mInbox, IdList(), "No Context");
    node->setData(i18n("No Context"), 0, Qt::DisplayRole);
    node->setData(KIcon("mail-folder-inbox"), 0, Qt::DecorationRole);
    node->setRowData(Zanshin::Inbox, Zanshin::ItemTypeRole);

    TodoNode *node2 = createNode(mRoot, IdList(), "Contexts");
    node2->setData(i18n("Contexts"), 0, Qt::DisplayRole);
    node2->setData(KIcon("document-multiple"), 0, Qt::DecorationRole);
    node2->setRowData(Zanshin::CategoryRoot, Zanshin::ItemTypeRole);
}

Id translateFrom(Id id)
{
    //TODO proper id mapping
    return id+10;
}

Id translateTo(Id id)
{
    //TODO proper id mapping
    return id-10;
}

IdList translateFrom(IdList l)
{
    IdList list;
    foreach (Id id, l) {
        list << translateFrom(id);
    }
    return list;
}

Id CategoriesStrategy::getId(const QModelIndex &sourceChildIndex)
{
    Zanshin::ItemType type = (Zanshin::ItemType) sourceChildIndex.data(Zanshin::ItemTypeRole).toInt();
    if (type!=Zanshin::StandardTodo /*&& type!=Zanshin::ProjectTodo*/) { //Filter all other items
        return -1;
    }
    return translateFrom(mRelations->addItem(sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>()));
}

IdList CategoriesStrategy::getParents(const QModelIndex &sourceChildIndex, const IdList& ignore)
{
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

void CategoriesStrategy::createVirtualNode(Id id, IdList parents, const QString& name)
{
    IdList p = translateFrom(parents);
    if (p.isEmpty()) {
        p << mRoot;
    }
    createNode(translateFrom(id), p, name);
}

void CategoriesStrategy::doRemoveNode(Id id)
{
    kDebug() << id;
    ReparentingStrategy::removeNode(translateFrom(id));
}

void CategoriesStrategy::doChangeParents(Id id, IdList parents)
{
    ReparentingStrategy::updateParents(translateFrom(id), translateFrom(parents));
}

void CategoriesStrategy::doRenameParent(Id id, const QString& name)
{
    ReparentingStrategy::renameNode(translateFrom(id), name);
}

void CategoriesStrategy::doUpdateItems(const IdList &itemsToUpdate)
{
    kDebug() << itemsToUpdate;
    foreach (Id id , itemsToUpdate) {        
        Akonadi::Item item = getData(translateFrom(id), Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
        if (!item.isValid()) {
            kWarning() << "could not find item " << id;
            continue;
        }
        Q_ASSERT(item.isValid());
        mRelations->updateRelationTree(item);
        new Akonadi::ItemModifyJob(item, this);
    }
}


void CategoriesStrategy::setData(TodoNode* node, Id id)
{
    kDebug() << id;
    if (id == mInbox || id == mRoot) {
        return;
    }

    const QString &categoryName = mRelations->getName(translateTo(id));
    node->setData(categoryName, 0, Qt::DisplayRole);
    node->setData(categoryName, 0, Qt::EditRole);
    node->setData(KIcon("view-pim-notes"), 0, Qt::DecorationRole);
    node->setRowData(Zanshin::Category, Zanshin::ItemTypeRole);
}

bool CategoriesStrategy::reparentOnParentRemoval(Id child) const
{
//     kDebug() << child;
    if (mRelations->isVirtual(translateTo(child))) {
        return false;
    }
    return true;
}


void CategoriesStrategy::onNodeRemoval(const Id& changed)
{
    kDebug() << changed;
    mRelations->removeNode(translateTo(changed));
}


QMimeData* CategoriesStrategy::mimeData(const QModelIndexList& indexes) const
{
    QStringList ids;
    foreach (const QModelIndex &proxyIndex, indexes) {
        ids << QString::number(proxyIndex.data(Zanshin::RelationIdRole).toLongLong());
    }

    if (!ids.isEmpty()) {
        QMimeData *mimeData = new QMimeData();
        QByteArray categories = ids.join(",").toUtf8();
        mimeData->setData("application/x-vnd.zanshin.relationid", categories);
        return mimeData;
    }
    return 0;
}

QStringList CategoriesStrategy::mimeTypes()
{
    return QStringList() << "application/x-vnd.zanshin.relationid";
}

Qt::DropActions CategoriesStrategy::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::ItemFlags CategoriesStrategy::flags(const QModelIndex& index, Qt::ItemFlags flags)
{
    Zanshin::ItemType type = (Zanshin::ItemType) index.data(Zanshin::ItemTypeRole).toInt();
    if (type == Zanshin::Inbox || type == Zanshin::CategoryRoot) {
        return Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    }
    return flags | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
}

bool CategoriesStrategy::onDropMimeData(Id id, const QMimeData *mimeData, Qt::DropAction action)
{
    if (action != Qt::MoveAction || (!KUrl::List::canDecode(mimeData) && !mimeData->hasFormat("application/x-vnd.zanshin.relationid"))) {
        kDebug() << "invalid drop " << action << KUrl::List::canDecode(mimeData);
        return false;
    }
    
    Zanshin::ItemType parentType = (Zanshin::ItemType)getData(id, Zanshin::ItemTypeRole).toInt();
    if (parentType!=Zanshin::Category && parentType!=Zanshin::CategoryRoot) {
        kDebug() << "not a category";
        return false;
    }
    if (KUrl::List::canDecode(mimeData)) {
        KUrl::List urls = KUrl::List::fromMimeData(mimeData);
        kDebug() << urls;
        foreach (const KUrl &url, urls) {
            const Akonadi::Item urlItem = Akonadi::Item::fromUrl(url);
            if (!urlItem.isValid()) {
                qWarning() << "invalid item";
                continue;
            }
            mRelations->moveNode(mRelations->getItemId(urlItem), IdList() << translateTo(id));
        }
    } else {
        QStringList sourceItems = QString(mimeData->data("application/x-vnd.zanshin.relationid")).split(",");
        foreach (const QString &source, sourceItems) {
            Id s = source.toLongLong();
            mRelations->moveNode(s, IdList() << translateTo(id));
        }
        //TODO if multiple nodes of a hierarchy are dropped, all nodes are added as direct child (the hierarchy is destroyed).
        //TODO move only dragged node and not all (remove a single category and add the new one from the parent list)
    }
    return true;
}

bool CategoriesStrategy::onSetData(Id id, const QVariant& value, int role)
{
    Zanshin::ItemType type = (Zanshin::ItemType) getData(id, Zanshin::ItemTypeRole).toInt();
    if (type==Zanshin::Category) {
        mRelations->renameNode(translateTo(id), value.toString());
        return true;
    }
    return false;
}

void CategoriesStrategy::reset()
{
    ReparentingStrategy::reset();
}

QVariant CategoriesStrategy::data(Id id, int role) const
{
    if (role == Zanshin::RelationIdRole) {
        return translateTo(id);
    }
    return ReparentingStrategy::data(id, role);
}



