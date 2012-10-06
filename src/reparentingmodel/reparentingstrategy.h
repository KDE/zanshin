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


#ifndef REPARENTINGSTRATEGY_H
#define REPARENTINGSTRATEGY_H
#include <QList>
#include <QModelIndex>
#include <Akonadi/EntityTreeModel>
#include "globaldefs.h"


class ReparentingModel;
class TodoNode;

/**
 * A reparenting strategy for a ReparentingModel
 *
 * The strategy is basically responsible for identifying nodes, and to give information about the nodes parent relations.
 * Each node can have multiple parents (TODO not yet implemented), and is identified by a unique Id.
 *
 * A node may also insert virtual nodes (nodes without corresponding sourceindex), to i.e. build parent structures.
 *
 * Nodes may also be filtered by returning < 0 on getId;
 */
class ReparentingStrategy
{
public:
    ReparentingStrategy();
    virtual ~ReparentingStrategy(){};
    virtual void init() {};
    /**
     * Return the unique Id for the node.
     *
     * The Id must be unique for this strategy.
     * Ids may be reused after a reset (given all caches are cleared).
     *
     * If the node should be filtered return -1 (children are still added)
     */
    virtual Id getId(const QModelIndex &sourceIndex) = 0;
    /**
     * Return the parents of a node.
     *
     * @param ignore is a list of ids which must be removed from the return list (otherwise moves won't work).
     */
    virtual IdList getParents(const QModelIndex &sourceIndex, const IdList &ignore = IdList());

    /**
     * Reset all internal data (caches etc.)
     *
     * Issued on model reset.
     */
    virtual void reset();

    /**
     * React to the removal of a node (e.g. cleanup cache)
     */
    virtual void onNodeRemoval(const Id &changed);

    /**
     * Set the reparenting model.
     */
    void setModel(ReparentingModel *model);

    /**
     * Return true if @param child should be reparented on parent removal, otherwise the child is removed as well.
     */
    virtual bool reparentOnParentRemoval(Id child) const;

    /**
     * Set data on a virtual node.
     *
     * Called during the creation of a virtual node.
     */
    virtual void setNodeData(TodoNode* /*node*/, Id /*id*/) {};
    virtual QVariant data(Id /*index*/, int /*column*/, int /*role*/, bool &/*forward*/) const { return QVariant(); };

    virtual QMimeData *mimeData(const QModelIndexList &/*proxyIndexes*/) const{return 0;};
    virtual QStringList mimeTypes() { return QStringList(); };
    virtual Qt::ItemFlags flags(const QModelIndex &proxyIndex, Qt::ItemFlags flags) {return flags;};
    virtual Qt::DropActions supportedDropActions() const { return Qt::IgnoreAction; };
    virtual bool onDropMimeData(Id id, const QMimeData* , Qt::DropAction ){ return false; };
    virtual bool onSetData(Id id, const QVariant &value, int role, int column) { return false; };

protected:
    /**
     * Creates a virtual node (no corresponding sourceIndex).
     */
    virtual QList<TodoNode*> createNode(Id id, IdList pid, QString name);
    /**
     * Remove a node.
     */
    void removeNode(Id id);
    /**
     * Trigger an update of the parents (move the node to it's new parents).
     */
    void updateParents(Id id, IdList parents);
    void updateParents(Id id);
    /**
     * Rename a virtual node.
     */
    void renameNode(Id id, QString name);
    /**
     * Get the data of a node (same as index.data)
     */
    QVariant getData(Id id, int role) const;
    void setData(Id id, const QVariant &value, int role);

    Akonadi::Collection getParentCollection(Id id);
    
    Id getNextId();
    /**
     * Set number of reserved id's for internal use (default is 10)
     *
     * Get id will never be used for those
     **/
    void setMinId(Id);
    bool mReparentOnRemoval;

    ReparentingModel *m_model;
private:
    Id mMinIdCounter;
    Id mIdCounter;

};

/**
 * Used to test reparenting without virtual nodes
 */
class TestReparentingStrategy : public ReparentingStrategy
{
public:
    enum Roles {
        First = Akonadi::EntityTreeModel::TerminalUserRole,
        IdRole,
        ParentRole,
        ParentListRole
    };

    explicit TestReparentingStrategy();

    virtual Id getId(const QModelIndex& );
    virtual IdList getParents(const QModelIndex &, const IdList &ignore = IdList());
};


/**
 * Used to test reparenting including virtual nodes
 */
class TestParentStructureStrategy : public ReparentingStrategy
{
public:

    enum Roles {
        First = Akonadi::EntityTreeModel::TerminalUserRole,
        TopicRole,
        TopicParentRole
    };

    explicit TestParentStructureStrategy();

    void addParent(qint64 identifier, qint64 parentIdentifier, const QString &name);
    void setParent(const QModelIndex &item, const qint64 &parentIdentifier);
    void removeParent(const qint64 &identifier);
    void onNodeRemoval(const qint64 &changed) { qDebug() << "removed node: " << changed; };

    virtual bool reparentOnParentRemoval(Id ) const;

    virtual Id getId(const QModelIndex& );
    virtual IdList getParents(const QModelIndex &, const IdList &ignore = IdList());

    virtual void init();
};



#endif // REPARENTINGSTRATEGY_H
