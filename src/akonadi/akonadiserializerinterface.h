/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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

#ifndef AKONADI_SERIALIZERINTERFACE_H
#define AKONADI_SERIALIZERINTERFACE_H

#include "domain/datasource.h"
#include "domain/tag.h"
#include "domain/task.h"
#include "domain/note.h"
#include "domain/project.h"
#include "domain/context.h"

#include <Akonadi/Item>

namespace Akonadi {

class Collection;
class Item;
class Tag;

class SerializerInterface
{
public:
    typedef QSharedPointer<QObject> QObjectPtr;

    enum DataSourceNameScheme {
        FullPath,
        BaseName
    };

    SerializerInterface();
    virtual ~SerializerInterface();

    virtual bool representsCollection(QObjectPtr object, Akonadi::Collection collection) = 0;
    virtual bool representsItem(QObjectPtr object, Akonadi::Item item) = 0;
    virtual bool representsAkonadiTag(Domain::Tag::Ptr tag, Akonadi::Tag akonadiTag) const = 0;

    virtual QString objectUid(QObjectPtr object) = 0;

    virtual Domain::DataSource::Ptr createDataSourceFromCollection(Akonadi::Collection collection, DataSourceNameScheme naming) = 0;
    virtual void updateDataSourceFromCollection(Domain::DataSource::Ptr dataSource, Akonadi::Collection collection, DataSourceNameScheme naming) = 0;
    virtual Akonadi::Collection createCollectionFromDataSource(Domain::DataSource::Ptr dataSource) = 0;
    virtual bool isListedCollection(Akonadi::Collection collection) = 0;
    virtual bool isSelectedCollection(Akonadi::Collection collection) = 0;
    virtual bool isNoteCollection(Akonadi::Collection collection) = 0;
    virtual bool isTaskCollection(Akonadi::Collection collection) = 0;

    virtual bool isTaskItem(Akonadi::Item item) = 0;
    virtual Domain::Task::Ptr createTaskFromItem(Akonadi::Item item) = 0;
    virtual void updateTaskFromItem(Domain::Task::Ptr task, Akonadi::Item item) = 0;
    virtual Akonadi::Item createItemFromTask(Domain::Task::Ptr task) = 0;

    virtual bool isTaskChild(Domain::Task::Ptr task, Akonadi::Item item) = 0;
    virtual QString relatedUidFromItem(Akonadi::Item item) = 0;
    virtual void updateItemParent(Akonadi::Item item, Domain::Task::Ptr parent) = 0;
    virtual void updateItemProject(Akonadi::Item item, Domain::Project::Ptr project) = 0;
    virtual void removeItemParent(Akonadi::Item item) = 0;
    virtual Akonadi::Item::List filterDescendantItems(const Akonadi::Item::List &potentialChildren, const Akonadi::Item &ancestorItem) = 0;

    virtual bool isNoteItem(Akonadi::Item item) = 0;
    virtual Domain::Note::Ptr createNoteFromItem(Akonadi::Item item) = 0;
    virtual void updateNoteFromItem(Domain::Note::Ptr note, Akonadi::Item item) = 0;

    virtual Akonadi::Item createItemFromNote(Domain::Note::Ptr note) = 0;

    virtual bool isProjectItem(Akonadi::Item item) = 0;
    virtual Domain::Project::Ptr createProjectFromItem(Akonadi::Item item) = 0;
    virtual void updateProjectFromItem(Domain::Project::Ptr project, Akonadi::Item item) = 0;
    virtual Akonadi::Item createItemFromProject(Domain::Project::Ptr project) = 0;
    virtual bool isProjectChild(Domain::Project::Ptr project, Akonadi::Item item) = 0;

    virtual Domain::Context::Ptr createContextFromTag(Akonadi::Tag tag) = 0;
    virtual void updateContextFromTag(Domain::Context::Ptr context, Akonadi::Tag tag) = 0;
    virtual Akonadi::Tag createTagFromContext(Domain::Context::Ptr context) = 0;
    virtual bool isContextTag(const Domain::Context::Ptr &context, const Akonadi::Tag &tag) const = 0;
    virtual bool isContextChild(Domain::Context::Ptr context, Akonadi::Item item) const = 0;

    virtual Domain::Tag::Ptr createTagFromAkonadiTag(Akonadi::Tag tag) = 0;
    virtual void updateTagFromAkonadiTag(Domain::Tag::Ptr tag, Akonadi::Tag akonadiTag) = 0;

    virtual bool hasContextTags(Akonadi::Item item) const = 0;
    virtual bool hasAkonadiTags(Akonadi::Item item) const = 0;

    static QByteArray contextTagType();
};

}

#endif // AKONADI_SERIALIZERINTERFACE_H
