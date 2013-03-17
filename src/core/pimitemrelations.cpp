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


#include "pimitemrelations.h"
#include "pimitemfactory.h"
#include <quuid.h>
#include <KUrl>
#include <QDomElement>

PimItemRelation::PimItemRelation(PimItemRelation::Type t, const QList< PimItemTreeNode >& p)
:   type(t),
    parentNodes(p)
{

}

PimItemRelation::PimItemRelation()
:   type(Invalid)
{

}

PimItemTreeNode::PimItemTreeNode(const QByteArray &u, const QString &n, const QList<PimItemTreeNode> &p)
:   uid(u),
    name(n),
    parentNodes(p)
{

}
QDomDocument loadDocument(const QByteArray &xml)
{
    QString errorMsg;
    int errorLine, errorColumn;
    QDomDocument document;
    bool ok = document.setContent( xml, &errorMsg, &errorLine, &errorColumn );
    if ( !ok ) {
        kWarning() << xml;
        qWarning( "Error loading document: %s, line %d, column %d", qPrintable( errorMsg ), errorLine, errorColumn );
        return QDomDocument();
    }
    return document;
}


PimItemRelation::Type typeFromString(const QString &type)
{
    if (type == QLatin1String("Project")) {
        return PimItemRelation::Project;
    } else if (type == QLatin1String("Context")) {
        return PimItemRelation::Context;
    } else if (type == QLatin1String("Topic")) {
        return PimItemRelation::Topic;
    }
    qWarning() << type;
    Q_ASSERT(0);
    return PimItemRelation::Project;
}

QString typeToString(PimItemRelation::Type type)
{
    switch (type) {
        case PimItemRelation::Project:
            return QLatin1String("Project");
        case PimItemRelation::Context:
            return QLatin1String("Context");
        case PimItemRelation::Topic:
            return QLatin1String("Topic");
        default:
            qWarning() << "unhandled type" << type;
    }
    qWarning() << type;
    Q_ASSERT(0);
    return QString();
}


QDomDocument createXMLDocument()
{
    QDomDocument document;
    QString p = "version=\"1.0\" encoding=\"UTF-8\"";
    document.appendChild(document.createProcessingInstruction( "xml", p ) );
    return document;
}

void addElement(QDomElement &element, const QString &name, const QString &value)
{
    QDomElement e = element.ownerDocument().createElement( name );
    QDomText t = element.ownerDocument().createTextNode( value );
    e.appendChild( t );
    element.appendChild( e );
}

void addNodes(const QList < PimItemTreeNode > &nodes, QDomElement &element)
{
    foreach(const PimItemTreeNode &node, nodes) {
        if (node.uid.isEmpty()) {
            kWarning() << "Empty uid in relation, skipping. name: " << node.name;
            continue;
        }
        QDomElement e = element.ownerDocument().createElement( "tree" );
        addElement(e, "uid", node.uid);
        addElement(e, "name", node.name);
        addNodes(node.parentNodes, e);
        element.appendChild( e );
    }
}

PimItemTreeNode getTreeNode(QDomElement e)
{
  QString name;
  QString uid;
  QList<PimItemTreeNode> nodes;
  for ( QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      if (e.tagName() == "tree") {
        nodes.append(getTreeNode(n.toElement()));
      } else if (e.tagName() == "uid") {
        uid = e.text();
      } else if (e.tagName() == "name") {
        name = e.text();
      } else {
        kDebug() <<"Unknown element";
        Q_ASSERT(false);
      }
    } else {
      kDebug() << "Node is not an element";
      Q_ASSERT(false);
    }
  }
  return PimItemTreeNode(uid.toLatin1(), name, nodes);
}

PimItemRelation getRelation(QDomElement parent)
{
  QString type;
  QList<PimItemTreeNode> nodes;
  for ( QDomNode n = parent.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      if (e.tagName() == "tree") {
        const PimItemTreeNode node = getTreeNode(n.toElement());
        if (node.uid.isEmpty()) {
            kWarning() << "Found node with empty uid, skipping.";
            continue;
        }
        nodes.append(node);
      } else if (e.tagName() == "type") {
        type = e.text();
      } else {
        kDebug() <<"Unknown element";
        Q_ASSERT(false);
      }
    } else {
      kDebug() <<"Node is not an element";
      Q_ASSERT(false);
    }
  }
  return PimItemRelation(typeFromString(type), nodes);
} 

