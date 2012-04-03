/* This file is part of Zanshin Todo.
 * 
 * Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>
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

#include "nepomukadapter.h"
#include <queries.h>
#include <pimitem.h>
#include <tagmanager.h>
#include "globaldefs.h"
#include "topicsmodel.h"
#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Query/Result>
#include <Nepomuk/Types/Class>
#include <Nepomuk/Resource>
#include <KDebug>
#include <nepomuk/resourcetypeterm.h>
#include <nepomuk/resourcewatcher.h>
#include <Soprano/Vocabulary/NAO>
#include <QMimeData>

StructureAdapter::StructureAdapter(QObject* parent): QObject(parent), m_model(0)
{

}


void StructureAdapter::setModel(TopicsModel* model)
{
    m_model = model;
}


TestStructureAdapter::TestStructureAdapter(QObject* parent)
: StructureAdapter(parent)
{

}


TopicsModel::IdList TestStructureAdapter::onSourceInsertRow(const QModelIndex &sourceChildIndex)
{
    if (!sourceChildIndex.isValid()) {
        kWarning() << "invalid indexx";
        return TopicsModel::IdList();
    }

    if (!sourceChildIndex.data(TopicParentRole).isValid()) {
        return TopicsModel::IdList();
    }
    const TopicsModel::Id &parent = sourceChildIndex.data(TopicParentRole).value<TopicsModel::Id>();

    return TopicsModel::IdList() << parent;
}

TopicsModel::IdList TestStructureAdapter::onSourceDataChanged(const QModelIndex &sourceIndex)
{
    return onSourceInsertRow(sourceIndex);
}


void TestStructureAdapter::addParent(const TopicsModel::Id& identifier, const TopicsModel::Id& parentIdentifier, const QString& name)
{
    kDebug() << identifier << parentIdentifier << name;
    m_model->createOrUpdateParent(identifier, parentIdentifier, name);
}

void TestStructureAdapter::removeParent(const TopicsModel::Id& identifier)
{
    m_model->removeNode(identifier);
}


NepomukAdapter::NepomukAdapter(QObject* parent)
: StructureAdapter(parent), m_counter(0)
{

}


void NepomukAdapter::setType(const QUrl &type)
{
    m_type = type;
    
    Nepomuk::Query::Query query;
    query.setTerm(Nepomuk::Query::ResourceTypeTerm(Nepomuk::Types::Class(m_type)));

    Nepomuk::Query::QueryServiceClient *queryServiceClient = new Nepomuk::Query::QueryServiceClient(this);
    connect(queryServiceClient, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)), this, SLOT(checkResults(QList<Nepomuk::Query::Result>)));
    connect(queryServiceClient, SIGNAL(finishedListing()), this, SLOT(queryFinished()));
    connect(queryServiceClient, SIGNAL(entriesRemoved(QList<QUrl>)), this, SLOT(removeResult(QList<QUrl>)));
    if ( !queryServiceClient->query(query) ) {
        kWarning() << "error";
    }
}

void NepomukAdapter::checkResults(const QList< Nepomuk::Query::Result > &results)
{
    //kDebug() <<  results.size() << results.first().resource().resourceUri() << results.first().resource().label() << results.first().resource().types() << results.first().resource().className();
    foreach (const Nepomuk::Query::Result &result, results) {
        Nepomuk::Resource res(result.resource().resourceUri());
        kDebug() << res.resourceUri() << res.label() << res.types() << res.className();
        if (res.types().contains(m_type)) {
            addParent(res);
        } else {
            kWarning() << "unknown result " << res.types();
        }
    }
}


void NepomukAdapter::addParent (const Nepomuk::Resource& topic)
{
    kDebug() << "add topic" << topic.label() << topic.resourceUri();
    QObject *guard = new QObject(this);
    m_guardMap[topic.resourceUri()] = guard;
    
    Nepomuk::ResourceWatcher *m_resourceWatcher = new Nepomuk::ResourceWatcher(guard);
    m_resourceWatcher->addResource(topic);
    m_resourceWatcher->addProperty(Soprano::Vocabulary::NAO::prefLabel());
    connect(m_resourceWatcher, SIGNAL(propertyAdded(Nepomuk::Resource,Nepomuk::Types::Property,QVariant)), this, SLOT(propertyChanged(Nepomuk::Resource,Nepomuk::Types::Property,QVariant)));
    m_resourceWatcher->start();
    
    Nepomuk::Query::QueryServiceClient *queryServiceClient = new Nepomuk::Query::QueryServiceClient(guard);
    queryServiceClient->setProperty("resourceuri", topic.resourceUri());
    connect(queryServiceClient, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)), this, SLOT(itemsWithTopicAdded(QList<Nepomuk::Query::Result>)));
    connect(queryServiceClient, SIGNAL(entriesRemoved(QList<QUrl>)), this, SLOT(itemsFromTopicRemoved(QList<QUrl>)));
    connect(queryServiceClient, SIGNAL(finishedListing()), this, SLOT(queryFinished()));
    //connect(queryServiceClient, SIGNAL(finishedListing()), queryServiceClient, SLOT(deleteLater()));
    if ( !queryServiceClient->sparqlQuery(MindMirrorQueries::itemsWithTopicsQuery(QList <QUrl>() << topic.resourceUri())) ) {
        kWarning() << "error";
    }
//     emit parentAdded(topic.resourceUri().toString(), QString(), topic.label());
    Q_ASSERT(m_topicMap.contains(topic.resourceUri()));
    m_model->createOrUpdateParent(m_topicMap[topic.resourceUri()], -1, topic.label());
}

void NepomukAdapter::onNodeRemoval(const qint64& changed)
{
    //NepomukUtils::deleteTopic(childIndex.data(Zanshin::UriRole).toUrl()); //TODO maybe leave this up to nepomuk subresource handling?
}


void NepomukAdapter::removeResult(const QList<QUrl> &results)
{
    foreach (const QUrl &result, results) {
        Nepomuk::Resource res(result);
        kDebug() << res.resourceUri() << res.label() << res.types() << res.className();
        if (res.types().contains(m_type)) {
            Q_ASSERT(m_topicMap.contains(res.resourceUri()));
            m_model->removeNode(m_topicMap[res.resourceUri()]);
            m_guardMap.take(res.resourceUri())->deleteLater();
        } else {
            kWarning() << "unknown result " << res.types();
        }
    }
}

void NepomukAdapter::queryFinished()
{
    kWarning();
    //emit ready();
}


void NepomukAdapter::itemsWithTopicAdded(const QList<Nepomuk::Query::Result> &results)
{
    const QUrl &parent = sender()->property("resourceuri").toUrl();
    kDebug() << parent;
    
    QModelIndexList list;
    foreach (const Nepomuk::Query::Result &result, results) {
        Nepomuk::Resource res = Nepomuk::Resource(result.resource().resourceUri());
        kDebug() << res.resourceUri() << res.label() << res.types() << res.className();
        const Akonadi::Item item = PimItemUtils::getItemFromResource(res);
        if (!item.isValid()) {
            continue;
        }
        const QModelIndexList &indexes = Akonadi::EntityTreeModel::modelIndexesForItem(m_model, item);
        if (indexes.isEmpty()) {
            kDebug() << "item not found" << item.url();
            continue;
        }
        list.append(indexes.first()); //TODO hanle all
        Q_ASSERT(m_topicMap.contains(parent));
        m_model->itemParentsChanged(indexes.first(), TopicsModel::IdList() << m_topicMap[parent]);
    }
}

void NepomukAdapter::itemsFromTopicRemoved(const QList<QUrl> &items)
{
    const QUrl &topic = sender()->property("topic").toUrl();
    kDebug() << "removing nodes from topic: " << topic;
    QModelIndexList list;
    foreach (const QUrl &uri, items) {
        Nepomuk::Resource res = Nepomuk::Resource(uri);
        const Akonadi::Item item = PimItemUtils::getItemFromResource(res);
        if (!item.isValid()) {
            continue;
        }
        kDebug() << item.url();
        const QModelIndexList &indexes = Akonadi::EntityTreeModel::modelIndexesForItem(m_model, item);
        if (indexes.isEmpty()) {
            kDebug() << "item not found" << item.url();
            continue;
        }
        list.append(indexes.first()); //TODO handle all
        m_model->itemParentsChanged(indexes.first(), TopicsModel::IdList());
    }
}

void NepomukAdapter::propertyChanged(const Nepomuk::Resource &res, const Nepomuk::Types::Property &property, const QVariant &value)
{
    if (property.uri() == Soprano::Vocabulary::NAO::prefLabel()) {
        kDebug() << "renamed " << res.resourceUri() << " to " << value.toString();
        Q_ASSERT(m_topicMap.contains(res.resourceUri()));
        m_model->renameParent(m_topicMap[res.resourceUri()], value.toString());
    }
}

bool NepomukAdapter::onDropMimeData(const QMimeData* mimeData, Qt::DropAction action,  qint64 id)
{
    bool moveToTrash = false;
    QUrl targetTopic;
    if (id >= 0) {
        //kDebug() << "dropped on item " << data(parent, UriRole) << data(parent, Qt::DisplayRole).toString();
        targetTopic = m_topicMap.key(id);
        
    }
    /*
     i f (mimeData*->hasText()) { //plain text is interpreted as topic anyway, so you can drag a textfragment from anywhere and create a new topic when dropped
     const QUrl &sourceTopic = QUrl(mimeData->text());
     //beginResetModel();
     if (targetTopic.isValid()) {
         kDebug() << "set topic: " << targetTopic << " on dropped topic: " << sourceTopic;
         NepomukUtils::moveToTopic(sourceTopic, targetTopic);
} else {
    kDebug() << "remove all topics from topic:" << sourceTopic;
    NepomukUtils::removeAllTopics(sourceTopic);
}
//endResetModel(); //TODO emit item move instead
return true;
}*/
    
    //TODO support also drop of other urls (files), and add to topic contextview?
    
    if (!mimeData->hasUrls()) {
        kWarning() << "no urls in drop";
        return false;
    }
    kDebug() << mimeData->urls();
    
    foreach (const KUrl &url, mimeData->urls()) {
        const Akonadi::Item item = Akonadi::Item::fromUrl(url);
        if (!item.isValid()) {
            kDebug() << "invalid item";
            continue;
        }
        
        if (targetTopic.isValid()) {
            kDebug() << "set topic: " << targetTopic << " on dropped item: " << item.url();
            NepomukUtils::moveToTopic(item, targetTopic);
        } else {
            kDebug() << "remove all topics from item:" << item.url();
            NepomukUtils::removeAllTopics(item);
        }
        
    }
    return true;
}

bool NepomukAdapter::onSetData(qint64 id, const QVariant &value, int role) {
    QUrl targetTopic = m_topicMap.key(id);
    if (!targetTopic.isValid()) {
        kWarning() << "tried to rename invalid topic";
        return false;
    }
    NepomukUtils::renameTopic(targetTopic, value.toString());
    return true;
}