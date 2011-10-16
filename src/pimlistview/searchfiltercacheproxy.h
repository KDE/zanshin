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


#ifndef SEARCHFILTERCACHECPROXY_H
#define SEARCHFILTERCACHECPROXY_H

#include "kidentityproxymodel_copy.h"
#include "notetakermodel.h"
#include <Nepomuk/Query/Result>
#include <KUrl>

class QUrl;
namespace Nepomuk {
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
class SearchFilterCache: public KIdentityProxyModelCopy
{
    Q_OBJECT
public:
    explicit SearchFilterCache(QObject* parent = 0);

   /* enum CustomRoles {
        TopicRole = NotetakerModel::UserRole,
        TagRole, //Returns a taglist for the item
        TopicRole, //returns a topic list for the item
        FulltextQueryMatch,
        NepomukProxyUserRole
    };*/

    void setFulltextSearch(const QString &);
    bool isFulltextMatch(const Akonadi::Item &) const;
    void setTopicFilter(const QList<KUrl> &topicList, bool noTopic);
    bool isTopicMatch(const Akonadi::Item &item) const;
    
    //virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const;

private slots:
    void itemChanged(const Nepomuk::Resource &resource, const Nepomuk::Types::Property &property, const QVariant &value);
    void newFulltextMatches(QList< Nepomuk::Query::Result > results);
    void entriesRemoved(QList< QUrl > removedResources);
    void newTopicMatches(QList< Nepomuk::Query::Result > results);
    void topicMatchRemoved(QList< QUrl > removedResources);
    void queryFinished();
    
private:    
    Akonadi::Item::List m_fulltextHits;
    Akonadi::Item::List m_topicHits;
    Nepomuk::Query::QueryServiceClient *m_queryServiceClient;
    Nepomuk::Query::QueryServiceClient *m_topicQueryServiceClient;
};

#endif // SEARCHFILTERCACHECPROXY_H
