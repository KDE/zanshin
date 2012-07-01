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


#ifndef SEARCHFILTERCACHECPROXY_H
#define SEARCHFILTERCACHECPROXY_H

#include <kidentityproxymodel.h>
#include <Nepomuk2/Query/Result>
#include <KUrl>
#include <akonadi/item.h>

class QUrl;
namespace Nepomuk2 {
namespace Query {
class QueryServiceClient;
}
}

/**
 * A cache which can be inserted before the filter and notify the notesortfilterproxy of value changes, without invalidating the whole filter
 * 
 * Make data from nepomuk accessible in the model
 * 
 */
class SearchFilterCache: public KIdentityProxyModel
{
    Q_OBJECT
public:
    explicit SearchFilterCache(QObject* parent = 0);
    
    void setFulltextSearch(const QString &);
    bool isFulltextMatch(const Akonadi::Item &) const;
//     void setTopicFilter(const QList<KUrl> &topicList, bool noTopic);
//     bool isTopicMatch(const Akonadi::Item &item) const;
    
private slots:
    void itemChanged(const Nepomuk2::Resource &resource, const Nepomuk2::Types::Property &property, const QVariant &value);
    void newFulltextMatches(QList< Nepomuk2::Query::Result > results);
    void entriesRemoved(QList< QUrl > removedResources);
//     void newTopicMatches(QList< Nepomuk2::Query::Result > results);
//     void topicMatchRemoved(QList< QUrl > removedResources);
    void queryFinished();
    
private:    
    Akonadi::Item::List m_fulltextHits;
//     Akonadi::Item::List m_topicHits;
    Nepomuk2::Query::QueryServiceClient *m_queryServiceClient;
//     Nepomuk2::Query::QueryServiceClient *m_topicQueryServiceClient;
};

#endif // SEARCHFILTERCACHECPROXY_H
