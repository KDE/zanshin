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
    virtual IdList getParents(const QModelIndex &);

    virtual void onNodeRemoval(const qint64 &changed);

    virtual void reset(){};

    void setModel(ReparentingModel *model);

    bool reparentOnRemoval() const;

protected:
    TodoNode *createNode(Id id, Id pid, QString name);
    Id getNextId();
    Id mIdCounter;
    bool mReparentOnRemoval;
private:
    ReparentingModel *m_model;
    
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

//     void addParent(const qint64 &identifier, const qint64 &parentIdentifier, const QString &name);
//     void setParent(const QModelIndex &item, const qint64 &parentIdentifier);
//     void removeParent(const qint64 &identifier);
//     void onNodeRemoval(const qint64 &changed) { qDebug() << "removed node: " << changed; };

    virtual Id getId(const QModelIndex& );
    virtual IdList getParents(const QModelIndex &);

};



class ProjectStrategy : public ReparentingStrategy
{
public:
    ProjectStrategy();
    virtual void init();
    virtual Id getId(const QModelIndex& );
    virtual IdList getParents(const QModelIndex&);
    virtual void reset();
private:
    QHash<QString, Id> mUidMapping;
    QHash<Akonadi::Collection::Id, Id> mCollectionMapping;
    const Id mInbox;
};

#endif // REPARENTINGSTRATEGY_H
