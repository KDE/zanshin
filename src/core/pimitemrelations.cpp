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


