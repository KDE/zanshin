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

#include "parentstructurestrategy.h"
#include "parentstructuremodel.h"

#include <queries.h>
#include <pimitem.h>
#include <tagmanager.h>
#include "globaldefs.h"
#include <Nepomuk2/Query/Query>
#include <Nepomuk2/Query/QueryServiceClient>
#include <Nepomuk2/Query/Result>
#include <Nepomuk2/Types/Class>
#include <Nepomuk2/Resource>
#include <Nepomuk2/Vocabulary/PIMO>
#include <KDebug>
#include <KIcon>
#include <nepomuk2/resourcetypeterm.h>
#include <nepomuk2/resourcewatcher.h>
#include <Soprano/Vocabulary/NAO>
#include <QMimeData>

ParentStructureStrategy::ParentStructureStrategy(QObject* parent): QObject(parent), m_model(0)
{

}


void ParentStructureStrategy::setModel(ParentStructureModel* model)
{
    m_model = model;
}


TestParentStructureStrategy::TestParentStructureStrategy(QObject* parent)
: ParentStructureStrategy(parent)
{

}


ParentStructureModel::IdList TestParentStructureStrategy::onSourceInsertRow(const QModelIndex &sourceChildIndex)
{
    if (!sourceChildIndex.isValid()) {
        kWarning() << "invalid indexx";
        return ParentStructureModel::IdList();
    }

    if (!sourceChildIndex.data(TopicParentRole).isValid()) {
        return ParentStructureModel::IdList();
    }
    const ParentStructureModel::Id &parent = sourceChildIndex.data(TopicParentRole).value<ParentStructureModel::Id>();

    return ParentStructureModel::IdList() << parent;
}

ParentStructureModel::IdList TestParentStructureStrategy::onSourceDataChanged(const QModelIndex &sourceIndex)
{
    return onSourceInsertRow(sourceIndex);
}


void TestParentStructureStrategy::addParent(const ParentStructureModel::Id& identifier, const ParentStructureModel::Id& parentIdentifier, const QString& name)
{
    kDebug() << identifier << parentIdentifier << name;
    m_model->createOrUpdateParent(identifier, parentIdentifier, name);
}

void TestParentStructureStrategy::setParent(const QModelIndex &item, const qint64& parentIdentifier)
{
    m_model->itemParentsChanged(item, ParentStructureModel::IdList() << parentIdentifier);
}


void TestParentStructureStrategy::removeParent(const ParentStructureModel::Id& identifier)
{
    m_model->removeNode(identifier);
}



NepomukParentStructureStrategy::NepomukParentStructureStrategy(QObject* parent)
:   ParentStructureStrategy(parent),
    m_counter(0),
    m_queryServiceClient(0)
{
    setType(Nepomuk2::Vocabulary::PIMO::Topic());
}

void NepomukParentStructureStrategy::init()
{
    if (m_queryServiceClient) {
        disconnect(m_queryServiceClient, 0, 0, 0);
        m_queryServiceClient->deleteLater();
        m_queryServiceClient = 0;
    }
    Nepomuk2::Query::Query query;
    query.setTerm(Nepomuk2::Query::ResourceTypeTerm(Nepomuk2::Types::Class(m_type)));
    
    query.addRequestProperty(Nepomuk2::Query::Query::RequestProperty(Nepomuk2::Vocabulary::PIMO::superTopic()));
    
    m_queryServiceClient = new Nepomuk2::Query::QueryServiceClient(this);
    connect(m_queryServiceClient, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)), this, SLOT(checkResults(QList<Nepomuk2::Query::Result>)));
    connect(m_queryServiceClient, SIGNAL(finishedListing()), this, SLOT(queryFinished()));
    connect(m_queryServiceClient, SIGNAL(entriesRemoved(QList<QUrl>)), this, SLOT(removeResult(QList<QUrl>)));
    if ( !m_queryServiceClient->query(query) ) {
        kWarning() << "error";
    }
}


void NepomukParentStructureStrategy::setType(const QUrl &type)
{
    m_type = type;
    
}

