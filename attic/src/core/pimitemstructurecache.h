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

#ifndef PIMITEMSTRUCTURECACHE_H
#define PIMITEMSTRUCTURECACHE_H

#include "pimitemrelationcache.h"
#include "pimitemrelations.h"

class PimItemStructureCache: public VirtualRelationCache {
public:
    explicit PimItemStructureCache(PimItemRelation::Type);
    void addNode(const QString &name, const IdList &parents);
    virtual void updateRelationTree(Akonadi::Item& item);
protected:
    //Build a relation tree from the context of an item
    Relation getRelationTree(Id id, const Akonadi::Item &item);
private:
    TreeNode createNode(const PimItemTreeNode &node);
    Relation createRelation(const PimItemRelation &relation, const Id itemId);
    QList<PimItemTreeNode> getParentTreeNodes(Id id);
    QList<TreeNode> getParentList(Id id);
    PimItemRelation::Type mType;
};

class ProjectStructureCache: public VirtualRelationCache {
public:
    ProjectStructureCache();
    Id addCollection(const Akonadi::Collection &);
    bool hasChildren(Id) const;
    void printCache();

    Akonadi::Item::Id itemId(Id id) const;
    IdList getChildren(Id id) const;
protected:
    Relation getRelationTree(Id id, const Akonadi::Item &item);
private:
    QHash<Akonadi::Collection::Id, Id> mCollectionMapping;
};

#endif // PIMITEMSTRUCTURECACHE_H
