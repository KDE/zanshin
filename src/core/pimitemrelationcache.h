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

#ifndef PIMITEMRELATIONCACHE_H
#define PIMITEMRELATIONCACHE_H

#include <akonadi/item.h>
#include "reparentingmodel/kbihash_p.h"
#include "globaldefs.h"

/**
 * A relation tree
 *
 * All uids are meant to be globaly unique and may be matched against other objects depending on the type.
 * The name provides an intermediate representation which should be overridden by the representation of a matching object.
 *
 * The same object (same uid), may exist several times in the tree under different nodes.
 *
 * For conflict resolution the hosts mechanisms should be used such as lastModifiedDate or sequence number.
 *
 */

struct TreeNode {
    TreeNode(const QString &name, const Id &uid, const QList<TreeNode> &parentNodes);
    TreeNode(const QString &name, const Id &uid);
    QString name;
    Id id;
    QList<TreeNode> parentNodes;
    bool knowsParents;
};

struct Relation
{
  Relation(Id id, const QList<TreeNode> &parentNodes);
  Relation();
  Id id;
  QList<TreeNode> parentNodes;
  //     QDateTime timestamp; //for merging
};

/*
 * PimItemRelationCache: A cache which assigns an id to each of the passed in objects, and allows queries by UID
 * PimItemRelations: Allows additionally the creation of virtual items (which have a name).
 */
class PimItemRelationCache: public QObject
{
    Q_OBJECT
public:
    PimItemRelationCache();
    //Handles merging, typically the tree of a single item
    virtual Id addItem(const Akonadi::Item &);
    Id getItemId(const Akonadi::Item &) const;
    Id getId(const QByteArray &uid) const;
    QByteArray getUid(Id) const;
    
    virtual void removeNode(Id);
    
    //for all nodes
    IdList getParents(Id child);
    
    virtual void addNode(const QString &/*name*/, const IdList &/*parents*/){};
    //for all nodes
    void moveNode(Id, IdList parents);
    virtual bool isVirtual(Id) const;
    typedef KBiAssociativeContainer< QMultiMap<Id, Id>, QMultiMap<Id, Id> > ParentMapping;

signals:
    //for all nodes
    void nodeRemoved(Id id);
    //for all nodes
    void parentsChanged(Id id, IdList parents);
    //signals items which need updating
    void updateItems(IdList);
protected:
    IdList values(Id key, const PimItemRelationCache::ParentMapping &map) const;
    IdList getAffectedChildItems(Id id) const;
    void removeNodeRecursive(Id id);
    virtual Relation getRelationTree(Id id, const Akonadi::Item &item) = 0;
    virtual void rebuildCache(){};
    IdList getChildNodes(Id id) const;
    virtual void mergeNode(const TreeNode &node);

    void addUidMapping(const QByteArray &uid, Id);
    Id getUidMapping(const QByteArray &uid);
    
    ParentMapping mParents;
    QMap<Akonadi::Item::Id, Id> mItemIdCache;
    Id getNextId();
    //Only for debugging
    QHash<QByteArray, Id> uidMapping() const;
private:
    QHash<QByteArray, Id> mUidMapping;
    Id mIdCounter;
    Id getOrCreateItemId(const Akonadi::Item &item);

};

/**
 * Interfaces directly with akonadi (could be further abstracted for other storage backends, but that's for another one)
 *
 * Handles modifications decoupled from the displaying model (stores information to all underlying items).
 *
 * Works with RelationTrees, as abstraction from the actual relation mechanism.
 *
 * This class is mainly useful for parent relations where every item transports a complete branch of a larger relation tree.
 * It doesn't add any value to simple relations such as todos relatedTo as there we have no need for merging and changes are only saved to a single item.
 *
 * TODO cleanup mItemIdCache after an item has been removed
 * TODO Remove Akonadi::Item from this class and instead work only with Relations and an id (where the id can be the akonadi item id or also something else), that should make the wohle thing much more testable
 *
 *
 * PimItemRelationCache vs TodoMetadataModel for Projects
 * The primary motiviation, aside of codesharing with PimItemRelations, is to have a cache which can be queried for things like UID to akonadi-item, or childitems of a node.
 * Those queries could also be written agains the model of course.... In this case the Interface would have to have a pointer to the model.
 *
 * After all both solutions are perfectly possible, but since I want a uniform solution for all three relation threes, i opted for getting rid of the TodoMetadataModel.
 * (It is also easier to implement a good api for modifications there)
 */
class VirtualRelationCache: public PimItemRelationCache
{
    Q_OBJECT
public:
    VirtualRelationCache();

    virtual void removeNode(Id);

    //only for virtual nodes
    QString getName(Id);

    //only for virtual nodes
    void renameNode(Id, const QString &);

    //Store current relations to item
    virtual void updateRelationTree(Akonadi::Item &/* item */) {};

signals:
    //only for virtual nodes
    void virtualNodeAdded(Id id, IdList parents, const QString &name);
    //only for virtual nodes
    void virtualNodeRenamed(Id id, const QString &name);
protected:
    virtual void removeNodeRecursive(Id id);
    virtual void mergeNode(const TreeNode &node);
    
    QMap<Id, QString> mNames;
};

#endif // PIMITEMRELATIONCACHE_H
