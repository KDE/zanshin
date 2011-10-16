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



namespace MindMirrorNepomukUtils
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
    KJob * addToTopic(const Akonadi::Item &item, const QUrl &topicId);  //TODO job
//     static void removeFromTopic(const Akonadi::Item &item, const QString &topicId);
    KJob * moveToTopic(const Akonadi::Item &item, const QUrl &topicId);  //TODO job
    KJob * removeAllTopics(const Akonadi::Item &item);  //TODO job
    KJob * renameTopic(const QUrl &topicId, const QString &name);  //TODO job
    KJob * addToTopic(const QUrl &subtopicId, const QUrl &supertopicId);  //TODO job
    //QStringList getTopicList(const Nepomuk::Thing &thing);
    QStringList getTopicNameList(const Akonadi::Item& item);
    /**
     * Remove the subtopic from the given supertopic
     */
//     static void removeFromTopic(const QString &subtopicId, const QString &supertopicId);
     KJob * moveToTopic(const QUrl &topicId, const QUrl &supertopicId);  //TODO job
     KJob * removeAllTopics(const QUrl &topicId);  //TODO job
}

/**
 * Manage Tags and Topics of items
 * interface to set, get tags
 * 
 * Currently Tags are setup the following way:
 * 
 * Creation: A random resource of the type pimo:topic is created, afterwards we set the label with the name
 * While this does not allow to retrieve the same topic only by label, this is intended beause:
 * 1. topics can be renamed
 * 2. two topics CAN have subtopics with the same label, while being differen topics (although it is possible to add the same subtopic to different subtopics)
 * 
 * Hierarchy: by setting PIMO::superTopic() on the topicthing
 * 
 * TODO: use hasSuperTopic and hasRootTopic
 * TODO: Maybe add all topics to a single supertopic (i.e. MindMirror) to have them grouped, or maybe use a special MindMirror graph
 * 
 */
#if 0
class TagManager : public QObject
{
    Q_OBJECT
private:
    explicit TagManager();
    TagManager(TagManager const&); // Don't Implement
    void operator=(TagManager const&); // Don't implement
public:
    static TagManager &instance()
    {
        static TagManager inst;
        return inst;
    }

    virtual ~TagManager();

    //returns true if nepomuk is operational
    bool isOperational();

    //Tags
    static KJob* tagItem(const Akonadi::Item&, const QString &tag);
    static KJob* tagItem(const Akonadi::Item &item, const Nepomuk::SimpleResource &tag);
    static void untagItem(const Akonadi::Item&, const QString &tag);
    static QList<Nepomuk::Tag> getItemTags(const Akonadi::Item &);
    static bool itemIsTaggedWith(const Akonadi::Item&, const QRegExp &tag);


    ///Helper functions to copy properties when resource was converted
    static QList<Nepomuk::Resource> getProperties(Nepomuk::Resource);
    static void setProperties(Nepomuk::Resource &, const QList<Nepomuk::Resource> &);

    //Akonadi item topics
    /**
     * Adds the item to a topic.
     * if the topic doesn't exist it is created
     * The topic has the format /supertopic1/supertopic2/topic
     */
    static void addToTopic(const Akonadi::Item &item, const QString &topicId);
    /**
     * Remove the tiem from the given topic
     */
    static void removeFromTopic(const Akonadi::Item &item, const QString &topicId);
    /**
     * Convenicence Function
     * Remove all old topics and add to new topic
     */
    static void moveToTopic(const Akonadi::Item &item, const QString &topicId);
    /**
     * Convenience Function
     * Remove all currently set topics
     */
    static void removeAllTopics(const Akonadi::Item &item);

    /**
     * Returns also true if item has a subtopic of topic
     */
    static bool itemHasTopic(const Nepomuk::Thing &item, const Nepomuk::Thing &topic);

    //Topics
    /**
     * Create a new topic with the given topicName and @return the id
     *
     */
    static QString createTopic(const QString &topicName);
    /**
     * Return the label (to display) of a topic
     */
    QString topicName(const QString &topicId) const;
    /**
     * Set a new label (to display) for the given topic
     */
    static void renameTopic(const QString &topicId, const QString &name);
    /**
     * Adds the subtopic to a supertopic
     * topics are of format /supertopic/subtopic
     * if subtopic already exists, thing of that subtopic is taken
     * and a new resource with identifier $subtopic id created and linked to the same thing.
     * the resource gets then the supertopic supertopichome
     */
    static void addToTopic(const QString &subtopicId, const QString &supertopicId);
    /**
     * Remove the subtopic from the given supertopic
     */
    static void removeFromTopic(const QString &subtopicId, const QString &supertopicId);
    static void moveToTopic(const QString &topicId, const QString &supertopicId);
    static void removeAllTopics(const QString &topicId);

    /**
     * subtopic is subtopic of maintopic
     * Returns true for direct subtopics
     */
    static bool isSubtopic(const QString &maintopic, const QString &subtopic);

    static QStringList getTopicList(const Nepomuk::Thing &thing);
    static QList <KUrl> getTopics(const Nepomuk::Thing &thing);
    static QList <KUrl> getAvailableTopics();

    static Nepomuk::Thing topicThing(const QString &topicId);
    static Nepomuk::Thing itemThing(const Akonadi::Item &);


signals:
    ///depreceated
    void itemTagChanged(const Akonadi::Item&);
    void itemTopicChanged(const Akonadi::Item&);


};

#endif

#endif // TAGMANAGER_H