/*
void NoteMessageWrapper::NoteMessageWrapperPrivate::parseRelationsPart(KMime::Content *part)
{
  QDomDocument document = loadDocument(part);
  if (document.isNull()) {
    return;
  }
  QDomElement top = document.documentElement();
  if ( top.tagName() != "relations" ) {
    qWarning( "XML error: Top tag was %s instead of the expected relations",
              top.tagName().toAscii().data() );
    return;
  }

  for ( QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      relations.append(getRelation(e));
    } else {
      kDebug() <<"Node is not an element";
      Q_ASSERT(false);
    }

  }
}
*/


QString relationToXML(const PimItemRelation &rel)
{
    QDomDocument document = createXMLDocument();
    QDomElement element = document.createElement( "relations" );
    element.setAttribute( "version", "1.0" );
    QDomElement e = document.createElement( "relation" );
    addNodes(rel.parentNodes, e);
    addElement(e, "type", typeToString(rel.type));
    element.appendChild(e);
    document.appendChild(element);
    kDebug() << document.toString();
    return document.toString();
}

PimItemRelation relationFromXML(const QByteArray &xml)
{
    QDomDocument document = loadDocument(xml);
    if (document.isNull()) {
        return PimItemRelation();
    }
    QDomElement top = document.documentElement();
    if ( top.tagName() != "relations" ) {
        qWarning( "XML error: Top tag was %s instead of the expected relations",
                top.tagName().toAscii().data() );
        return PimItemRelation();
    }

    for ( QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling() ) {
        if ( n.isElement() ) {
            QDomElement e = n.toElement();
            return getRelation(e);
        } else {
            kDebug() <<"Node is not an element";
            Q_ASSERT(false);
        }

    }
    return PimItemRelation();
}

PimItemRelation removeDuplicates(const PimItemRelation &rel)
{
    if (rel.type != PimItemRelation::Project) {
        return rel;
    }
    QList<PimItemTreeNode> projects;
    QStringList existingProjects;
    foreach(const PimItemTreeNode &node, rel.parentNodes) {
        if (existingProjects.contains(node.uid)) {
            continue;
        }
        existingProjects << node.uid;
        projects << node;
    }
    return PimItemRelation(PimItemRelation::Project, projects);
}





TreeNode::TreeNode(const QString& n, const Id& i, const QList< TreeNode >& p)
:   name(n),
    id(i),
    parentNodes(p)
{
}

Relation::Relation(Id i, const QList< TreeNode >& p)
:   id(i),
    parentNodes(p)
{
}

Relation::Relation()
:   id(-1)
{
}




PimItemRelationCache::PimItemRelationCache()
:   QObject(),
    mIdCounter(1)
{

}


void PimItemRelationCache::mergeNode(const TreeNode &/*node*/)
{
}

Id PimItemRelationCache::addItem(const Akonadi::Item &item)
{
    Q_ASSERT(item.isValid());
    //TODO cache
    
//     kDebug() << pimitem->itemType();
    Id id = getOrCreateItemId(item);
    if (id < 0) {
        return -1;
    }
    const Relation &rel = getRelationTree(id, item);
    Q_ASSERT(rel.id == id);
//     qDebug() << " <<<<<<<<<<<<<<<<< " << item.url().url() << id << rel.id << rel.parentNodes.size();
    mParents.removeLeft(id);
    foreach (const TreeNode &node, rel.parentNodes) {
        Q_ASSERT(id != node.id);
        mParents.insert(id, node.id);
        mergeNode(node);
    }
//     kDebug() << item.id() << mParents.values(id);
    Q_ASSERT(mItemIdCache.contains(item.id()));
    return id;
}

Id PimItemRelationCache::getItemId(const Akonadi::Item &item) const
{
//     kDebug() << item.id();
//     qDebug() << "itemids: " << mItemIdCache;
//     qDebug() << "parents " << mParents;
//     qDebug() << "uids " << mUidMapping;
    Q_ASSERT(item.isValid());
    Q_ASSERT(mItemIdCache.contains(item.id()));
    return mItemIdCache.value(item.id());
}

