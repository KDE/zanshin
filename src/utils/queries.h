/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) <year>  <name of author>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef QUERIES_H
#define QUERIES_H

#include <QList>
#include <QUrl>

#include <nepomuk/query.h>
#include <nepomuk/term.h>

namespace Akonadi {
    class Item;
}

namespace MindMirrorQueries
{

    Nepomuk::Query::Term resourceTypes();
    
    Nepomuk::Query::Term itemUrlTerm(const Akonadi::Item &item);
    
    Nepomuk::Query::Term itemResourceTerm(const Akonadi::Item &item);
    
    Nepomuk::Query::Term itemThingTerm(const Akonadi::Item &item);
    
    Nepomuk::Query::Term itemTopicsTerm(const Akonadi::Item &item);
    
    Nepomuk::Query::Term itemsWithTopicsTerm(const QList <QUrl> topics);
    
    ///get thing of item
    QString itemThingQuery(const Akonadi::Item &item);
    
    ///get topics of item
    QString itemTopicsQuery(const Akonadi::Item &item);
    
    ///get items with one of the topics
    QString itemsWithTopicsQuery(const QList <QUrl> topics);
    
    QString itemsWithNoTopicsQuery();
   

};

#endif
