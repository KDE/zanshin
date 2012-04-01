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

#include "nepomukadapter.h"
#include <queries.h>
#include <pimitem.h>
#include "globaldefs.h"
#include "topicsmodel.h"
#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Query/Result>
#include <Nepomuk/Types/Class>
#include <Nepomuk/Resource>
#include <KDebug>
#include <nepomuk/resourcetypeterm.h>
#include <nepomuk/resourcewatcher.h>
#include <Soprano/Vocabulary/NAO>

StructureAdapter::StructureAdapter(QObject* parent): QObject(parent), m_model(0)
{

}


void StructureAdapter::setModel(TopicsModel* model)
{
    m_model = model;
}


TestStructureAdapter::TestStructureAdapter(QObject* parent)
: StructureAdapter(parent)
{

}


QStringList TestStructureAdapter::onSourceInsertRow(const QModelIndex &sourceChildIndex)
{
    if (!sourceChildIndex.isValid()) {
        kWarning() << "invalid indexx";
        return QStringList();
    }

    const QString &parent = sourceChildIndex.data(TopicParentRole).toString();
    if (parent.isEmpty()) {
        return QStringList();
    }

    return QStringList() << parent;
}

QStringList TestStructureAdapter::onSourceDataChanged(const QModelIndex &sourceIndex)
{
    return onSourceInsertRow(sourceIndex);

//         QSet<QString> newCategories = QSet<QString>::fromList(sourceIndex.data(Zanshin::CategoriesRole).toStringList());
//         
//         QSet<QString> oldCategories = QSet<QString>::fromList(m_categories);
//         QSet<QString> interCategories = newCategories;
//         interCategories.intersect(oldCategories);
//         newCategories-= interCategories;
//         
//         foreach (const QString &newCategory, newCategories) {
//             addCategory(newCategory);
//         }
}



// void TestStructureAdapter::addItem(const QString& parentIdentifier, const Akonadi::Item::List &items)
// {
//     emit itemsAdded(parentIdentifier, items);
// }

void TestStructureAdapter::addParent(const QString& identifier, const QString& parentIdentifier, const QString& name)
{
    kDebug() << identifier << parentIdentifier << name;
    m_model->createOrRenameParent(identifier, parentIdentifier, name);
//     emit parentAdded(identifier, parentIdentifier, name);
}

void TestStructureAdapter::removeParent(const QString& identifier)
{
//     emit parentRemoved(identifier);
}


NepomukAdapter::NepomukAdapter(QObject* parent)
: StructureAdapter(parent)
{

}


void NepomukAdapter::setType(const QUrl &type)
{
    m_type = type;
    
    Nepomuk::Query::Query query;
    query.setTerm(Nepomuk::Query::ResourceTypeTerm(Nepomuk::Types::Class(m_type)));

    Nepomuk::Query::QueryServiceClient *queryServiceClient = new Nepomuk::Query::QueryServiceClient(this);
    connect(queryServiceClient, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)), this, SLOT(checkResults(QList<Nepomuk::Query::Result>)));
    connect(queryServiceClient, SIGNAL(finishedListing()), this, SLOT(queryFinished()));
    connect(queryServiceClient, SIGNAL(entriesRemoved(QList<QUrl>)), this, SLOT(removeResult(QList<QUrl>)));
    if ( !queryServiceClient->query(query) ) {
        kWarning() << "error";
    }
}

void NepomukAdapter::checkResults(const QList< Nepomuk::Query::Result > &results)
{
    //kDebug() <<  results.size() << results.first().resource().resourceUri() << results.first().resource().label() << results.first().resource().types() << results.first().resource().className();
    foreach (const Nepomuk::Query::Result &result, results) {
        Nepomuk::Resource res(result.resource().resourceUri());
        kDebug() << res.resourceUri() << res.label() << res.types() << res.className();
        if (res.types().contains(m_type)) {
            addParent(res);
        } else {
            kWarning() << "unknown result " << res.types();
        }
    }
}


void NepomukAdapter::addParent (const Nepomuk::Resource& topic)
{
    kDebug() << "add topic" << topic.label() << topic.resourceUri();
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
    //connect(queryServiceClient, SIGNAL(finishedListing()), queryServiceClient, SLOT(deleteLater()));
    if ( !queryServiceClient->sparqlQuery(MindMirrorQueries::itemsWithTopicsQuery(QList <QUrl>() << topic.resourceUri())) ) {
        kWarning() << "error";
    }
//     emit parentAdded(topic.resourceUri().toString(), QString(), topic.label());
    m_model->createNode(topic.resourceUri().toString(), QString(), topic.label());
}

void NepomukAdapter::removeResult(const QList<QUrl> &results)
{
    foreach (const QUrl &result, results) {
        Nepomuk::Resource res(result);
        kDebug() << res.resourceUri() << res.label() << res.types() << res.className();
        if (res.types().contains(m_type)) {
            emit parentRemoved(res.resourceUri().toString());
            m_guardMap.take(res.resourceUri())->deleteLater();
        } else {
            kWarning() << "unknown result " << res.types();
        }
    }
}

void NepomukAdapter::queryFinished()
{
    kWarning();
    //emit ready();
}


void NepomukAdapter::itemsWithTopicAdded(const QList<Nepomuk::Query::Result> &results)
{
    const QUrl &parent = sender()->property("resourceuri").toUrl();
    kDebug() << parent;
    
    QModelIndexList list;
    foreach (const Nepomuk::Query::Result &result, results) {
        Nepomuk::Resource res = Nepomuk::Resource(result.resource().resourceUri());
        kDebug() << res.resourceUri() << res.label() << res.types() << res.className();
        const Akonadi::Item item = PimItemUtils::getItemFromResource(res);
        if (!item.isValid()) {
            continue;
        }
        const QModelIndexList &indexes = Akonadi::EntityTreeModel::modelIndexesForItem(m_model, item);
        if (indexes.isEmpty()) {
            kDebug() << "item not found" << item.url();
            continue;
        }
        list.append(indexes.first()); //TODO hanle all
    }
    emit itemsAdded(parent.toString(), list);
}

void NepomukAdapter::itemsFromTopicRemoved(const QList<QUrl> &items)
{
    const QUrl &topic = sender()->property("topic").toUrl();
    kDebug() << "removing nodes from topic: " << topic;
//     Akonadi::Item::List list;
    QModelIndexList list;
    foreach (const QUrl &uri, items) {
        Nepomuk::Resource res = Nepomuk::Resource(uri);
        const Akonadi::Item item = PimItemUtils::getItemFromResource(res);
        if (!item.isValid()) {
            continue;
        }
        kDebug() << item.url();
        const QModelIndexList &indexes = Akonadi::EntityTreeModel::modelIndexesForItem(m_model, item);
        if (indexes.isEmpty()) {
            kDebug() << "item not found" << item.url();
            continue;
        }
        list.append(indexes.first()); //TODO handle all
    }
    emit itemsRemovedFromParent(topic.toString(), list);
}

void NepomukAdapter::propertyChanged(const Nepomuk::Resource &res, const Nepomuk::Types::Property &property, const QVariant &value)
{
    if (property.uri() == Soprano::Vocabulary::NAO::prefLabel()) {
        kDebug() << "renamed " << res.resourceUri() << " to " << value.toString();
        emit parentChanged(res.resourceUri().toString(), QString(), value.toString());
    }
}