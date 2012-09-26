/* This file is part of Zanshin Todo.

   Copyright 2012 Christian Mollekopf <chrigi_1@fastmail.fm>

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

#include "itemmonitor.h"
#include <queries.h>
#include <QTimer>
#include <Nepomuk2/Query/QueryServiceClient>
#include <Nepomuk2/Query/Result>
#include <soprano/nao.h>

ItemMonitor::ItemMonitor(const Akonadi::Item& item, QObject* parent): QObject(parent), mItem (item)
{
    QTimer::singleShot(0, this, SLOT(init()));
}

void ItemMonitor::init()
{
    Nepomuk2::Query::QueryServiceClient *client = new Nepomuk2::Query::QueryServiceClient(this);
    connect(client, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)), SLOT(gotThing(QList<Nepomuk2::Query::Result>)));
    if (!client->sparqlQuery(MindMirrorQueries::itemThingQuery(mItem))) {
        kWarning() << "failed to start query";
    }
}


void ItemMonitor::gotThing(const QList< Nepomuk2::Query::Result > &result)
{
    Nepomuk2::Query::QueryServiceClient *client = qobject_cast<Nepomuk2::Query::QueryServiceClient*>(sender());
    Q_ASSERT(client);
    client->close();
    emit gotThing(result.first().resource());
    
    Nepomuk2::Query::QueryServiceClient *topicsClient = new Nepomuk2::Query::QueryServiceClient(this);
    connect(topicsClient, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)), SLOT(newTopics(QList<Nepomuk2::Query::Result>)));
    connect(topicsClient, SIGNAL(entriesRemoved(QList<QUrl>)), SLOT(topicsRemoved(QList<QUrl>)));
    
    Nepomuk2::Query::RequestPropertyMap encodedRps;
    encodedRps.insert( QString::fromLatin1( "reqProp1" ), Soprano::Vocabulary::NAO::prefLabel() );
    if (!topicsClient->sparqlQuery(MindMirrorQueries::itemTopicsQuery(mItem), encodedRps)) {
        kWarning() << "failed to start query: " << topicsClient->errorMessage();
    }
}

void ItemMonitor::newTopics(const QList< Nepomuk2::Query::Result > &results)
{
    foreach (const Nepomuk2::Query::Result &result, results) {
        const Nepomuk2::Resource &res = result.resource();
        const Soprano::Node &property = result.requestProperty(Soprano::Vocabulary::NAO::prefLabel());
//         kDebug() << "result added: " << property.isValid() << property.isLiteral() << property.literal().isString() << property.literal().type() << result.requestProperties().size();
        mTopics.insert(res.uri(), property.literal().toString());
    }
    emit topicsChanged(mTopics.values());
}

void ItemMonitor::topicsRemoved(const QList <QUrl> &results)
{
    foreach (const QUrl &result, results) {
        mTopics.remove(result);
    }
    emit topicsChanged(mTopics.values());
}