Id PimItemRelationCache::getOrCreateItemId(const Akonadi::Item &item)
{
    Q_ASSERT(item.isValid());
    if (mItemIdCache.contains(item.id())) {
        return mItemIdCache.value(item.id());
    }
    Id id;
    PimItem::Ptr pimitem(PimItemFactory::getItem(item));
    Q_ASSERT (!pimitem.isNull());
    QByteArray uid = pimitem->getUid().toLatin1();
    if (uid.isEmpty()) {
        kWarning() << "empty uid: " << item.id();
        return -1;
    }
    Q_ASSERT(!uid.isEmpty());
    if (mUidMapping.contains(uid)) {
        id = mUidMapping.value(uid);
    } else {
        id = getNextId();
        mUidMapping.insert(uid, id);
    }
    mItemIdCache[item.id()] = id;
//    kDebug() << item.id() << id;
    return id;
}

bool PimItemRelationCache::isVirtual(Id id) const
{
    return !mItemIdCache.values().contains(id);
}

IdList values(Id key, const PimItemRelationCache::ParentMapping &map)
{
    IdList parentNodes;
    PimItemRelationCache::ParentMapping::left_const_iterator i = map.constFindLeft(key);
    while (i != map.leftConstEnd() && i.key() == key) {
        parentNodes << i.value();
        ++i;
    }
    return parentNodes;
}

IdList PimItemRelationCache::getParents(Id id)
{
    return values(id, mParents);
}

IdList keys(Id key, const PimItemRelationCache::ParentMapping &map)
{
    IdList parentNodes;
    PimItemRelationCache::ParentMapping::right_const_iterator i = map.constFindRight(key);
    while (i != map.rightConstEnd() && i.key() == key) {
        parentNodes << i.value();
        ++i;
    }
    return parentNodes;
}

IdList PimItemRelationCache::getChildNodes(Id id) const
{
    IdList result;
    const IdList &list = keys(id, mParents);
    result.append(list);
    foreach (Id child, list) {
        result.append(getChildNodes(child));
    }
    return result;
}

IdList PimItemRelationCache::getAffectedChildItems(Id id) const
{
    IdList itemList;
    IdList itemsToUpdate = getChildNodes(id);
//     kDebug() << itemsToUpdate;
    foreach (Id update, itemsToUpdate) {
        if (isVirtual(update)) {
            continue;
        }
        itemList << update;
    }
    return itemList;
}

void PimItemRelationCache::moveNode(Id id, IdList parents)
{
    kDebug() << id << parents;
    IdList itemList = getAffectedChildItems(id);
    if (!isVirtual(id)) {
        itemList << id;
    }

    mParents.removeLeft(id);
    foreach(Id parent, parents) {
        mParents.insert(id, parent);
    }
    rebuildCache();
    emit parentsChanged(id, parents);
    emit updateItems(itemList);
}

void PimItemRelationCache::removeNodeRecursive(Id id)
{
//     kDebug() << id;
    mParents.removeLeft(id);
//     if (mItemIdCache.values().contains(id)) {
//         mItemIdCache.remove(mItemIdCache.key(id));
//     }
    Q_ASSERT(!mParents.leftContains(id));

    const IdList &children = getChildNodes(id);
    foreach (Id child, children) {
        removeNodeRecursive(child);
    }
}

void PimItemRelationCache::removeNode(Id id)
{
    if (!mParents.leftContains(id)) {
        return;
    }
    const IdList &itemList = getAffectedChildItems(id);
//     kDebug() << id;
    removeNodeRecursive(id);

    rebuildCache();

    emit nodeRemoved(id);
    emit updateItems(itemList);
}

Id PimItemRelationCache::getNextId()
{
    return mIdCounter++;
}

void PimItemRelationCache::addUidMapping(const QByteArray& uid, Id id)
{
    Q_ASSERT(!uid.isEmpty());
    Q_ASSERT(!mUidMapping.contains(uid) || (getId(uid) == id));
    if (!mUidMapping.contains(uid)) {
        mUidMapping.insert(uid, id);
    }
}

Id PimItemRelationCache::getId(const QByteArray& uid) const
{
    return mUidMapping.value(uid);
}

