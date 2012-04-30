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

#include "queries.h"

#include <QStringList>
#include <QRegExp>

#include <Nepomuk/Tag>
#include <Nepomuk/Thing>
#include <nepomuk/simpleresource.h>
#include <nepomuk/simpleresourcegraph.h>
#include <nepomuk/query.h>
#include <nepomuk/term.h>
#include <Nepomuk/Vocabulary/NIE>
#include <Nepomuk/Vocabulary/PIMO>
#include <Nepomuk/Vocabulary/NCAL>
#include <Nepomuk/Vocabulary/NFO>
#include <nepomuk/literalterm.h>
#include <nepomuk/comparisonterm.h>

#include <akonadi/item.h>
#include <nepomuk/orterm.h>
#include <nepomuk/resourcetypeterm.h>
#include <nepomuk/andterm.h>
#include <aneo.h>
#include <soprano/rdf.h>
#include <soprano/nao.h>
#include <nepomuk/resourceterm.h>



class KJob;
namespace Akonadi {
    class Item;
}
namespace Soprano {
    class Statement;
}



namespace MindMirrorQueries
{
    using namespace Nepomuk::Query;
        using namespace Nepomuk::Types;
        using namespace Nepomuk::Vocabulary;
    
    Nepomuk::Query::Term resourceTypes()
    {
        
        AndTerm outer;
        OrTerm resourcesTerm;
        resourcesTerm.addSubTerm(ResourceTypeTerm(Class(NCAL::Todo())));
        resourcesTerm.addSubTerm(ResourceTypeTerm(Class(NCAL::Event())));
        resourcesTerm.addSubTerm(ResourceTypeTerm(Class(NFO::HtmlDocument())));
        outer.addSubTerm(resourcesTerm);
        outer.addSubTerm(ResourceTypeTerm(Class(Vocabulary::ANEO::AkonadiDataObject())));
        return outer;
    }
    
    Nepomuk::Query::Term itemUrlTerm(const Akonadi::Item &item)
    {
        return LiteralTerm(item.url().url());
    }
    
    Nepomuk::Query::Term itemResourceTerm(const Akonadi::Item &item)
    {
        AndTerm outer;
        outer.addSubTerm(resourceTypes());
        outer.addSubTerm(ComparisonTerm(Nepomuk::Types::Property(NIE::url()), itemUrlTerm(item), ComparisonTerm::Equal));
        return outer;
    }
    
    Nepomuk::Query::Term itemThingTerm(const Akonadi::Item &item)
    {
        AndTerm outer;
        outer.addSubTerm(ResourceTypeTerm(Class(PIMO::Thing())));
        return ComparisonTerm(Nepomuk::Types::Property(PIMO::groundingOccurrence()), itemResourceTerm(item), ComparisonTerm::Equal);
    }
    
    Nepomuk::Query::Term itemTopicsTerm(const Akonadi::Item &item)
    {
        AndTerm outer;
        outer.addSubTerm(itemThingTerm(item));
        ComparisonTerm topic(PIMO::isRelated(), ResourceTypeTerm(Class(PIMO::Topic())), ComparisonTerm::Equal);
        topic.setVariableName("topic");
        outer.addSubTerm(topic);
        return outer;
    }
    
    Nepomuk::Query::Term itemsWithTopicsTerm(const QList <QUrl> topics)
    {
        OrTerm topicTerm;
        foreach( const QUrl &t, topics) {
            topicTerm.addSubTerm(ResourceTerm(t));
        }
        
        AndTerm outer;
        outer.addSubTerm(ComparisonTerm(PIMO::isRelated(), topicTerm, ComparisonTerm::Equal)); //matches the things
        
        AndTerm resourceTerm;

        ComparisonTerm uriTerm(NIE::url(), Term());
        uriTerm.setVariableName("url");
        resourceTerm.addSubTerm(uriTerm);
        
        ComparisonTerm gt(PIMO::groundingOccurrence(), resourceTerm);
        gt.setVariableName("groundingOccurrence");
        outer.addSubTerm(gt);
        
        return outer;
    }
    
