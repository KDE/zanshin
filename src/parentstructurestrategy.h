/* This file is part of Zanshin Todo.
 * 
 * Copyright 2012 Christian Mollekopf <chrigi_1@fastmail.fm>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#ifndef NEPOMUKADAPTER_H
#define NEPOMUKADAPTER_H

#include <QObject>
#include <QUrl>
#include <QAbstractItemModel>
#include <akonadi/item.h>
#include <akonadi/entitytreemodel.h>
#include "todonode.h"

class QUrl;
namespace Nepomuk2 {
namespace Types {

class Property;
}

namespace Query {
class Result;
class Query;
class QueryServiceClient;
}
class Resource;
}

class ParentStructureModel;

class ParentStructureStrategy: public QObject
{
    Q_OBJECT
public:
    explicit ParentStructureStrategy(QObject* parent = 0);
    virtual void init(){};
    
    void setModel(ParentStructureModel *model);
    
    //Called when the item first appears in the model, used to get the correct parent right away
    virtual QList<qint64> onSourceInsertRow(const QModelIndex &/*sourceChildIndex*/) { return QList<qint64>();};
    //Called whenever the item has changed, to reevaluate the parents
    virtual QList<qint64> onSourceDataChanged(const QModelIndex &/*changed*/) {return QList<qint64>();};
    //Called whenever a parentNode is removed by removeNode(). (I.e. to cleanup the internals)
    virtual void onNodeRemoval(const qint64 &/*changed*/) {};
    
    virtual bool onDropMimeData(const QMimeData* /*mimeData*/, Qt::DropAction /*action*/, qint64 /*id*/){ return false; };
    virtual bool onSetData(qint64 /*id*/, const QVariant &/*value*/, int /*role*/) { return false; };
    
    virtual void setData(TodoNode */*node*/, qint64 /*id*/){};

    virtual void reset(){};
    
protected:
    ParentStructureModel *m_model;
};

class TestParentStructureStrategy : public ParentStructureStrategy
{    
    Q_OBJECT
public:
    
    enum Roles {
        First = Akonadi::EntityTreeModel::TerminalUserRole,
        TopicRole,
        TopicParentRole, //For items and topics
        TopicNameRole
    };
    
    explicit TestParentStructureStrategy(QObject* parent = 0);
    
    void addParent(const qint64 &identifier, const qint64 &parentIdentifier, const QString &name);
    void setParent(const QModelIndex &item, const qint64 &parentIdentifier);
    void removeParent(const qint64 &identifier);
    void onNodeRemoval(const qint64 &changed) { qDebug() << "removed node: " << changed; };
    
    virtual QList<qint64> onSourceInsertRow(const QModelIndex &sourceChildIndex);
    virtual QList<qint64> onSourceDataChanged(const QModelIndex &changed);
    
};

class NepomukParentStructureStrategy : public ParentStructureStrategy
{
    Q_OBJECT
public:
    explicit NepomukParentStructureStrategy(QObject* parent = 0);
    
    virtual void init();
    
    //Set the basic query
    virtual void setType(const QUrl &);
    
    virtual QList<qint64> onSourceInsertRow(const QModelIndex &sourceChildIndex);
    virtual QList<qint64> onSourceDataChanged(const QModelIndex &changed);
    virtual void onNodeRemoval(const qint64& changed);
    virtual bool onDropMimeData(const QMimeData* mimeData, Qt::DropAction action, qint64 id);
    virtual bool onSetData(qint64 id, const QVariant &value, int role);
    
    virtual void setData(TodoNode* node, qint64 id);

    virtual void reset();
    
private slots:
    void checkResults(const QList<Nepomuk2::Query::Result> &);
    void removeResult(const QList<QUrl> &);
    void queryFinished();
    
    void itemsWithTopicAdded(const QList<Nepomuk2::Query::Result> &results);
    void itemsFromTopicRemoved(const QList<QUrl> &items);
    void propertyChanged(const Nepomuk2::Resource &res, const Nepomuk2::Types::Property &property, const QVariant &value);
    
private:
    void addParent (const Nepomuk2::Resource& topic, const QUrl &parent = QUrl());
    Nepomuk2::Query::QueryServiceClient *m_queryServiceClient;
    QMap<QUrl, QObject*> m_guardMap;
    QMap<QUrl, qint64> m_topicMap;
    QMap<QUrl, QList<qint64> > m_topicCache; //cache akonadi item uris and their topics
    QUrl m_type;
    qint64 m_counter;
    
};

#endif // NEPOMUKADAPTER_H