QByteArray PimItemRelationCache::getUid(Id id) const
{
    //No empty uids in map
    Q_ASSERT(!mUidMapping.key(id).isEmpty() || !mUidMapping.values().contains(id));
    return mUidMapping.key(id);
}

Id PimItemRelationCache::getUidMapping(const QByteArray& uid)
{
    if (!mUidMapping.contains(uid)) {
        mUidMapping.insert(uid, getNextId());
    }
    return getId(uid);
}








VirtualRelationCache::VirtualRelationCache()
:   PimItemRelationCache()
{

}

void VirtualRelationCache::mergeNode(const TreeNode &node)
{
//     kDebug() << node.id << node.name;
    bool created = false;
    if (!mNames.contains(node.id)) {
        created = true;
    }
    if (mNames.value(node.id) != node.name || created) {
        mNames.insert(node.id, node.name);
        //TODO the names need some changing for projects as the name comes from the item itself and not one of its children
        if (!created && !node.name.isEmpty()) {
            emit virtualNodeRenamed(node.id, node.name);
        }
    }

    PimItemRelationCache::mergeNode(node);
    //TODO emit changes if changed
    mParents.removeLeft(node.id);
    foreach (const TreeNode &parentNode, node.parentNodes) {
        mParents.insert(node.id, parentNode.id);
        mergeNode(parentNode);
    }
    
    if (created) {
//         kDebug() << "created node " << node.id << mParents.values(node.id) << node.name;
        QString name = node.name;
        if (name.isEmpty()) {
            name = "noname";
        }
//         Q_ASSERT(!node.name.isEmpty());
        emit virtualNodeAdded(node.id, values(node.id, mParents), name);
    }
}

QString VirtualRelationCache::getName(Id id)
{
//     kDebug() << id << mNames.value(id);
//     Q_ASSERT(mNames.contains(id));
    return mNames.value(id);
}

void VirtualRelationCache::removeNodeRecursive(Id id)
{
    mNames.remove(id);
    Q_ASSERT(!mNames.contains(id));
    PimItemRelationCache::removeNodeRecursive(id);
}

void VirtualRelationCache::removeNode(Id id)
{
    if (!mParents.leftContains(id) && !mNames.contains(id)) {
        return;
    }
    PimItemRelationCache::removeNode(id);
}

void VirtualRelationCache::renameNode(Id id, const QString &name)
{
    if (name == mNames.value(id)) {
        return;
    }
    IdList itemList = getAffectedChildItems(id);
    if (!isVirtual(id)) {
        itemList << id;
    }
    mNames.insert(id, name);
    rebuildCache();
    emit virtualNodeRenamed(id, name);
    emit updateItems(itemList);
}



PimItemStructureCache::PimItemStructureCache(PimItemRelation::Type type)
:   VirtualRelationCache(),
    mType(type)
{

}

TreeNode PimItemStructureCache::createNode(const PimItemTreeNode &node)
{
    Id id = getUidMapping(node.uid);
    QList<TreeNode> parents;
    foreach(const PimItemTreeNode &parentNode, node.parentNodes) {
        parents << createNode(parentNode);
    }
    return TreeNode(node.name, id, parents);
}


Relation PimItemStructureCache::createRelation(const PimItemRelation &relation, const Id itemId)
{
    QList<TreeNode> parents;
    foreach(const PimItemTreeNode &n, relation.parentNodes) {
        parents << createNode(n);
    }
    return Relation(itemId, parents);
}


Relation PimItemStructureCache::getRelationTree(Id id, const Akonadi::Item &item)
{
    PimItem::Ptr pimitem(PimItemFactory::getItem(item));
    Q_ASSERT (!pimitem.isNull());
    foreach(const PimItemRelation &rel, pimitem->getRelations()) {
//         kDebug() << rel.type;
        if (rel.type == mType) {
            return createRelation(rel, id); //TODO merge multiple relations
        }
    }
    return Relation(id, QList<TreeNode>());
}

QList<PimItemTreeNode> PimItemStructureCache::getParentTreeNodes(Id id)
{
    QList<PimItemTreeNode> list;
    IdList parents = values(id, mParents);
    foreach (Id parent, parents) {
        list << PimItemTreeNode(getUid(parent), mNames.value(parent), getParentTreeNodes(parent));
        kDebug() << mNames.value(parent);
    }
    return list;
}

