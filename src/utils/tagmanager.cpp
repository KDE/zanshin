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

#include <Nepomuk2/Resource>
#include <Nepomuk2/Variant>
#include <Nepomuk2/Types/Class>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDF>

#include <nepomuk2/pimo.h>

#include <nepomuk2/resourcewatcher.h>
#include <nepomuk2/datamanagement.h>
#include <nepomuk2/storeresourcesjob.h>


#include <QStringList>

#include <akonadi/item.h>

#include "abstractpimitem.h"
#include <nepomuk2/simpleresource.h>
#include <nepomuk2/simpleresourcegraph.h>
#include <Nepomuk2/Vocabulary/NIE>
#include <KJob>
#include <Nepomuk2/Query/QueryServiceClient>
#include <Nepomuk2/Query/Result>

#include <nepomuk2/createresourcejob.h>

#include "queries.h"
#include "tagmanager_p.h"


namespace NepomukUtils {


KJob *addToTopic(const Akonadi::Item& item, const QUrl& topicId)
{
    if (!item.isValid()) {
        kDebug() << "invalid item";
        return 0;
    }
    kDebug() << "adding item to topic " << item.url() << topicId;
    Nepomuk2::SimpleResourceGraph graph;
    Nepomuk2::SimpleResource topic(topicId);
    topic.addType(Nepomuk2::Vocabulary::PIMO::Topic());
    Nepomuk2::SimpleResource thing = itemThing(item, graph);
    thing.addProperty(Nepomuk2::Vocabulary::PIMO::isRelated(), topic.uri());
    graph << topic << thing;
    return graph.save();
}

// TODO all topics MUST either have a pimo:hasSuperTopic or be defined as root topic with pimo:hasRootTopic
KJob *createTopic(const QString& topicName, const QUrl& supertopicId)
{
    kDebug() << "creating topic " << topicName << supertopicId;
    Nepomuk2::SimpleResourceGraph graph;
    Nepomuk2::SimpleResource topic;
    topic.addType(Nepomuk2::Vocabulary::PIMO::Topic());
    topic.setProperty(Soprano::Vocabulary::NAO::prefLabel(),topicName);
    topic.setProperty(Soprano::Vocabulary::NAO::description(),"A MindMirror Topic");
    
    if (supertopicId.isValid()) {
        kDebug() << "adding to supertopic: " << supertopicId;
        Nepomuk2::SimpleResource superTopic(supertopicId);
        superTopic.addType(Nepomuk2::Vocabulary::PIMO::Topic());
        topic.setProperty(Nepomuk2::Vocabulary::PIMO::superTopic(), superTopic.uri());
        graph << superTopic;
    }
    graph << topic;
    
    return graph.save();
}

KJob* deleteTopic(const QUrl& topicId)
{
    kDebug() << "removing topic: " << topicId;
    return Nepomuk2::removeResources(QList<QUrl>() << topicId, Nepomuk2::RemoveSubResoures);
}

KJob *moveToTopic(const Akonadi::Item& item, const QUrl& topicId)
{
    if (!item.isValid()) {
        kDebug() << "invalid item";
        return 0;
    }
    kDebug() << "moving item to topic " << item.url() << topicId;
    Nepomuk2::SimpleResourceGraph graph;
    Nepomuk2::SimpleResource topic(topicId);
    topic.addType(Nepomuk2::Vocabulary::PIMO::Topic());
    Nepomuk2::SimpleResource thing = itemThing(item, graph);
    thing.addProperty(Nepomuk2::Vocabulary::PIMO::isRelated(), topic.uri());
    graph << topic << thing;
    return Nepomuk2::storeResources(graph, Nepomuk2::IdentifyNew, Nepomuk2::OverwriteProperties);
}

KJob *moveToTopic(const QUrl& topicId, const QUrl& supertopicId)
{
    kDebug() << "moving " << topicId << " to " << supertopicId;
    if (!topicId.isValid() || !supertopicId.isValid()) {
        return 0;
    }
    
    Nepomuk2::SimpleResourceGraph graph;
    Nepomuk2::SimpleResource topic(topicId);
    
    Nepomuk2::SimpleResource superTopic(supertopicId);
    superTopic.addType(Nepomuk2::Vocabulary::PIMO::Topic());
    graph << superTopic;
    topic.setProperty(Nepomuk2::Vocabulary::PIMO::superTopic(), superTopic.uri());
    return Nepomuk2::storeResources(graph, Nepomuk2::IdentifyNew, Nepomuk2::OverwriteProperties);
}

KJob *removeAllTopics(const Akonadi::Item& item)
{
    KJob *job = new RemoveAllTopicsJob(item);
    job->start();
    return job;
}

KJob *removeAllTopics(const QUrl& topicId)
{
    return Nepomuk2::removeProperties(QList<QUrl>() << topicId, QList<QUrl>() << Nepomuk2::Vocabulary::PIMO::superTopic());
}

KJob *addToTopic(const QUrl& subtopicId, const QUrl& supertopicId)
{
    kDebug() << "supertopic: " << supertopicId << " sub: " << subtopicId;
    return Nepomuk2::addProperty(QList<QUrl>() << subtopicId, Nepomuk2::Vocabulary::PIMO::superTopic(), QVariantList() << supertopicId);
}


KJob *renameTopic(const QUrl& topicId, const QString& name)
{
    kDebug() << "rename " << topicId << " to " << name;
    return Nepomuk2::setProperty(QList<QUrl>() << topicId, Soprano::Vocabulary::NAO::prefLabel(), QVariantList() << name);
}

KJob* tagItem(const Akonadi::Item &item, const QString& tagstring)
{
    if (!item.isValid()) {
        kDebug() << "invalid item";
        return 0;
    }
    Nepomuk2::SimpleResourceGraph graph;

    Nepomuk2::SimpleResource tag;
    tag.addType(Soprano::Vocabulary::NAO::Tag());
    tag.addProperty(Soprano::Vocabulary::NAO::identifier(), tagstring);
    tag.addProperty(Soprano::Vocabulary::NAO::prefLabel(), tagstring);
  
    Nepomuk2::SimpleResource thing = itemThing(item, graph);
    thing.addProperty(Soprano::Vocabulary::NAO::hasTag(), tag.uri());
    graph << tag << thing;
    return graph.save();
}

Nepomuk2::SimpleResource itemThing(const Akonadi::Item &item, Nepomuk2::SimpleResourceGraph &graph)
{
    Nepomuk2::SimpleResource resource;
    resource.setProperty( Nepomuk2::Vocabulary::NIE::url(), QUrl(item.url()) );
    graph << resource;
    Nepomuk2::SimpleResource thing;
    thing.addType(Nepomuk2::Vocabulary::PIMO::Thing());
    thing.addProperty(Nepomuk2::Vocabulary::PIMO::groundingOccurrence(), resource.uri());
    return thing;
}

}


