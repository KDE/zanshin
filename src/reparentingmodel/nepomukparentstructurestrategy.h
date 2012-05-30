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


#ifndef NEPOMUKPARENTSTRUCTURESTRATEGY_H
#define NEPOMUKPARENTSTRUCTURESTRATEGY_H

#include <QObject>
#include "reparentingstrategy.h"

#include <Nepomuk/Resource>
#include <Nepomuk/Query/Result>

namespace Nepomuk {
namespace Query {
class QueryServiceClient;
}
}

class NepomukParentStructureStrategy : public QObject, public ReparentingStrategy
{
    Q_OBJECT
public:
    explicit NepomukParentStructureStrategy();

    virtual void init();

    //Set the basic query
    virtual void setType(const QUrl &);

    virtual Id getId(const QModelIndex& );
    virtual IdList getParents(const QModelIndex &, const IdList &ignore = IdList());
    virtual void onNodeRemoval(const qint64& changed);

    virtual bool onDropMimeData(Id id, const QMimeData* , Qt::DropAction );
    virtual bool onSetData(Id id, const QVariant &value, int role);

    virtual void setData(TodoNode* node, qint64 id);

    virtual void reset();

    virtual QStringList mimeTypes();
    virtual Qt::ItemFlags flags(const QModelIndex &index, Qt::ItemFlags);


private slots:
    void checkResults(const QList<Nepomuk::Query::Result> &);
    void removeResult(const QList<QUrl> &);
    void queryFinished();

    void itemsWithTopicAdded(const QList<Nepomuk::Query::Result> &results);
    void itemsFromTopicRemoved(const QList<QUrl> &items);
    void propertyChanged(const Nepomuk::Resource &res, const Nepomuk::Types::Property &property, const QVariant &value);

private:
    void addParent (const Nepomuk::Resource& topic, const QUrl &parent = QUrl());
    Nepomuk::Query::QueryServiceClient *m_queryServiceClient;
    QMap<QUrl, QObject*> m_guardMap;
    QMap<QUrl, Id> m_topicMap;
    QMap<Akonadi::Item::Id, Id> m_itemIdMap;
    QMap<QUrl, QList<Id> > m_topicCache; //cache akonadi item uris and their topics
    QUrl m_type;
    Id mInbox;
    Id mRoot;

};

#endif // NEPOMUKPARENTSTRUCTURESTRATEGY_H
