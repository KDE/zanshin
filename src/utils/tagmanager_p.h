/* This file is part of Zanshin Todo.
 * 
 * Copyright 2012 Christian Mollekopf <chrigi_1@fastmail.fm>
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

#ifndef TAGMANAGER_P_H
#define TAGMANAGER_P_H

#include <KJob>
#include <kdebug.h>
#include <Nepomuk/Vocabulary/PIMO>
#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Query/Result>
#include <QUrl>
#include <QVariant>
#include <nepomuk/datamanagement.h>
#include <nepomuk/resource.h>
#include <akonadi/item.h>
#include "queries.h"


class RemoveAllTopicsJob: public KJob
{
    Q_OBJECT
public:
    explicit RemoveAllTopicsJob(const Akonadi::Item &item, QObject* parent = 0)
    : KJob(parent), mItem(item), mThingFound(false), mTopicsFound(false)
    {
        kDebug() << item.id();
    }
    
    virtual void start() {
        kDebug();
        Nepomuk::Query::QueryServiceClient *topicsQuery = new Nepomuk::Query::QueryServiceClient(this);
        connect(topicsQuery, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)), SLOT(topicsFound(QList<Nepomuk::Query::Result>)));
        connect(topicsQuery, SIGNAL(finishedListing()), SLOT(topicsQueryFinished()));
        if (!topicsQuery->sparqlQuery(MindMirrorQueries::itemTopicsQuery(mItem))) {
            kWarning() << topicsQuery->errorMessage();
        }
        
        Nepomuk::Query::QueryServiceClient *thingquery = new Nepomuk::Query::QueryServiceClient(this);
        connect(thingquery, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)), SLOT(thingFound(QList<Nepomuk::Query::Result>)));
        if (!thingquery->sparqlQuery(MindMirrorQueries::itemThingQuery(mItem))) {
            kWarning() << thingquery->errorMessage();
        }
    }
    
private slots:
    void thingFound(const QList< Nepomuk::Query::Result > &results)
    {
        Q_ASSERT (!mThingFound); //This shouldn't be called twice (there is only 1 thing per item)
        if (results.isEmpty()) {
            kWarning() << "thing not found";
            setError(1);
            setErrorText("thing not found");
            emitResult();
            return;
        }
        Q_ASSERT(results.size() == 1);
        //kDebug() <<  results.size() << results.first().resource().resourceUri() << results.first().resource().label() << results.first().resource().types() << results.first().resource().className();
        Nepomuk::Resource thing(results.first().resource().resourceUri());
        Q_ASSERT(thing.isValid());
        mThing = thing;
        mThingFound = true;
        tryRemove();
    }
    
    void topicsFound(const QList< Nepomuk::Query::Result > &results)
    {
        foreach (const Nepomuk::Query::Result &result, results) {
            mTopics.append(result.resource().resourceUri());
        }
    }
    
    void topicsQueryFinished()
    {
        mTopicsFound = true;
        tryRemove();
    }
    
    void tryRemove()
    {
        if (!mThingFound || !mTopicsFound) {
            return;
        }
        QVariantList values;
        foreach (const QUrl &url, mTopics) {
            //             kDebug() << url;
            values << url;
        }
        KJob *removeJob = Nepomuk::removeProperty(QList<QUrl>() << mThing.resourceUri(), Nepomuk::Vocabulary::PIMO::isRelated(), values);
        connect(removeJob, SIGNAL(result(KJob*)), SLOT(removeJobFinished(KJob*)));
    }
    
    void removeJobFinished(KJob *job)
    {
        if (job->error()) {
            kWarning() << job->errorString();
            setError(1);
            setErrorText(job->errorString());
            emitResult();
            return;
        }
        kDebug() << "removed item from topics";
        emitResult();
    }
    
private:
    Akonadi::Item mItem;
    Nepomuk::Resource mThing;
    QList<QUrl> mTopics;
    bool mThingFound, mTopicsFound;
};



#endif
