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


#include "reparentingstrategy.h"
#include "globaldefs.h"
#include "reparentingmodel.h"
#include "todonode.h"
#include <klocalizedstring.h>
#include <KIcon>

ReparentingStrategy::ReparentingStrategy()
:   mReparentOnRemoval(true),
    mIdCounter(0)
{

}


IdList ReparentingStrategy::getParents(const QModelIndex &, const IdList & )
{
    return IdList();
}

void ReparentingStrategy::onNodeRemoval(const qint64& changed)
{

}

void ReparentingStrategy::setModel(ReparentingModel* model)
{
    m_model = model;
}

Id ReparentingStrategy::getNextId()
{
    return mIdCounter++;
}

TodoNode *ReparentingStrategy::createNode(Id id, Id pid, QString name)
{
    kDebug() << id << pid << name;
    return m_model->createNode(id, pid, name);
}

void ReparentingStrategy::renameNode(Id id, QString name)
{
    m_model->renameNode(id, name);
}

void ReparentingStrategy::updateParents(Id id, IdList parents)
{
    m_model->reparentNode(id, parents);
}

void ReparentingStrategy::removeNode(Id id)
{
    kDebug() << id;
    m_model->removeNodeById(id);
}


bool ReparentingStrategy::reparentOnRemoval(Id) const
{
    return mReparentOnRemoval;
}




TestReparentingStrategy::TestReparentingStrategy()
: ReparentingStrategy()
{

}

Id TestReparentingStrategy::getId(const QModelIndex &sourceChildIndex)
{
    if (!sourceChildIndex.data(IdRole).isValid()) {
        kWarning() << "error: missing idRole";
    }
    return sourceChildIndex.data(IdRole).value<Id>();
}

IdList TestReparentingStrategy::getParents(const QModelIndex &sourceChildIndex, const IdList &ignore)
{
    if (!sourceChildIndex.isValid()) {
        kWarning() << "invalid index";
        return IdList();
    }

    if (!sourceChildIndex.data(ParentRole).isValid()) {
        return IdList();
    }
    const Id &parent = sourceChildIndex.data(ParentRole).value<Id>();
    if (parent < 0) {
        return IdList();
    }
    if (ignore.contains(parent)) {
        return IdList();
    }

    return IdList() << parent;
}



ProjectStrategy::ProjectStrategy()
:   ReparentingStrategy(),
    mInbox(1)
{
    mIdCounter = 2;
    mReparentOnRemoval = false;
}

void ProjectStrategy::init()
{
    ReparentingStrategy::init();
    TodoNode *node = createNode(mInbox, -1, "Inbox");
    node->setData(i18n("Inbox"), 0, Qt::DisplayRole);
    node->setData(KIcon("mail-folder-inbox"), 0, Qt::DecorationRole);
    node->setRowData(Zanshin::Inbox, Zanshin::ItemTypeRole);
}


Id ProjectStrategy::getId(const QModelIndex &sourceChildIndex)
{
    Zanshin::ItemType type = (Zanshin::ItemType) sourceChildIndex.data(Zanshin::ItemTypeRole).toInt();
    if (type==Zanshin::Collection) {
        Akonadi::Collection::Id id = sourceChildIndex.data(Akonadi::EntityTreeModel::CollectionIdRole).value<Akonadi::Collection::Id>();
        if (!mCollectionMapping.contains(id)) {
            mCollectionMapping.insert(id, getNextId());
        }
//         kDebug() << "collection id: " << id << mCollectionMapping.value(id);
        return mCollectionMapping.value(id);
    }
    const QString &uid = sourceChildIndex.data(Zanshin::UidRole).toString();
    if (!mUidMapping.contains(uid)) {
        mUidMapping.insert(uid, getNextId());
    }
    return mUidMapping.value(uid);
}

IdList ProjectStrategy::getParents(const QModelIndex &sourceChildIndex, const IdList &ignore)
{
    Id id = getId(sourceChildIndex);
    Zanshin::ItemType type = (Zanshin::ItemType) sourceChildIndex.data(Zanshin::ItemTypeRole).toInt();
//     kDebug() << id << type;
    if (type==Zanshin::Collection) {
        const QModelIndex &parent = sourceChildIndex.parent();
        if (parent.isValid()) {
            return IdList() << getId(parent);
        }
        return IdList();
    }
    const QString &parentUid = sourceChildIndex.data(Zanshin::ParentUidRole).toString();
//     kDebug() << parentUid;
    if (type==Zanshin::ProjectTodo && parentUid.isEmpty()) {
//         kDebug() << "get source parent";
        const QModelIndex &parent = sourceChildIndex.parent();
        if (parent.isValid()) {
            return IdList() << getId(parent);
        }
        return IdList();
    } else if (type==Zanshin::StandardTodo && parentUid.isEmpty()) {
        return IdList() << mInbox;
    }
//     Q_ASSERT(type==Zanshin::StandardTodo);
    if (!mUidMapping.contains(parentUid)) {
        mUidMapping.insert(parentUid, getNextId());
       
    }
    return IdList() << mUidMapping.value(parentUid);
}

void ProjectStrategy::reset()
{
    mIdCounter = 2;
    mUidMapping.clear();
    mCollectionMapping.clear();
    
}








TestParentStructureStrategy::TestParentStructureStrategy(QObject* parent)
: ReparentingStrategy()
{
    mIdCounter = 3;
}