void PimItemStructureCache::updateRelationTree(Akonadi::Item &item)
{
//     kDebug() << item.id();
    PimItem::Ptr pimitem(PimItemFactory::getItem(item));
    Q_ASSERT(!pimitem.isNull());
    Q_ASSERT(mItemIdCache.contains(item.id()));
    const Id id = mItemIdCache.value(item.id());
//     kDebug() << id;
    QList<PimItemRelation> relations = pimitem->getRelations();
    int i = 0;
    foreach(const PimItemRelation &rel, pimitem->getRelations()) {
        if (rel.type == mType) {
            relations.removeAt(i);
        }
        i++;
    }
    relations << PimItemRelation(mType, getParentTreeNodes(id));
    pimitem->setRelations(relations);
    item = pimitem->getItem();
}

QList<TreeNode> PimItemStructureCache::getParentList(Id id)
{
    QList<TreeNode> list;
    IdList parents = values(id, mParents);
    foreach (Id parent, parents) {
        list << TreeNode(mNames.value(parent), parent, getParentList(parent));
    }
    return list;
}

void PimItemStructureCache::addNode(const QString& name, const IdList& parents)
{
//     foreach (Id id, mNames.keys()) {
//         kDebug() << id << mNames.value(id);
//     }
    
    Q_ASSERT(!name.isEmpty());
    QList<TreeNode> parentNodes;
    foreach (Id parent, parents) {
//         kDebug() << parent;
        parentNodes << TreeNode(getName(parent), parent, getParentList(parent));
        Q_ASSERT(!getName(parent).isEmpty());
    }
    Id id = getNextId();
    addUidMapping(QUuid::createUuid().toByteArray(), id);
    mergeNode(TreeNode(name, id, parentNodes));
}







ProjectStructureCache::ProjectStructureCache()
{

}

Relation ProjectStructureCache::getRelationTree(Id id, const Akonadi::Item& item)
{
    PimItem::Ptr pimitem(PimItemFactory::getItem(item));
    Q_ASSERT (!pimitem.isNull());
    const QByteArray uid = pimitem->getUid().toLatin1();
//     qDebug() << "######### " << item.url().url() << id << uid << pimitem->getRelations().size();
    addUidMapping(uid, id);
    QList<TreeNode> parents;
    foreach(const PimItemRelation &rel, pimitem->getRelations()) {
//         qDebug() << "relation " << rel.type << rel.parentNodes.size();
        if (rel.type == PimItemRelation::Project) {
            foreach (const PimItemTreeNode &p, rel.parentNodes) {
                if (p.uid.isEmpty()) {
                    kWarning() << "empty parent on item: " << item.id();
                    continue;
                }
                Id projectId = getUidMapping(p.uid);
//                 qDebug() << p.uid << projectId;
                parents << TreeNode(p.name, projectId);
            }
        }
    }
    return Relation(id, parents);
}

void ProjectStructureCache::updateRelationTree(Akonadi::Item& /*item*/)
{

}

Id ProjectStructureCache::addCollection(const Akonadi::Collection &col)
{
    if (!mCollectionMapping.contains(col.id())) {
        mCollectionMapping.insert(col.id(), getNextId());
    }
    return mCollectionMapping.value(col.id());
}

bool ProjectStructureCache::hasChildren(Id id) const
{
    //FIXME hotspot
    return mParents.rightContains(id);
}

Id ProjectStructureCache::addItem(const Akonadi::Item &item)
{
    return PimItemRelationCache::addItem(item);
}

Akonadi::Entity::Id ProjectStructureCache::itemId(Id id) const
{
    if (!mItemIdCache.values().contains(id)) {
        return -1;
    }
    return mItemIdCache.key(id);
}

IdList ProjectStructureCache::getChildren(Id id) const
{
    return getAffectedChildItems(id);
}


void ProjectStructureCache::printCache()
{
//     qDebug() << "itemids: " << mItemIdCache;
//     qDebug() << "collections: " << mCollectionMapping;
//     qDebug() << "parents " << mParents;
//     qDebug() << "uids " << mUidMapping;
}


