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


#ifndef PIMITEMRELATIONS_H
#define PIMITEMRELATIONS_H
#include <QDateTime>
#include <akonadi/item.h>
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
    TreeNode(const QString &name, const Id &uid, const QList<TreeNode> &parentNodes = QList<TreeNode>());
    QString name;
    Id id;
    QList<TreeNode> parentNodes;
};

struct Relation
{
  Relation(Id id, const QList<TreeNode> &parentNodes);
  Relation();
  QList<TreeNode> parentNodes;
  Id id;
  //     QDateTime timestamp; //for merging
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
 */
class PimItemRelations: public QObject
{
    Q_OBJECT
public:
    PimItemRelations();
    //Handles merging, typically the tree of a single item
    Id addItem(const Akonadi::Item &);
    Id getItemId(const Akonadi::Item &) const;

    /**
     * Recursively remove subtree
     * 
     * for all nodes
     */
    void removeNode(Id);
    //for all nodes
    IdList getParents(Id child);

    //only for virtual nodes
    QString getName(Id);

    //only for virtual nodes
    void renameNode(Id, const QString &);
    //for all nodes
    void moveNode(Id, IdList parents);
    bool isVirtual(Id) const;

    //Store current relations to item
    virtual void updateRelationTree(Akonadi::Item &item) = 0;

    //Get a path, representing the hierarchy, which can be used for manual editing
    virtual QString getPath(Id id) const = 0;
signals:

    //only for virtual nodes
    void virtualNodeAdded(Id id, IdList parents, const QString &name);
    //for all nodes
    void nodeRemoved(Id id);
    //for all nodes
    void parentsChanged(Id id, IdList parents);
    //only for virtual nodes
    void virtualNodeRenamed(Id id, const QString &name);
    //signals items which need updating
    void updateItems(IdList);
protected:
    IdList getAffectedChildItems(Id id) const;
    void removeNodeRecursive(Id id);
    virtual Relation getRelationTree(const Akonadi::Item &item) = 0;
    virtual void rebuildCache() = 0;
    IdList getChildNodes(Id id) const;
    Id getOrCreateItemId(const Akonadi::Item &item);
    void mergeNode(const TreeNode &node);
    
    QMap<Id, QString> mNames;
    QMultiMap<Id, Id> mParents;
    QMap<Akonadi::Item::Id, Id> mItemIdCache;
    Id mIdCounter;
};

class CategoriesStructure: public PimItemRelations {
public:
    CategoriesStructure();
    void addCategoryNode(const QString &categoryPath, const IdList &parents);
    Id getCategoryId(const QString& categoryPath) const;
    virtual void updateRelationTree(Akonadi::Item& item);
    virtual QString getPath(Id id) const;
protected:
    //Build a relation tree from the category of an item
    Relation getRelationTree(const Akonadi::Item &item);

    virtual void rebuildCache();
private:
    QString getCategoryPath(Id id) const;
    TreeNode createCategoryNode(const QString &categoryPath);
    QMap<QString, Id> mCategoryMap;
};


#endif // PIMITEMRELATIONS_H