void NepomukParentStructureStrategy::checkResults(const QList< Nepomuk2::Query::Result > &results)
{
    //kDebug() <<  results.size() << results.first().resource().uri() << results.first().resource().label() << results.first().resource().types() << results.first().resource().className();
    foreach (const Nepomuk2::Query::Result &result, results) {
        Nepomuk2::Resource res(result.resource().uri());
        const QUrl parent = result.requestProperty(Nepomuk2::Vocabulary::PIMO::superTopic()).uri();
        kDebug() << res.uri() << res.label() << res.types() <<  parent;
        if (res.types().contains(m_type)) {
            if (parent.isValid()) {
                addParent(res, parent);
            } else {
                addParent(res);
            }
        } else {
            kWarning() << "unknown result " << res.types();
        }
    }
}


void NepomukParentStructureStrategy::addParent (const Nepomuk2::Resource& topic, const QUrl &parent)
{
//     kDebug() << "add topic" << topic.label() << topic.uri() << parent;
    if (parent.isValid() && !m_topicMap.contains(parent)) {
        addParent(parent);
    }
    QObject *guard = new QObject(this);
    m_guardMap[topic.uri()] = guard;
    
    Nepomuk2::ResourceWatcher *m_resourceWatcher = new Nepomuk2::ResourceWatcher(guard);
    m_resourceWatcher->addResource(topic);
    m_resourceWatcher->addProperty(Soprano::Vocabulary::NAO::prefLabel());
    connect(m_resourceWatcher, SIGNAL(propertyAdded(Nepomuk2::Resource,Nepomuk2::Types::Property,QVariant)), this, SLOT(propertyChanged(Nepomuk2::Resource,Nepomuk2::Types::Property,QVariant)));
    m_resourceWatcher->start();
    
    Nepomuk2::Query::QueryServiceClient *queryServiceClient = new Nepomuk2::Query::QueryServiceClient(guard);
    queryServiceClient->setProperty("resourceuri", topic.uri());
    connect(queryServiceClient, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)), this, SLOT(itemsWithTopicAdded(QList<Nepomuk2::Query::Result>)));
    connect(queryServiceClient, SIGNAL(entriesRemoved(QList<QUrl>)), this, SLOT(itemsFromTopicRemoved(QList<QUrl>)));
    connect(queryServiceClient, SIGNAL(finishedListing()), this, SLOT(queryFinished()));
    if ( !queryServiceClient->sparqlQuery(MindMirrorQueries::itemsWithTopicsQuery(QList <QUrl>() << topic.uri())) ) {
        kWarning() << "error";
    }
    qint64 id = -1;
    if (m_topicMap.contains(topic.uri())) {
        id = m_topicMap[topic.uri()];
    } else {
        id = m_counter++;
        m_topicMap.insert(topic.uri(), id);
    }
    qint64 pid = -1;
    if (m_topicMap.contains(parent)) {
        pid = m_topicMap[parent];
    }
    m_model->createOrUpdateParent(id, pid, topic.label());
}

void NepomukParentStructureStrategy::onNodeRemoval(const qint64& id)
{
//     kDebug() << id;
    const QUrl &targetTopic = m_topicMap.key(id);
    if (targetTopic.isValid()) {
        NepomukUtils::deleteTopic(targetTopic); //TODO delete subtopics with subresource handling?
    }
}


void NepomukParentStructureStrategy::removeResult(const QList<QUrl> &results)
{
    foreach (const QUrl &result, results) {
        Nepomuk2::Resource res(result);
//         kDebug() << res.uri() << res.label() << res.types() << res.className();
        if (res.types().contains(m_type)) {
            Q_ASSERT(m_topicMap.contains(res.uri()));
            m_model->removeNode(m_topicMap.take(res.uri())); //We remove it right here, because otherwise we would try to remove the topic again in onNodeRemoval
            m_guardMap.take(res.uri())->deleteLater();
        } else {
            kWarning() << "unknown result " << res.types();
        }
    }
}

void NepomukParentStructureStrategy::queryFinished()
{
    kWarning();
}


