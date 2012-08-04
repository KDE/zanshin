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

#ifndef QUERIES_H
#define QUERIES_H

#include <QList>
#include <QUrl>

#include <nepomuk2/query.h>
#include <nepomuk2/term.h>

namespace Akonadi {
    class Item;
}

namespace MindMirrorQueries
{

    Nepomuk2::Query::Term resourceTypes();
    
    Nepomuk2::Query::Term itemUrlTerm(const Akonadi::Item &item);
    
    Nepomuk2::Query::Term itemResourceTerm(const Akonadi::Item &item);
    
    Nepomuk2::Query::Term itemThingTerm(const Akonadi::Item &item);
    
    Nepomuk2::Query::Term itemTopicsTerm(const Akonadi::Item &item);
    
    Nepomuk2::Query::Term itemsWithTopicsTerm(const QList <QUrl> topics);
    
    ///get thing of item
    QString itemThingQuery(const Akonadi::Item &item);
    
    ///get topics of item
    QString itemTopicsQuery(const Akonadi::Item &item);
    
    ///get items with one of the topics
    QString itemsWithTopicsQuery(const QList <QUrl> topics);
    
    QString itemsWithNoTopicsQuery();
   

};

#endif
