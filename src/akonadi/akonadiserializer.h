/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2014 Rémi Benoit <r3m1.benoit@gmail.com>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
   */


#ifndef AKONADI_SERIALIZER_H
#define AKONADI_SERIALIZER_H

#include "akonadiserializerinterface.h"

namespace Akonadi {

class Item;

class Serializer : public SerializerInterface
{
public:
    Serializer();
    virtual ~Serializer();

    bool representsCollection(QObjectPtr object, Collection collection) override;
    bool representsItem(QObjectPtr object, Item item) override;

    QString itemUid(const Item &item) override;

    Domain::DataSource::Ptr createDataSourceFromCollection(Akonadi::Collection collection, DataSourceNameScheme naming) override;
    void updateDataSourceFromCollection(Domain::DataSource::Ptr dataSource, Akonadi::Collection collection, DataSourceNameScheme naming) override;
    virtual Akonadi::Collection createCollectionFromDataSource(Domain::DataSource::Ptr dataSource) override;
    virtual bool isSelectedCollection(Akonadi::Collection collection) override;
    virtual bool isTaskCollection(Akonadi::Collection collection) override;

    bool isTaskItem(Akonadi::Item item) override;
    Domain::Task::Ptr createTaskFromItem(Akonadi::Item item) override;
    void updateTaskFromItem(Domain::Task::Ptr task, Akonadi::Item item) override;
    Akonadi::Item createItemFromTask(Domain::Task::Ptr task) override;
    bool isTaskChild(Domain::Task::Ptr task, Akonadi::Item item) override;
    QString relatedUidFromItem(Akonadi::Item item) override;
    void updateItemParent(Akonadi::Item item, Domain::Task::Ptr parent) override;
    void updateItemProject(Akonadi::Item item, Domain::Project::Ptr project) override;
    void removeItemParent(Akonadi::Item item) override;
    void promoteItemToProject(Akonadi::Item item) override;
    void clearItem(Akonadi::Item *item) override;
    Akonadi::Item::List filterDescendantItems(const Akonadi::Item::List &potentialChildren, const Akonadi::Item &ancestorItem) override;

    bool isProjectItem(Akonadi::Item item) override;
    Domain::Project::Ptr createProjectFromItem(Akonadi::Item item) override;
    void updateProjectFromItem(Domain::Project::Ptr project, Akonadi::Item item) override;
    Akonadi::Item createItemFromProject(Domain::Project::Ptr project) override;
    bool isProjectChild(Domain::Project::Ptr project, Akonadi::Item item) override;

    bool isContextChild(Domain::Context::Ptr context, Akonadi::Item item) const override;
    bool isContext(Akonadi::Item item) override;
    Domain::Context::Ptr createContextFromItem(Akonadi::Item item) override;
    Akonadi::Item createItemFromContext(Domain::Context::Ptr context) override;
    void updateContextFromItem(Domain::Context::Ptr context, Akonadi::Item item) override;
    void addContextToTask(Domain::Context::Ptr context, Akonadi::Item item) override;
    void removeContextFromTask(Domain::Context::Ptr context, Akonadi::Item item) override;
    QString contextUid(Akonadi::Item item) override;

    static QByteArray customPropertyAppName();
    static QByteArray customPropertyIsProject();
    static QByteArray customPropertyIsContext();
    static QByteArray customPropertyIsRunning();
    static QByteArray customPropertyContextList();

};

}

#endif // AKONADI_SERIALIZERINTERFACE_H
