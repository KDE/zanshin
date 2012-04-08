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

#ifndef TAGMANAGER_H
#define TAGMANAGER_H

#include <QObject>
#include <QStringList>
#include <QRegExp>

#include <Nepomuk/Tag>
#include <Nepomuk/Thing>
#include <nepomuk/simpleresource.h>
#include <nepomuk/simpleresourcegraph.h>

class KJob;
namespace Akonadi {
    class Item;
}
namespace Soprano {
    class Statement;
}



namespace NepomukUtils
{
    Nepomuk::SimpleResource itemThing(const Akonadi::Item&, Nepomuk::SimpleResourceGraph &);
    //Nepomuk::Thing itemThing(const Akonadi::Item &item);
    
    KJob* tagItem(const Akonadi::Item&, const QString &tag);  //TODO job
    //static KJob* untagItem(const Akonadi::Item&, const QString &tag);

/**
 * Creation: A random resource of the type pimo:topic is created, afterwards we set the label with the name
 * While this does not allow to retrieve the same topic only by label, this is intended beause:
 * 1. topics can be renamed
 * 2. two topics CAN have subtopics with the same label, while being differen topics (although it is possible to add the same subtopic to different subtopics)
 * 
 * Hierarchy: by setting PIMO::superTopic() on the topicthing
 * 
 */
    KJob * createTopic(const QString &topicName, const QUrl &supertopicId = QUrl()); 
    KJob * deleteTopic(const QUrl &topicId); 
    KJob * addToTopic(const Akonadi::Item &item, const QUrl &topicId);  //TODO job
//     static void removeFromTopic(const Akonadi::Item &item, const QString &topicId);
    KJob * moveToTopic(const Akonadi::Item &item, const QUrl &topicId);  //TODO job
    KJob * removeAllTopics(const Akonadi::Item &item);  //TODO job
    KJob * renameTopic(const QUrl &topicId, const QString &name);  //TODO job
    KJob * addToTopic(const QUrl &subtopicId, const QUrl &supertopicId);  //TODO job
    //QStringList getTopicList(const Nepomuk::Thing &thing);
//     QStringList getTopicNameList(const Akonadi::Item& item);
    /**
     * Remove the subtopic from the given supertopic
     */
//     static void removeFromTopic(const QString &subtopicId, const QString &supertopicId);
     KJob * moveToTopic(const QUrl &topicId, const QUrl &supertopicId);  //TODO job
     KJob * removeAllTopics(const QUrl &topicId);  //TODO job
}


#endif // TAGMANAGER_H
