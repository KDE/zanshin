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

#include "tagmanager.h"

#include <Nepomuk/Resource>
#include <Nepomuk/Variant>
#include <Nepomuk/Types/Class>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDF>

#include <nepomuk/pimo.h>

#include <nepomuk/resourcewatcher.h>
#include <nepomuk/datamanagement.h>

#include <aneo.h>

#include <QStringList>

#include <akonadi/item.h>

#include "abstractpimitem.h"
#include <nepomuk/simpleresource.h>
#include <nepomuk/simpleresourcegraph.h>
#include <Nepomuk/Vocabulary/NIE>
#include <KJob>
#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Query/Result>

#include <nepomuk/createresourcejob.h>

#include "queries.h"


namespace NepomukUtils {


KJob *addToTopic(const Akonadi::Item& item, const QUrl& topicId)
{
    if (!item.isValid()) {
        kDebug() << "invalid item";
        return 0;
    }
    Nepomuk::SimpleResourceGraph graph;
    Nepomuk::SimpleResource topic(topicId);
    topic.addType(Nepomuk::Vocabulary::PIMO::Topic()); //For some reason store resources otherwise complains about an invalid resource (although the resource was already created)
    Nepomuk::SimpleResource thing = itemThing(item, graph);
    thing.addProperty(Nepomuk::Vocabulary::PIMO::isRelated(), topic.uri());
    graph << topic << thing;
    return graph.save();
}

// TODO all topics MUST either have a pimo:hasSuperTopic or be defined as root topic with pimo:hasRootTopic
KJob *createTopic(const QString& topicName, const QUrl& supertopicId)
{
    return Nepomuk::createResource(QList<QUrl>() << Nepomuk::Vocabulary::PIMO::Topic(), topicName, "A MindMirror Topic");
}

KJob* deleteTopic(const QUrl& topicId)
{
    kDebug() << "removing topic: " << topicId;
    return Nepomuk::removeResources(QList<QUrl>() << topicId, Nepomuk::RemoveSubResoures);
}


/*
QStringList getTopicList(const Nepomuk::Thing& thing)
{
    QStringList list;
    QString query = QString::fromLatin1("select ?r where { ?r <%1> <%2>. <%4> <%3> ?r.}")
        .arg(Soprano::Vocabulary::RDF::type().toString())
        .arg(Nepomuk::Vocabulary::PIMO::Topic().toString())
        .arg(Nepomuk::Vocabulary::PIMO::isRelated().toString())
        .arg(thing.uri());
    QList<Nepomuk::Query::Result> results = Nepomuk::Query::QueryServiceClient::syncSparqlQuery(query);
    foreach (const Nepomuk::Query::Result &result, results) {
        list.append(result.resource().label());
    }
    return list;
}*/

QList<QUrl> getTopicList(const Akonadi::Item& item)
{
    QList<QUrl> list;
    QList<Nepomuk::Query::Result> results = Nepomuk::Query::QueryServiceClient::syncSparqlQuery(MindMirrorQueries::itemTopicsQuery(item));
    foreach (const Nepomuk::Query::Result &result, results) {
        list.append(result.resource().resourceUri());
    }
    return list;
}

QStringList getTopicNameList(const Akonadi::Item& item)
{
    QStringList list;
    QList<Nepomuk::Query::Result> results = Nepomuk::Query::QueryServiceClient::syncSparqlQuery(MindMirrorQueries::itemTopicsQuery(item));
    foreach (const Nepomuk::Query::Result &result, results) {
        list.append(result.resource().label());
    }
    return list;
}

KJob *moveToTopic(const Akonadi::Item& item, const QUrl& topicId)
{
    removeAllTopics(item);
    return addToTopic(item, topicId);
}

KJob *moveToTopic(const QUrl& topicId, const QUrl& supertopicId)
{
    removeAllTopics(topicId);
    return addToTopic(topicId, supertopicId);
}

KJob *removeAllTopics(const Akonadi::Item& item)
{
    QList<QUrl> list = getTopicList(item);
    if (list.isEmpty()) {
        return 0;
    }
    QList<Nepomuk::Query::Result> results = Nepomuk::Query::QueryServiceClient::syncSparqlQuery(MindMirrorQueries::itemThingQuery(item));
    const Nepomuk::Resource &thing = results.first().resource();
    Q_ASSERT(thing.isValid());
    kDebug() << "res: " << thing.resourceUri();
    QVariantList values;
    foreach (const QUrl &url, list) {
        kDebug() << url;
        values << url;
    }
    return Nepomuk::removeProperty(QList<QUrl>() << thing.resourceUri(), Nepomuk::Vocabulary::PIMO::isRelated(), values);
}

KJob *removeAllTopics(const QUrl& topicId)
{
    return Nepomuk::removeProperties(QList<QUrl>() << topicId, QList<QUrl>() << Nepomuk::Vocabulary::PIMO::superTopic());
}

KJob *addToTopic(const QUrl& subtopicId, const QUrl& supertopicId)
{
    kDebug() << "supertopic: " << supertopicId << " sub: " << subtopicId;
    return Nepomuk::addProperty(QList<QUrl>() << subtopicId, Nepomuk::Vocabulary::PIMO::superTopic(), QVariantList() << supertopicId);
}


KJob *renameTopic(const QUrl& topicId, const QString& name)
{
    kDebug() << "rename " << topicId << " to " << name;
    return Nepomuk::setProperty(QList<QUrl>() << topicId, Soprano::Vocabulary::NAO::prefLabel(), QVariantList() << name);
}

KJob* tagItem(const Akonadi::Item &item, const QString& tagstring)
{
    if (!item.isValid()) {
        kDebug() << "invalid item";
        return 0;
    }
    Nepomuk::SimpleResourceGraph graph;

    Nepomuk::SimpleResource tag;
    tag.addType(Soprano::Vocabulary::NAO::Tag());
    tag.addProperty(Soprano::Vocabulary::NAO::identifier(), tagstring);
    tag.addProperty(Soprano::Vocabulary::NAO::prefLabel(), tagstring);
  
    Nepomuk::SimpleResource thing = itemThing(item, graph);
    thing.addProperty(Soprano::Vocabulary::NAO::hasTag(), tag.uri());
    graph << tag << thing;
    return graph.save();
}

Nepomuk::SimpleResource itemThing(const Akonadi::Item &item, Nepomuk::SimpleResourceGraph &graph)
{
    Nepomuk::SimpleResource resource;
    resource.setProperty( Nepomuk::Vocabulary::NIE::url(), QUrl(item.url()) );
    graph << resource;
    Nepomuk::SimpleResource thing;
    thing.addType(Nepomuk::Vocabulary::PIMO::Thing());
    thing.addProperty(Nepomuk::Vocabulary::PIMO::groundingOccurrence(), resource.uri());
    return thing;
}

}


