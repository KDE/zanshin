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


#include "nepomukparentstructurestrategy.h"
#include "reparentingmodel.h"

#include <todonode.h>

#include <Nepomuk/Vocabulary/PIMO>
#include <Nepomuk/Query/Query>
#include <Nepomuk/Resource>
#include <nepomuk/resourcetypeterm.h>
#include <Soprano/Vocabulary/NAO>
#include <queries.h>
#include <pimitem.h>
#include <tagmanager.h>
#include "globaldefs.h"
#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Query/Result>
#include <Nepomuk/Types/Class>
#include <Nepomuk/Resource>
#include <Nepomuk/Vocabulary/PIMO>
#include <KDebug>
#include <nepomuk/resourcetypeterm.h>
#include <KIcon>
#include <nepomuk/resourcewatcher.h>
#include <Soprano/Vocabulary/NAO>
#include <QMimeData>

NepomukParentStructureStrategy::NepomukParentStructureStrategy()
:   QObject(),
    ReparentingStrategy(),
    m_queryServiceClient(0),
    mInbox(1),
    mRoot(2)
{
    setType(Nepomuk::Vocabulary::PIMO::Topic());
}

void NepomukParentStructureStrategy::init()
{
    ReparentingStrategy::init();
    TodoNode *node = createNode(mInbox, IdList(), "No Topic");
    node->setData(i18n("No Topic"), 0, Qt::DisplayRole);
    node->setData(KIcon("mail-folder-inbox"), 0, Qt::DecorationRole);
    node->setRowData(Zanshin::Inbox, Zanshin::ItemTypeRole);

    TodoNode *node2 = createNode(mRoot, IdList(), "Topics");
    node2->setData(i18n("Topics"), 0, Qt::DisplayRole);
    node2->setData(KIcon("document-multiple"), 0, Qt::DecorationRole);
    node2->setRowData(Zanshin::TopicRoot, Zanshin::ItemTypeRole);

    if (m_queryServiceClient) {
        disconnect(m_queryServiceClient, 0, 0, 0);
        m_queryServiceClient->deleteLater();
        m_queryServiceClient = 0;
    }
    Nepomuk::Query::Query query;
    query.setTerm(Nepomuk::Query::ResourceTypeTerm(Nepomuk::Types::Class(m_type)));

    query.addRequestProperty(Nepomuk::Query::Query::RequestProperty(Nepomuk::Vocabulary::PIMO::superTopic()));

    m_queryServiceClient = new Nepomuk::Query::QueryServiceClient(this);
    connect(m_queryServiceClient, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)), this, SLOT(checkResults(QList<Nepomuk::Query::Result>)));
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

