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

#ifndef PIMITEMSERVICES_H
#define PIMITEMSERVICES_H

#include <QtCore/QPointer>
#include <Akonadi/Item>
#include <Akonadi/Collection>
#include "globaldefs.h"
#include "pimitemrelations.h"
#include "pimitemindex.h"

class PimItemRelationCache;
class ProjectStructureInterface;
class PimItemRelationInterface;

/**
 * The purpose of this interface is to change the structure of a relation tree when we don't have direct access to the model.
 * 
 * This includes the following operatoins:
 * * addVirtualNode
 * * removeVirtualNode
 * * move/link to virtual node
 * * rename a virtual node
 *
 * It might make sense to provide the same interface also for projects
 *
 * TODO Why are we not using QModelIndexes? we wouldn't have to fetch each item this way
 * : because we want this to work uniformely from anywhere (e.g. the itemviewer), where we don't have the modelindex available
 *
 * 
 *
 * URIS:
 * Projects: UID, AkonadiItem
 * Context: UID, PimItemRelationId (resolvable through pimitemrelations)
 * Topic: UID, PimItemRelationId
 * Todo: UID, AkonadiItem
 * Note: UID, AkonadiItem
 * Collection: collection
 * 
 */
class PimItemServices
{
public:
    PimItemServices(){};
    virtual ~PimItemServices(){};
    static PimItemRelationInterface &contextInstance();
    static PimItemRelationInterface &topicInstance();
    static ProjectStructureInterface &projectInstance();
    static PimItemServices &getInstance(PimItemRelation::Type type);

    static PimItemIndex fromIndex(const QModelIndex &);

//     static PimNode projectNode(const Akonadi::Item &);
//     static PimNode contextNode(Id);
//     static PimNode topicNode(Id);
//     static PimNode todoNode(const Akonadi::Item &);
//     static PimNode noteNode(const Akonadi::Item &);
//     static PimNode collectionNode(const Akonadi::Collection &);

    static void create(PimItemIndex::ItemType type, const QString &name, const QList<PimItemIndex> &parents = QList<PimItemIndex>(), const Akonadi::Collection &col = Akonadi::Collection());
    static void remove(const PimItemIndex &node, QWidget *);
    static void remove(const QList<PimItemIndex> &nodes, QWidget *);
    static void moveTo(const PimItemIndex &node, const PimItemIndex &parent);
    static void linkTo(const PimItemIndex &node, const PimItemIndex &parent);
    static void unlink(const PimItemIndex &node, const PimItemIndex &parent);
    static void rename(const PimItemIndex &node, const QString &name);

    void setRelationsStructure(PimItemRelationCache *);

    virtual void add(const QString &/*name*/, const QList<PimItemIndex> &parents = QList<PimItemIndex>()) {Q_UNUSED(parents);};
//     virtual bool remove(QWidget * /*widget*/, const QModelIndexList &relations) {return false;};
//     virtual bool moveTo(const QModelIndex &/*node*/, const QModelIndex &parent) = 0;
//     virtual bool linkTo(const QModelIndex &/*node*/, const QModelIndex &parent) {return false;};
//     virtual bool unlink(const Akonadi::Item &/*item*/, QModelIndex parent) {return false;};
//     virtual bool rename(const QModelIndex &node, const QString &name) {return false;};
 
    virtual PimItemTreeNode getNode(const PimItemIndex &) const;
protected:
    QPointer<PimItemRelationCache> mStructure;
};

class PimItemRelationInterface : public PimItemServices
{
public:
    PimItemRelationInterface();
    virtual ~PimItemRelationInterface();

    void add(const QString &name, const QList<PimItemIndex> &parents = QList<PimItemIndex>());
//     bool remove(QWidget *widget, const QModelIndexList &relations);
//     bool moveTo(const QModelIndex &node, const QModelIndex &parent);
//     bool linkTo(const QModelIndex &node, const QModelIndex &parent);
//     bool unlink(const Akonadi::Item &item, const QModelIndex &parent);
//     bool rename(const QModelIndex &node, const QString &name);
private:
//     bool remove(const Id &relation);
};
// 
class ProjectStructureInterface: public PimItemServices
{
public:
    ProjectStructureInterface();
    bool moveTo(const PimItemIndex &node, const PimItemIndex &parent);
    void remove(const QList<PimItemIndex> &nodes, QWidget *);
    void remove(const PimItemIndex &node, QWidget *);
};

#endif // PIMITEMRELATIONINTERFACE_H
