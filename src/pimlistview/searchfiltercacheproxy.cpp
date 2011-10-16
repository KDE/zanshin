/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Christian Mollekopf <chrigi_1@fastmail.fm>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "searchfiltercacheproxy.h"

#include <Akonadi/EntityTreeModel>

#include <Nepomuk/Query/Query>
#include <Nepomuk/Types/Class>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/GroupTerm>
#include <Nepomuk/Query/AndTerm>
#include <Nepomuk/Query/LiteralTerm>
#include <Nepomuk/Vocabulary/NIE>
#include <Nepomuk/Vocabulary/NCAL>
#include <Nepomuk/Vocabulary/PIMO>
#include <Nepomuk/Vocabulary/NFO>

#include <aneo.h>

#include <nepomuk/orterm.h>
#include <nepomuk/comparisonterm.h>
#include <Nepomuk/Tag>
#include <Nepomuk/Query/QueryServiceClient>

#include "abstractpimitem.h"
#include "nepomukpropertycache.h"
#include <Nepomuk/Query/Result>

#include "queries.h"
#include <nepomuk/resourcewatcher.h>
#include <Soprano/Vocabulary/NAO>

SearchFilterCache::SearchFilterCache(QObject* parent)
: KIdentityProxyModelCopy(parent),
    m_queryServiceClient(new Nepomuk::Query::QueryServiceClient(this)),
    m_topicQueryServiceClient(new Nepomuk::Query::QueryServiceClient(this))
{
    connect(this, SIGNAL(modelReset()), this, SLOT(modelWasReset()));
    
    //Fulltext queries
    connect(m_queryServiceClient, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)), this, SLOT(newFulltextMatches(QList<Nepomuk::Query::Result>)));
    connect(m_queryServiceClient, SIGNAL(entriesRemoved(QList<QUrl>)), this, SLOT(entriesRemoved(QList<QUrl>)));
    connect(m_queryServiceClient, SIGNAL(finishedListing()), this, SLOT(queryFinished()));
    
    connect(m_topicQueryServiceClient, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)), this, SLOT(newTopicMatches(QList<Nepomuk::Query::Result>)));
    connect(m_topicQueryServiceClient, SIGNAL(entriesRemoved(QList<QUrl>)), this, SLOT(topicMatchRemoved(QList<QUrl>)));
    
    Nepomuk::ResourceWatcher *watcher = new Nepomuk::ResourceWatcher(this);
    watcher->addType(Nepomuk::Types::Class(Nepomuk::Vocabulary::NCAL::Todo()));
    watcher->addType(Nepomuk::Types::Class(Nepomuk::Vocabulary::NCAL::Event()));
    watcher->addType(Nepomuk::Types::Class(Nepomuk::Vocabulary::NFO::HtmlDocument()));
    watcher->addProperty(Nepomuk::Types::Property(Nepomuk::Vocabulary::PIMO::isRelated()));
    watcher->addProperty(Nepomuk::Types::Property(Soprano::Vocabulary::NAO::hasTag()));
    watcher->addProperty(Nepomuk::Types::Property(Nepomuk::Vocabulary::NIE::plainTextContent()));
    watcher->addProperty(Nepomuk::Types::Property(Nepomuk::Vocabulary::NIE::title()));
    connect(watcher, SIGNAL(propertyAdded(Nepomuk::Resource,Nepomuk::Types::Property,QVariant)), this, SLOT(itemChanged(Nepomuk::Resource,Nepomuk::Types::Property,QVariant)));
    connect(watcher, SIGNAL(propertyRemoved(Nepomuk::Resource,Nepomuk::Types::Property,QVariant)), this, SLOT(itemChanged(Nepomuk::Resource,Nepomuk::Types::Property,QVariant)));
}

//TODO connect to resource watcher
void SearchFilterCache::itemChanged(const Nepomuk::Resource &resource, const Nepomuk::Types::Property &property, const QVariant &value)
{
    Q_UNUSED(value);
    Q_UNUSED(property)
    const Akonadi::Item &item = Akonadi::Item::fromUrl(resource.property(Nepomuk::Vocabulary::NIE::url()).toUrl());
    if (!item.isValid()) {
        kWarning() << resource;
        return;
    }
    Q_ASSERT(item.isValid());
    kDebug() << item.id();
    const QModelIndexList &indexes = Akonadi::EntityTreeModel::modelIndexesForItem(this, item);
    if (indexes.isEmpty()) {
        kDebug() << "item not found" << item.url();
        return;
    }
    Q_ASSERT(indexes.size() == 1); //assumption that every item is only once shown in the list
    emit dataChanged(indexes.first(), indexes.first());
}

bool SearchFilterCache::isFulltextMatch(const Akonadi::Item &item ) const
{
    //kDebug() << item.id() << m_fulltextHits.contains(item);
    return m_fulltextHits.contains(item);
}

