/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_SERIALIZERINTERFACE_H
#define AKONADI_SERIALIZERINTERFACE_H

#include "domain/datasource.h"
#include "domain/task.h"
#include "domain/project.h"
#include "domain/context.h"

#include <Akonadi/Item>

namespace Akonadi {

class Collection;
class Item;

class SerializerInterface
{
public:
    typedef QSharedPointer<SerializerInterface> Ptr;
    typedef QSharedPointer<QObject> QObjectPtr;

    enum DataSourceNameScheme {
        FullPath,
        BaseName
    };

    SerializerInterface();
    virtual ~SerializerInterface();

    // Passing Akonadi::Item and Akonadi::Collection by value is a bit wasteful (lots of copy constructor)
    // but is currently necessary because of mockitopp.

    virtual bool representsCollection(QObjectPtr object, Akonadi::Collection collection) = 0;
    virtual bool representsItem(QObjectPtr object, Akonadi::Item item) = 0;

    virtual QString itemUid(const Item &item) = 0;

    virtual Domain::DataSource::Ptr createDataSourceFromCollection(Akonadi::Collection collection, DataSourceNameScheme naming) = 0;
    virtual void updateDataSourceFromCollection(Domain::DataSource::Ptr dataSource, Akonadi::Collection collection, DataSourceNameScheme naming) = 0;
    virtual Akonadi::Collection createCollectionFromDataSource(Domain::DataSource::Ptr dataSource) = 0;
    virtual bool isSelectedCollection(Akonadi::Collection collection) = 0;
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
    virtual void promoteItemToProject(Akonadi::Item item) = 0;
    virtual void clearItem(Akonadi::Item *item) = 0;
    virtual Akonadi::Item::List filterDescendantItems(const Akonadi::Item::List &potentialChildren, const Akonadi::Item &ancestorItem) = 0;

    virtual bool isProjectItem(Akonadi::Item item) = 0;
    virtual Domain::Project::Ptr createProjectFromItem(Akonadi::Item item) = 0;
    virtual void updateProjectFromItem(Domain::Project::Ptr project, Akonadi::Item item) = 0;
    virtual Akonadi::Item createItemFromProject(Domain::Project::Ptr project) = 0;
    virtual bool isProjectChild(Domain::Project::Ptr project, Akonadi::Item item) = 0;

    virtual bool isContextChild(Domain::Context::Ptr context, Akonadi::Item item) const = 0;
    virtual bool isContext(Akonadi::Item item) = 0;
    virtual Domain::Context::Ptr createContextFromItem(Akonadi::Item item) = 0;
    virtual Akonadi::Item createItemFromContext(Domain::Context::Ptr project) = 0;
    virtual void updateContextFromItem(Domain::Context::Ptr context, Akonadi::Item item) = 0;
    virtual void addContextToTask(Domain::Context::Ptr context, Akonadi::Item item) = 0;
    virtual void removeContextFromTask(Domain::Context::Ptr context, Akonadi::Item item) = 0;
    virtual QString contextUid(Akonadi::Item item) = 0;
};

}

#endif // AKONADI_SERIALIZERINTERFACE_H