void NepomukParentStructureStrategy::checkResults(const QList< Nepomuk::Query::Result > &results)
{
    //kDebug() <<  results.size() << results.first().resource().resourceUri() << results.first().resource().label() << results.first().resource().types() << results.first().resource().className();
    foreach (const Nepomuk::Query::Result &result, results) {
        Nepomuk::Resource res(result.resource().resourceUri());
        const QUrl parent = result.requestProperty(Nepomuk::Vocabulary::PIMO::superTopic()).uri();
//         kDebug() << res.resourceUri() << res.label() << res.types() << res.className() << parent;
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


void NepomukParentStructureStrategy::addParent (const Nepomuk::Resource& topic, const QUrl &parent)
{
//     kDebug() << "add topic" << topic.label() << topic.uri() << parent;
    if (parent.isValid() && !m_topicMap.contains(parent)) {
        addParent(parent);
    }
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
    if ( !queryServiceClient->sparqlQuery(MindMirrorQueries::itemsWithTopicsQuery(QList <QUrl>() << topic.resourceUri())) ) {
        kWarning() << "error";
    }
    qint64 id = -1;
    if (m_topicMap.contains(topic.resourceUri())) {
        id = m_topicMap[topic.resourceUri()];
    } else {
        id = getNextId();
        m_topicMap.insert(topic.resourceUri(), id);
    }
    Q_ASSERT(id >= 0);
    IdList parents;
    if (m_topicMap.contains(parent)) {
        parents << m_topicMap[parent];
    } else {
        parents << mRoot;
    }
    createNode(id, parents, topic.label());
}

void NepomukParentStructureStrategy::onNodeRemoval(const qint64& id)
{
//     kDebug() << id;
//     const QUrl &targetTopic = m_topicMap.key(id);
//     if (targetTopic.isValid()) {
//         NepomukUtils::deleteTopic(targetTopic); //TODO delete subtopics with subresource handling?
//     }
}

void NepomukParentStructureStrategy::removeResult(const QList<QUrl> &results)
{
    foreach (const QUrl &result, results) {
        Nepomuk::Resource res(result);
//         kDebug() << res.resourceUri() << res.label() << res.types() << res.className();
        if (res.types().contains(m_type)) {
            Q_ASSERT(m_topicMap.contains(res.resourceUri()));
            removeNode(m_topicMap.take(res.resourceUri())); //We remove it right here, because otherwise we would try to remove the topic again in onNodeRemoval
            m_guardMap.take(res.resourceUri())->deleteLater();
        } else {
            kWarning() << "unknown result " << res.types();
        }
    }
}

void NepomukParentStructureStrategy::queryFinished()
{
//     kWarning();
}

void NepomukParentStructureStrategy::itemsWithTopicAdded(const QList<Nepomuk::Query::Result> &results)
{
    const QUrl &parent = sender()->property("resourceuri").toUrl();
//     kDebug() << parent;

    foreach (const Nepomuk::Query::Result &result, results) {
        const Nepomuk::Resource &res = Nepomuk::Resource(result.resource().resourceUri());
//         kDebug() << res.resourceUri() << res.label() << res.types() << res.className();
        const Akonadi::Item item = PimItemUtils::getItemFromResource(res);
        if (!item.isValid()) {
            kWarning() << "invalid Item";
            continue;
        }
        Q_ASSERT(m_topicMap.contains(parent));
        m_topicCache.insert(item.url(),  IdList() << m_topicMap[parent]); //TODO preserve existing topics (multi topic items)
        //If the index is already available change it right away
        const QModelIndexList &indexes = Akonadi::EntityTreeModel::modelIndexesForItem(m_model->sourceModel(), item);
        if (indexes.isEmpty()) {
            kWarning() << "item not found" << item.url() << m_topicMap[parent];
            continue;
        }

        updateParents(getId(indexes.first()), m_topicCache.value(item.url())); //TODO handle all
    }
}

Id NepomukParentStructureStrategy::getId(const QModelIndex &sourceIndex)
{
    const Akonadi::Item &item = sourceIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    if (!m_itemIdMap.contains(item.id())) {
        m_itemIdMap.insert(item.id(), getNextId());
    }
//     kWarning() << item.url() << m_itemIdMap.value(item.id());
    return m_itemIdMap.value(item.id());
}

IdList NepomukParentStructureStrategy::getParents(const QModelIndex &sourceChildIndex, const IdList& ignore)
{
    const Akonadi::Item &item = sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
//     kDebug() << item.url() << m_topicCache.value(item.url());
    Q_ASSERT(item.isValid());
    IdList parents = m_topicCache.value(item.url());
    foreach(Id i, ignore) {
        parents.removeAll(i);
    }
    if (parents.isEmpty()) {
        parents << mInbox;
    }
    return parents;
}


void NepomukParentStructureStrategy::itemsFromTopicRemoved(const QList<QUrl> &items)
{
    const QUrl &topic = sender()->property("topic").toUrl();
//     kDebug() << "removing nodes from topic: " << topic;
    foreach (const QUrl &uri, items) {
        Nepomuk::Resource res = Nepomuk::Resource(uri);
        const Akonadi::Item item = PimItemUtils::getItemFromResource(res);
        if (!item.isValid()) {
            continue;
        }
//         kDebug() << item.url();
        m_topicCache.remove(item.url()); //TODO preserve other topics
        const QModelIndexList &indexes = Akonadi::EntityTreeModel::modelIndexesForItem(m_model->sourceModel(), item);
        if (indexes.isEmpty()) {
            kDebug() << "item not found" << item.url();
            continue;
        }
        updateParents(getId(indexes.first()), m_topicCache.value(item.url())); //TODO handle all
    }
}

void NepomukParentStructureStrategy::propertyChanged(const Nepomuk::Resource &res, const Nepomuk::Types::Property &property, const QVariant &value)
{
    if (property.uri() == Soprano::Vocabulary::NAO::prefLabel()) {
//         kDebug() << "renamed " << res.resourceUri() << " to " << value.toString();
        Q_ASSERT(m_topicMap.contains(res.resourceUri()));
        renameNode(m_topicMap[res.resourceUri()], value.toString());
    }
    if (property.uri() == Nepomuk::Vocabulary::PIMO::superTopic()) {
        //TODO handle move of topic
    }
}

bool NepomukParentStructureStrategy::onDropMimeData(Id id, const QMimeData *mimeData, Qt::DropAction )
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
//     kDebug() << mimeData->urls();

    foreach (const KUrl &url, mimeData->urls()) {
        const Akonadi::Item item = Akonadi::Item::fromUrl(url);
        if (!item.isValid()) {
            kDebug() << "invalid item";
            continue;
        }

        if (targetTopic.isValid()) {
//             kDebug() << "set topic: " << targetTopic << " on dropped item: " << item.url();
            NepomukUtils::moveToTopic(item, targetTopic);
        } else {
//             kDebug() << "remove all topics from item:" << item.url();
            NepomukUtils::removeAllTopics(item);
        }
    }
    return true;
}

bool NepomukParentStructureStrategy::onSetData(Id id, const QVariant &value, int /*role*/) {
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
    //TODO filter inbox & root
    node->setData(KIcon("view-pim-notes"), 0, Qt::DecorationRole);
    node->setRowData(Zanshin::Topic, Zanshin::ItemTypeRole);
    node->setRowData(m_topicMap.key(id), Zanshin::UriRole);
}

void NepomukParentStructureStrategy::reset()
{
    m_topicCache.clear();
    m_topicMap.clear();
    m_guardMap.clear();
    ReparentingStrategy::reset();
}

QStringList NepomukParentStructureStrategy::mimeTypes()
{
    QStringList list;
    list.append("text/uri-list");
    list.append("text/plain");
    return list;
}

Qt::ItemFlags NepomukParentStructureStrategy::flags(const QModelIndex &index, Qt::ItemFlags flags)
{
    Zanshin::ItemType type = (Zanshin::ItemType) index.data(Zanshin::ItemTypeRole).toInt();
    if (type == Zanshin::Inbox || type == Zanshin::TopicRoot) {
        return Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    }
    return flags | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
}


