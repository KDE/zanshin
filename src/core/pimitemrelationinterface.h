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

#ifndef PIMITEMRELATIONINTERFACE_H
#define PIMITEMRELATIONINTERFACE_H

#include <QtCore/QPointer>
#include "globaldefs.h"
#include "pimitemrelations.h"

class ProjectStructureInterface;

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


    struct PimNode {
        
        enum NodeType {
            Invalid,
            Empty,
            Collection,
            Project,
            Context,
            Topic,
            PimItem,
            Todo,
            Note
        };
        PimNode(NodeType t): type(t){};
        NodeType type;
        Akonadi::Item item;
        Id relationId;
        Akonadi::Collection collection;
        QString uid;
    };

class PimItemStructureInterface
{
public:
    PimItemStructureInterface(){};
    virtual ~PimItemStructureInterface(){};
    static PimItemStructureInterface &contextInstance();
    static PimItemStructureInterface &topicInstance();
    static ProjectStructureInterface &projectInstance();

    static PimNode fromIndex(const QModelIndex &);

//     static PimNode projectNode(const Akonadi::Item &);
//     static PimNode contextNode(Id);
//     static PimNode topicNode(Id);
//     static PimNode todoNode(const Akonadi::Item &);
//     static PimNode noteNode(const Akonadi::Item &);
//     static PimNode collectionNode(const Akonadi::Collection &);

    static void create(PimNode::NodeType type, const QString &name, const QList<PimNode> &parents = QList<PimNode>(), const Akonadi::Collection &col = Akonadi::Collection());
    static void remove(const PimNode &node, QWidget *);
    static void remove(const QList<PimNode> &nodes, QWidget *);
    static void moveTo(const PimNode &node, const PimNode &parent);
    static void linkTo(const PimNode &node, const PimNode &parent);
    static void unlink(const PimNode &node, const PimNode &parent);
    static void rename(const PimNode &node, const QString &name);

    void setRelationsStructure(PimItemRelationCache *);

    virtual void add(const QString &/*name*/, const QList<PimNode> &parents = QList<PimNode>()) {Q_UNUSED(parents);};
//     virtual bool remove(QWidget * /*widget*/, const QModelIndexList &relations) {return false;};
//     virtual bool moveTo(const QModelIndex &/*node*/, const QModelIndex &parent) = 0;
//     virtual bool linkTo(const QModelIndex &/*node*/, const QModelIndex &parent) {return false;};
//     virtual bool unlink(const Akonadi::Item &/*item*/, QModelIndex parent) {return false;};
//     virtual bool rename(const QModelIndex &node, const QString &name) {return false;};
protected:
    QPointer<PimItemRelationCache> mStructure;
};

class PimItemRelationInterface : public PimItemStructureInterface
{
public:
    PimItemRelationInterface();
    virtual ~PimItemRelationInterface();

    void add(const QString &name, const QList<PimNode> &parents = QList<PimNode>());
//     bool remove(QWidget *widget, const QModelIndexList &relations);
//     bool moveTo(const QModelIndex &node, const QModelIndex &parent);
//     bool linkTo(const QModelIndex &node, const QModelIndex &parent);
//     bool unlink(const Akonadi::Item &item, const QModelIndex &parent);
//     bool rename(const QModelIndex &node, const QString &name);
private:
    friend class PimItemRelationsStructure;
//     bool remove(const Id &relation);
};
// 
class ProjectStructureInterface: public PimItemStructureInterface
{
public:
    ProjectStructureInterface();
    bool moveTo(const PimNode &node, const PimNode &parent);
    void remove(const QList<PimNode> &nodes, QWidget *);
    void remove(const PimNode &node, QWidget *);
    
};

#endif // PIMITEMRELATIONINTERFACE_H