void SearchFilterCache::setTopicFilter(const QList<KUrl> &topicList, bool noTopic)
{
    m_topicHits.clear();
    if (noTopic) {
        kWarning() << "Not implemented";
    } else if (!topicList.isEmpty()) {
        QList<QUrl> list;
        foreach (const KUrl &url, topicList) {
            list << QUrl(url);
        }
        if (!m_topicQueryServiceClient->sparqlQuery(MindMirrorQueries::itemsWithTopicsQuery(list))) { //This closes the old query automatically
            kWarning() << "failed ot start query";
        }
    }
}

void SearchFilterCache::newTopicMatches(QList< Nepomuk::Query::Result > results)
{
    foreach (const Nepomuk::Query::Result &result, results) {
        const Nepomuk::Variant &v = result.additionalBinding(QLatin1String("url"));
        Akonadi::Item item = Akonadi::Item::fromUrl(v.toUrl());
        if (!item.isValid()) {
            kWarning() << "invalid item";
            return;
        }
        Q_ASSERT(item.isValid());
        kDebug() << item.id();
        const QModelIndexList &indexes = Akonadi::EntityTreeModel::modelIndexesForItem(this, item);
        if (indexes.isEmpty()) { //can happen if the item is in a non monitored collection
            continue;
        }
        m_topicHits << item;
        Q_ASSERT(indexes.size() == 1); //assumption that every item is only once shown in the list
        emit dataChanged(indexes.first(), indexes.first());
    }
}

void SearchFilterCache::topicMatchRemoved(QList< QUrl > removedResources)
{
    kWarning() << "why are entries removed?" << removedResources;
}


bool SearchFilterCache::isTopicMatch(const Akonadi::Item &item) const
{
    return m_topicHits.contains(item);
}

void SearchFilterCache::setFulltextSearch(const QString &string)
{
    kDebug() << string;
    m_fulltextHits.clear();
    
    if (string.isEmpty()) {
        return;
    }
    kDebug() << "run query";
    //TODO search tags from here
    using namespace Nepomuk::Query;
    using namespace Nepomuk::Types;
    using namespace Nepomuk::Vocabulary;
    //TODO take definitions from abstractpimitem, and take it there from a file shared with the feeder (i.e. NCAL::TODO)
    OrTerm resourcesTerm;
    resourcesTerm.addSubTerm(ResourceTypeTerm(Class(NCAL::Todo())));
    resourcesTerm.addSubTerm(ResourceTypeTerm(Class(NCAL::Event())));
    resourcesTerm.addSubTerm(ResourceTypeTerm(Class(NFO::HtmlDocument())));

    OrTerm textSearchTerm;
    //textSearchTerm.addSubTerm(LiteralTerm(string));
    textSearchTerm.addSubTerm(ComparisonTerm( NIE::title(), LiteralTerm(string)));
    textSearchTerm.addSubTerm(ComparisonTerm( NIE::plainTextContent(), LiteralTerm(string)));
    
    AndTerm outerGroup;
    outerGroup.addSubTerm(ResourceTypeTerm(Class(Vocabulary::ANEO::AkonadiDataObject())));
    outerGroup.addSubTerm(resourcesTerm);
    outerGroup.addSubTerm(textSearchTerm);
    Query query;
    query.setTerm( outerGroup );
    //query.addRequestProperty(Query::Query::RequestProperty(Nepomuk::Vocabulary::NIE::url()));
    query.addRequestProperty(Query::Query::RequestProperty(Vocabulary::ANEO::akonadiItemId()));
    Q_ASSERT(query.isValid());
    if (!m_queryServiceClient->query(query)) { //This closes the old query automatically
        kWarning() << "failed ot start query";
    }
}

void SearchFilterCache::newFulltextMatches(QList< Nepomuk::Query::Result > results)
{
    kDebug();
    foreach (const Nepomuk::Query::Result &result, results) {
        Entity::Id id = Nepomuk::Variant::fromNode(result.requestProperty(Vocabulary::ANEO::akonadiItemId())).toString().toInt();
        Akonadi::Item item(id);
        Q_ASSERT(item.isValid());
        kDebug() << item.id();
        const QModelIndexList &indexes = Akonadi::EntityTreeModel::modelIndexesForItem(this, item);
        if (indexes.isEmpty()) { //can happen if the item is in a non monitored collection
            continue;
        }
        m_fulltextHits << item;
        Q_ASSERT(indexes.size() == 1); //assumption that every item is only once shown in the list
        emit dataChanged(indexes.first(), indexes.first());
    }
}

void SearchFilterCache::entriesRemoved(QList< QUrl > removedResources)
{
    //TODO
    kWarning() << "why are entries removed?";
}

void SearchFilterCache::queryFinished()
{
    kDebug();
}



