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

    virtual void reset(){};

    void setModel(ReparentingModel *model);

    ///Return true if @param child should be reparented on parent removal
    virtual bool reparentOnRemoval(Id child) const;

protected:
    virtual TodoNode *createNode(Id id, IdList pid, QString name);
    void removeNode(Id id);
    void updateParents(Id id, IdList parents);
    void renameNode(Id id, QString name);
    
    Id getNextId();
    Id mIdCounter;
    bool mReparentOnRemoval;

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

    virtual Id getId(const QModelIndex& );
    virtual IdList getParents(const QModelIndex &, const IdList &ignore = IdList());

};



class ProjectStrategy : public ReparentingStrategy
{
public:
    ProjectStrategy();
    virtual void init();
    virtual Id getId(const QModelIndex& );
    virtual IdList getParents(const QModelIndex&, const IdList &ignore = IdList());
    virtual void reset();
private:
    QHash<QString, Id> mUidMapping;
    QHash<Akonadi::Collection::Id, Id> mCollectionMapping;
    const Id mInbox;
};


class TestParentStructureStrategy : public ReparentingStrategy
{
public:

    enum Roles {
        First = Akonadi::EntityTreeModel::TerminalUserRole,
        TopicRole,
        TopicParentRole, //For items and topics
        TopicNameRole
    };

    explicit TestParentStructureStrategy(QObject* parent = 0);

    void addParent(qint64 identifier, qint64 parentIdentifier, const QString &name);
    void setParent(const QModelIndex &item, const qint64 &parentIdentifier);
    void removeParent(const qint64 &identifier);
    void onNodeRemoval(const qint64 &changed) { qDebug() << "removed node: " << changed; };

    virtual bool reparentOnRemoval(Id ) const;

    virtual Id getId(const QModelIndex& );
    virtual IdList getParents(const QModelIndex &, const IdList &ignore = IdList());

    virtual void init();

};

// class NepomukParentStructureStrategy : public QObject, public ParentStructureStrategy
// {
//     Q_OBJECT
// public:
//     explicit NepomukParentStructureStrategy(QObject* parent = 0);
// 
//     virtual void init();
// 
//     //Set the basic query
//     virtual void setType(const QUrl &);
// 
//     virtual QList<qint64> onSourceInsertRow(const QModelIndex &sourceChildIndex);
//     virtual QList<qint64> onSourceDataChanged(const QModelIndex &changed);
//     virtual void onNodeRemoval(const qint64& changed);
//     virtual bool onDropMimeData(const QMimeData* mimeData, Qt::DropAction action, qint64 id);
//     virtual bool onSetData(qint64 id, const QVariant &value, int role);
// 
//     virtual void setData(TodoNode* node, qint64 id);
// 
//     virtual void reset();
// 
// private slots:
//     void checkResults(const QList<Nepomuk::Query::Result> &);
//     void removeResult(const QList<QUrl> &);
//     void queryFinished();
// 
//     void itemsWithTopicAdded(const QList<Nepomuk::Query::Result> &results);
//     void itemsFromTopicRemoved(const QList<QUrl> &items);
//     void propertyChanged(const Nepomuk::Resource &res, const Nepomuk::Types::Property &property, const QVariant &value);
// 
// private:
//     void addParent (const Nepomuk::Resource& topic, const QUrl &parent = QUrl());
//     Nepomuk::Query::QueryServiceClient *m_queryServiceClient;
//     QMap<QUrl, QObject*> m_guardMap;
//     QMap<QUrl, qint64> m_topicMap;
//     QMap<QUrl, QList<qint64> > m_topicCache; //cache akonadi item uris and their topics
//     QUrl m_type;
//     qint64 m_counter;
// 
// };

#endif // REPARENTINGSTRATEGY_H