void TestParentStructureStrategy::init()
{
    ReparentingStrategy::init();
    TodoNode *node = createNode(997, -1, "No Topic");
    node->setData(i18n("No Topic"), 0, Qt::DisplayRole);
    node->setData(KIcon("mail-folder-inbox"), 0, Qt::DecorationRole);
    node->setRowData(Zanshin::Inbox, Zanshin::ItemTypeRole);

    TodoNode *node2 = createNode(998, -1, "Topics");
    node2->setData(i18n("Topics"), 0, Qt::DisplayRole);
    node2->setData(KIcon("document-multiple"), 0, Qt::DecorationRole);
    node2->setRowData(Zanshin::TopicRoot, Zanshin::ItemTypeRole);


}

bool TestParentStructureStrategy::reparentOnRemoval(Id id) const
{
    if (id < 900) {
        kDebug() << "reparent " << id;
        return false;
    }
    return true;
}


Id TestParentStructureStrategy::getId(const QModelIndex &sourceChildIndex)
{
    if (!sourceChildIndex.isValid()) {
        kWarning() << "invalid index";
        return -1;
    }
    return sourceChildIndex.data(102).value<qint64>()+1000;

//     Zanshin::ItemType type = static_cast<Zanshin::ItemType>(sourceChildIndex.data(Zanshin::ItemTypeRole).toInt());
//     if (type == Zanshin::Inbox) {
//         return 997;
//     }
//     if (type == Zanshin::TopicRoot) {
//         return 998;
//     }
//     
//     if (!sourceChildIndex.data(TopicRole).isValid()) {
// //         Q_ASSERT(sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>().isValid());
// //         return sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>().id()+1000;
//         Q_ASSERT(sourceChildIndex.data(102).canConvert<qint64>());
//         kDebug() << sourceChildIndex.data(102).value<qint64>();
//         return sourceChildIndex.data(102).value<qint64>()+1000;
//     }
//     Q_ASSERT(sourceChildIndex.data(TopicRole).canConvert(QVariant::Int));
//     kDebug() << sourceChildIndex.data(TopicRole).toInt();
//     return sourceChildIndex.data(TopicRole).toInt();
}

IdList TestParentStructureStrategy::getParents(const QModelIndex &sourceChildIndex, const IdList &ignore)
{
    Q_ASSERT(sourceChildIndex.isValid());

    if (!sourceChildIndex.data(TopicParentRole).isValid()) {
//         if (sourceChildIndex.data(TopicRole).isValid()) {
//             kWarning() << "topic role";
//             return IdList() << 998; //Topics
//         }
        return IdList() << 997; //No Topics
    }
    const Id &parent = sourceChildIndex.data(TopicParentRole).value<Id>();
    if (ignore.contains(parent)) {
        return IdList() << 997; //No Topics
    }
    return IdList() << parent;
}

void TestParentStructureStrategy::addParent(Id identifier, Id parentIdentifier, const QString& name)
{
    kDebug() << identifier << parentIdentifier << name;
    if (parentIdentifier < 0 ) {
        parentIdentifier = 998;
    }
    createNode(identifier, parentIdentifier, name);
}

void TestParentStructureStrategy::setParent(const QModelIndex &item, const qint64& parentIdentifier)
{
    updateParents(getId(item), IdList() << parentIdentifier);
}


void TestParentStructureStrategy::removeParent(const Id& identifier)
{
    removeNode(identifier);
}


#if 0
NepomukParentStructureStrategy::NepomukParentStructureStrategy(QObject* parent)
:
    QObject(parent),
    ParentStructureStrategy(),
    m_counter(0),
    m_queryServiceClient(0),
{
    setType(Nepomuk::Vocabulary::PIMO::Topic());
}

void NepomukParentStructureStrategy::init()
{
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
        kDebug() << res.resourceUri() << res.label() << res.types() << res.className() << parent;
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
        id = m_counter++;
        m_topicMap.insert(topic.resourceUri(), id);
    }
    qint64 pid = -1;
    if (m_topicMap.contains(parent)) {
        pid = m_topicMap[parent];
    }
    createNode(id, pid, topic.label());
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
        Nepomuk::Resource res(result);
//         kDebug() << res.resourceUri() << res.label() << res.types() << res.className();
        if (res.types().contains(m_type)) {
            Q_ASSERT(m_topicMap.contains(res.resourceUri()));
            m_model->removeNode(m_topicMap.take(res.resourceUri())); //We remove it right here, because otherwise we would try to remove the topic again in onNodeRemoval
            m_guardMap.take(res.resourceUri())->deleteLater();
        } else {
            kWarning() << "unknown result " << res.types();
        }
    }
}

void NepomukParentStructureStrategy::queryFinished()
{
    kWarning();
}


void NepomukParentStructureStrategy::itemsWithTopicAdded(const QList<Nepomuk::Query::Result> &results)
{
    const QUrl &parent = sender()->property("resourceuri").toUrl();
//     kDebug() << parent;

    QModelIndexList list;
    foreach (const Nepomuk::Query::Result &result, results) {
        Nepomuk::Resource res = Nepomuk::Resource(result.resource().resourceUri());
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
        list.append(indexes.first()); //TODO hanle all
        updateParents(indexes.first(), m_topicCache.value(item.url()));
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
        Nepomuk::Resource res = Nepomuk::Resource(uri);
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
        updateParents(indexes.first(), m_topicCache.value(item.url()));
    }
}

void NepomukParentStructureStrategy::propertyChanged(const Nepomuk::Resource &res, const Nepomuk::Types::Property &property, const QVariant &value)
{
    if (property.uri() == Soprano::Vocabulary::NAO::prefLabel()) {
        kDebug() << "renamed " << res.resourceUri() << " to " << value.toString();
        Q_ASSERT(m_topicMap.contains(res.resourceUri()));
        renameNode(m_topicMap[res.resourceUri()], value.toString());
    }
    if (property.uri() == Nepomuk::Vocabulary::PIMO::superTopic()) {
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

#endif

