/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Rémi Benoit <r3m1.benoit@gmail.com>

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

#ifndef AKONADI_SERIALIZER_H
#define AKONADI_SERIALIZER_H

#include "akonadiserializerinterface.h"

namespace Akonadi {

class Item;
class Tag;

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

    Domain::Context::Ptr createContextFromTag(Akonadi::Tag tag) override;
    void updateContextFromTag(Domain::Context::Ptr context, Akonadi::Tag tag) override;
    Akonadi::Tag createTagFromContext(Domain::Context::Ptr context) override;
    bool isContextTag(const Domain::Context::Ptr &context, const Akonadi::Tag &tag) const override;
    bool isContextChild(Domain::Context::Ptr context, Akonadi::Item item) const override;

    bool isContext(Akonadi::Item item) override;
    bool itemRepresentsContext(const Domain::Context::Ptr &context, Akonadi::Item item) const override;
    Domain::Context::Ptr createContextFromItem(Akonadi::Item item) override;
    Akonadi::Item createItemFromContext(Domain::Context::Ptr context) override;
    void updateContextFromItem(Domain::Context::Ptr context, Akonadi::Item item) override;
    void addContextToTask(Domain::Context::Ptr context, Akonadi::Item item) override;
    void removeContextFromTask(Domain::Context::Ptr context, Akonadi::Item item) override;
    QString contextUid(Akonadi::Item item) override;

private:
    bool isContext(const Akonadi::Tag &tag) const;
};

}

#endif // AKONADI_SERIALIZERINTERFACE_H