    QString itemThingQuery(const Akonadi::Item &item)
    {
        //can we add to this query preferred results?
        //prefer thing created by zanshin, things with isRelated properties
        QString query = QString::fromLatin1("select distinct ?r where { ?r <%2> ?g. ?g <%3> <%1>.} LIMIT 1")
            .arg(item.url().url())
            .arg(Nepomuk::Vocabulary::PIMO::groundingOccurrence().toString())
            .arg(Nepomuk::Vocabulary::NIE::url().toString());
        return query;
    }
    
    ///get topics of item
    QString itemTopicsQuery(const Akonadi::Item &item)
    {//"select ?topic where { ?topic <%4> <%5>. ?r <%6> ?topic. ?r <%2> ?g. ?g <%3> <%1>.}"
    //select ?t where { ?t rdf:type pimo:Topic. ?r pimo:isRelated ?t. ?r pimo:groundingOccurrence ?g. ?g nie:url <akonadi:?item=225> }
        //TODO for some reason ?thing <%6> ?r can not be changed into ?r <%6> ?thing although the property is bidirectional
    QString query = QString::fromLatin1("select ?r ?reqProp1 where { ?r <%4> <%5>. ?thing <%6> ?r. ?thing <%2> ?g. ?g <%3> <%1>. ?r <%7> ?reqProp1.}")
            .arg(item.url().url())
            .arg(Nepomuk::Vocabulary::PIMO::groundingOccurrence().toString())
            .arg(Nepomuk::Vocabulary::NIE::url().toString())
            .arg(Soprano::Vocabulary::RDF::type().toString())
            .arg(PIMO::Topic().toString())
            .arg(PIMO::isRelated().toString())
            .arg(Soprano::Vocabulary::NAO::prefLabel().toString());
        kDebug() << query;
        return query;
    }
    
    ///get items with one of the topics
    QString itemsWithTopicsQuery(const QList <QUrl> topics)
    {
        if (topics.isEmpty()) {
            kWarning() << "not topics passed";
            return QString();
        }
        //select ?g ?u where {{?r pimo:isRelated <nepomuk:/res/8c9f92b8-c368-4fe5-974e-50d15390305d>} UNION {?r pimo:isRelated <nepomuk:/res/8c9f92b8-c368-4fe5-974e-50d15390305a>}. ?r pimo:groundingOccurrence ?g. ?g nie:url ?u}
        QString topicQueryPart = QString::fromLatin1("{?thing <%1> <%2>}").arg(PIMO::isRelated().toString()).arg(topics.first().toString());
        for (int i = 1; i < topics.size(); i++) { //TODO iterator
            topicQueryPart.append(QString::fromLatin1(" UNION {?thing <%1> <%2>}").arg(PIMO::isRelated().toString()).arg(topics.at(i).toString()));
        }
        
       QString query = QString::fromLatin1("select ?r ?url where {%1. ?thing <%2> ?r. ?r <%3> ?url}")
            .arg(topicQueryPart)
            .arg(PIMO::groundingOccurrence().toString())
            .arg(NIE::url().toString());
//         kDebug() << query;
        return query;
    }
    
    QString itemsWithNoTopicsQuery()
    {
        //select ?g ?u where {{?r pimo:isRelated <nepomuk:/res/8c9f92b8-c368-4fe5-974e-50d15390305d>} UNION {?r pimo:isRelated <nepomuk:/res/8c9f92b8-c368-4fe5-974e-50d15390305a>}. ?r pimo:groundingOccurrence ?g. ?g nie:url ?u}
      /* SELECT ?fp ?q 
 { ?fp rdf:type bad:FP. 
 { SELECT ?fp (count (?var) as ?q) { ?fp bad:mode ?var } 
 GROUP BY ?fp } FILTER (?q > 1)}
 */
       QString query = QString::fromLatin1("select ?r ?url where {?thing <%1> ?topic. ?topic hasType Topic. ?thing <%2> ?r. ?r <%3> ?url.}") //Count number of topics and filter the ones with more than 0 topics
            .arg(PIMO::isRelated().toString())
            .arg(PIMO::groundingOccurrence().toString())
            .arg(NIE::url().toString());
        kDebug() << query;
        return query;
    }

}

