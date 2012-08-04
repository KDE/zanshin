/* This file is part of Zanshin Todo.

   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/


#include "searchfiltercacheproxy.h"

#include <Nepomuk2/Query/Query>
#include <Nepomuk2/Types/Class>
#include <Nepomuk2/Query/ResourceTypeTerm>
#include <Nepomuk2/Query/GroupTerm>
#include <Nepomuk2/Query/AndTerm>
#include <Nepomuk2/Query/LiteralTerm>
#include <Nepomuk2/Vocabulary/NIE>
#include <Nepomuk2/Vocabulary/NCAL>
#include <Nepomuk2/Vocabulary/PIMO>
#include <Nepomuk2/Vocabulary/NFO>

#include <aneo.h>

#include <nepomuk2/orterm.h>
#include <nepomuk2/comparisonterm.h>
#include <Nepomuk2/Tag>
#include <Nepomuk2/Query/QueryServiceClient>
#include <Nepomuk2/Variant>
#include "abstractpimitem.h"
#include <Nepomuk2/Query/Result>

#include "queries.h"
#include <todoproxymodelbase.h>
#include <nepomuk2/resourcewatcher.h>
#include <Soprano/Vocabulary/NAO>

SearchFilterCache::SearchFilterCache(QObject* parent)
: KIdentityProxyModel(parent),
    m_queryServiceClient(new Nepomuk2::Query::QueryServiceClient(this))
//     m_topicQueryServiceClient(new Nepomuk2::Query::QueryServiceClient(this))
{
    //Fulltext queries
    connect(m_queryServiceClient, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)), this, SLOT(newFulltextMatches(QList<Nepomuk2::Query::Result>)));
    connect(m_queryServiceClient, SIGNAL(entriesRemoved(QList<QUrl>)), this, SLOT(entriesRemoved(QList<QUrl>)));
    connect(m_queryServiceClient, SIGNAL(finishedListing()), this, SLOT(queryFinished()));
/*    
    connect(m_topicQueryServiceClient, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)), this, SLOT(newTopicMatches(QList<Nepomuk2::Query::Result>)));
    connect(m_topicQueryServiceClient, SIGNAL(entriesRemoved(QList<QUrl>)), this, SLOT(topicMatchRemoved(QList<QUrl>)));*/
    
    Nepomuk2::ResourceWatcher *watcher = new Nepomuk2::ResourceWatcher(this);
    watcher->addType(Nepomuk2::Types::Class(Nepomuk2::Vocabulary::NCAL::Todo()));
    watcher->addType(Nepomuk2::Types::Class(Nepomuk2::Vocabulary::NCAL::Event()));
    watcher->addType(Nepomuk2::Types::Class(Nepomuk2::Vocabulary::NFO::HtmlDocument()));
    watcher->addProperty(Nepomuk2::Types::Property(Nepomuk2::Vocabulary::PIMO::isRelated()));
    watcher->addProperty(Nepomuk2::Types::Property(Soprano::Vocabulary::NAO::hasTag()));
    watcher->addProperty(Nepomuk2::Types::Property(Nepomuk2::Vocabulary::NIE::plainTextContent()));
    watcher->addProperty(Nepomuk2::Types::Property(Nepomuk2::Vocabulary::NIE::title()));
    connect(watcher, SIGNAL(propertyAdded(Nepomuk2::Resource,Nepomuk2::Types::Property,QVariant)), this, SLOT(itemChanged(Nepomuk2::Resource,Nepomuk2::Types::Property,QVariant)));
    connect(watcher, SIGNAL(propertyRemoved(Nepomuk2::Resource,Nepomuk2::Types::Property,QVariant)), this, SLOT(itemChanged(Nepomuk2::Resource,Nepomuk2::Types::Property,QVariant)));
}

//TODO connect to resource watcher
void SearchFilterCache::itemChanged(const Nepomuk2::Resource &resource, const Nepomuk2::Types::Property &property, const QVariant &value)
{
    Q_UNUSED(value);
    Q_UNUSED(property)
    const Akonadi::Item &item = Akonadi::Item::fromUrl(resource.property(Nepomuk2::Vocabulary::NIE::url()).toUrl());
    if (!item.isValid()) {
        kWarning() << resource;
        return;
    }
    Q_ASSERT(item.isValid());
    kDebug() << item.id();
    const QModelIndexList &indexes = TodoProxyModelBase::modelIndexesForItem(this, item);
    if (indexes.isEmpty()) {
        kDebug() << "item not found" << item.url();
    }
    foreach (const QModelIndex &idx, indexes) {
        emit dataChanged(idx, idx);
    }
}

bool SearchFilterCache::isFulltextMatch(const Akonadi::Item &item ) const
{
    kDebug() << item.id() << m_fulltextHits.contains(item);
    return m_fulltextHits.contains(item);
}
/*
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

void SearchFilterCache::newTopicMatches(QList< Nepomuk2::Query::Result > results)
{
    foreach (const Nepomuk2::Query::Result &result, results) {
        const Nepomuk2::Variant &v = result.additionalBinding(QLatin1String("url"));
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
}*/

void SearchFilterCache::setFulltextSearch(const QString &string)
{
    kDebug() << string;
    m_fulltextHits.clear();

    if (string.isEmpty()) {
        return;
    }
    kDebug() << "run query";
    //TODO search tags from here
    using namespace Nepomuk2::Query;
    using namespace Nepomuk2::Types;
    using namespace Nepomuk2::Vocabulary;
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
    //query.addRequestProperty(Query::Query::RequestProperty(Nepomuk2::Vocabulary::NIE::url()));
    query.addRequestProperty(Query::Query::RequestProperty(Vocabulary::ANEO::akonadiItemId()));
    Q_ASSERT(query.isValid());
    if (!m_queryServiceClient->query(query)) { //This closes the old query automatically
        kWarning() << "failed ot start query";
    }
}

void SearchFilterCache::newFulltextMatches(QList< Nepomuk2::Query::Result > results)
{
    foreach (const Nepomuk2::Query::Result &result, results) {
        Akonadi::Entity::Id id = Nepomuk2::Variant::fromNode(result.requestProperty(Vocabulary::ANEO::akonadiItemId())).toString().toInt();
        Akonadi::Item item(id);
        Q_ASSERT(item.isValid());
//         kDebug() << item.id();
        const QModelIndexList &indexes = TodoProxyModelBase::modelIndexesForItem(this, item);
        if (indexes.isEmpty()) { //can happen if the item is in a non monitored collection
//             kDebug() << "could not find item";
            continue;
        }
        m_fulltextHits << item;
        foreach (const QModelIndex &idx, indexes) {
            emit dataChanged(idx, idx);
        }
    }
}

void SearchFilterCache::entriesRemoved(QList< QUrl > /*removedResources*/)
{
    //TODO
    kWarning() << "why are entries removed?";
}

void SearchFilterCache::queryFinished()
{
//     kDebug();
}



