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




typedef qint64 Id;
typedef QList<qint64> IdList;
class ReparentingModel;
class TodoNode;
class ReparentingStrategy
{
public:
    ReparentingStrategy();
    virtual ~ReparentingStrategy(){};
    virtual void init() {};
    /// Get the id for an object
    virtual Id getId(const QModelIndex &/*sourceChildIndex*/) = 0;
    /// Get parents
    virtual IdList getParents(const QModelIndex &, const IdList &ignore = IdList());

    virtual void onNodeRemoval(const Id &changed);

    virtual void reset();

    void setModel(ReparentingModel *model);

    ///Return true if @param child should be reparented on parent removal
    virtual bool reparentOnRemoval(Id child) const;

    /**
     * Set data on a virtual node
     */
    virtual void setData(TodoNode* node, Id id) {};

    virtual QStringList mimeTypes() { return QStringList(); };
    virtual Qt::ItemFlags flags(const QModelIndex &index, Qt::ItemFlags flags) {return flags;};
    
    virtual bool onDropMimeData(Id id, const QMimeData* , Qt::DropAction ){ return false; };
    virtual bool onSetData(Id id, const QVariant &value, int role) { return false; };

protected:
    virtual TodoNode *createNode(Id id, IdList pid, QString name);
    void removeNode(Id id);
    void updateParents(Id id, IdList parents);
    void renameNode(Id id, QString name);
    
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

class TestReparentingStrategy : public ReparentingStrategy
{
public:
    enum Roles {
        First = Akonadi::EntityTreeModel::TerminalUserRole,
        IdRole,
        ParentRole
    };

    explicit TestReparentingStrategy();

    virtual Id getId(const QModelIndex& );
    virtual IdList getParents(const QModelIndex &, const IdList &ignore = IdList());
};


// class InboxStrategy : public ReparentingStrategy
// {
// protected:
//     InboxStrategy(const QString &inboxName, const QString &rootName);
//     virtual void init();
//     const Id mInbox;
//     const Id mRoot;
// };

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

    virtual bool reparentOnRemoval(Id ) const;

    virtual Id getId(const QModelIndex& );
    virtual IdList getParents(const QModelIndex &, const IdList &ignore = IdList());

    virtual void init();
};



#endif // REPARENTINGSTRATEGY_H