void NepomukParentStructureStrategy::itemsWithTopicAdded(const QList<Nepomuk2::Query::Result> &results)
{
    const QUrl &parent = sender()->property("resourceuri").toUrl();
//     kDebug() << parent;
    
    QModelIndexList list;
    foreach (const Nepomuk2::Query::Result &result, results) {
        Nepomuk2::Resource res = Nepomuk2::Resource(result.resource().uri());
//         kDebug() << res.uri() << res.label() << res.types() << res.className();
        const Akonadi::Item item = PimItemUtils::getItemFromResource(res);
        if (!item.isValid()) {
            kWarning() << "invalid Item";
            continue;
        }
        Q_ASSERT(m_topicMap.contains(parent));
        m_topicCache.insert(item.url(),  ParentStructureModel::IdList() << m_topicMap[parent]); //TODO preserve existing topics (multi topic items)
        //If the index is already available change it right away
        const QModelIndexList &indexes = Akonadi::EntityTreeModel::modelIndexesForItem(m_model->sourceModel(), item);
        if (indexes.isEmpty()) {
            kWarning() << "item not found" << item.url() << m_topicMap[parent];
            continue;
        }
        list.append(indexes.first()); //TODO hanle all
        m_model->itemParentsChanged(indexes.first(), m_topicCache.value(item.url()));
    }
}

QList< qint64 > NepomukParentStructureStrategy::onSourceInsertRow(const QModelIndex& sourceChildIndex)
{
    const Akonadi::Item &item = sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
//     kDebug() << item.url() << m_topicCache.value(item.url());
    return m_topicCache.value(item.url());
//     return ParentStructureStrategy::onSourceInsertRow(sourceChildIndex);
}

QList< qint64 > NepomukParentStructureStrategy::onSourceDataChanged(const QModelIndex& changed)
{
    return onSourceInsertRow(changed);
}

void NepomukParentStructureStrategy::itemsFromTopicRemoved(const QList<QUrl> &items)
{
    const QUrl &topic = sender()->property("topic").toUrl();
    kDebug() << "removing nodes from topic: " << topic;
    QModelIndexList list;
    foreach (const QUrl &uri, items) {
        Nepomuk2::Resource res = Nepomuk2::Resource(uri);
        const Akonadi::Item item = PimItemUtils::getItemFromResource(res);
        if (!item.isValid()) {
            continue;
        }
        kDebug() << item.url();
        m_topicCache.remove(item.url()); //TODO preserve other topics
        const QModelIndexList &indexes = Akonadi::EntityTreeModel::modelIndexesForItem(m_model->sourceModel(), item);
        if (indexes.isEmpty()) {
            kDebug() << "item not found" << item.url();
            continue;
        }
        list.append(indexes.first()); //TODO handle all
        m_model->itemParentsChanged(indexes.first(), m_topicCache.value(item.url()));
    }
}

void NepomukParentStructureStrategy::propertyChanged(const Nepomuk2::Resource &res, const Nepomuk2::Types::Property &property, const QVariant &value)
{
    if (property.uri() == Soprano::Vocabulary::NAO::prefLabel()) {
        kDebug() << "renamed " << res.uri() << " to " << value.toString();
        Q_ASSERT(m_topicMap.contains(res.uri()));
        m_model->renameParent(m_topicMap[res.uri()], value.toString());
    }
    if (property.uri() == Nepomuk2::Vocabulary::PIMO::superTopic()) {
        //TODO handle move of topic
    }
}

bool NepomukParentStructureStrategy::onDropMimeData(const QMimeData* mimeData, Qt::DropAction /*action*/,  qint64 id)
{
    QUrl targetTopic;
    if (id >= 0) {
        //kDebug() << "dropped on item " << data(parent, UriRole) << data(parent, Qt::DisplayRole).toString();
        targetTopic = m_topicMap.key(id);
        
    }
    
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

bool NepomukParentStructureStrategy::onSetData(qint64 id, const QVariant &value, int /*role*/) {
    const QUrl &targetTopic = m_topicMap.key(id);
    if (!targetTopic.isValid()) {
        kWarning() << "tried to rename invalid topic";
        return false;
    }
    NepomukUtils::renameTopic(targetTopic, value.toString());
    return true;
}

void NepomukParentStructureStrategy::setData(TodoNode* node, qint64 id)
{
    node->setData(KIcon("view-pim-notes"), 0, Qt::DecorationRole);
    node->setRowData(Zanshin::Topic, Zanshin::ItemTypeRole);
    node->setRowData(m_topicMap.key(id), Zanshin::UriRole);
}

void NepomukParentStructureStrategy::reset()
{
    m_topicCache.clear();
    m_topicMap.clear();
    m_guardMap.clear();
    m_counter = 0;
    ParentStructureStrategy::reset();
}

