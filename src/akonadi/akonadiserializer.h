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

    bool representsCollection(QObjectPtr object, Collection collection) Q_DECL_OVERRIDE;
    bool representsItem(QObjectPtr object, Item item) Q_DECL_OVERRIDE;

    QString objectUid(QObjectPtr object) Q_DECL_OVERRIDE;

    Domain::DataSource::Ptr createDataSourceFromCollection(Akonadi::Collection collection, DataSourceNameScheme naming) Q_DECL_OVERRIDE;
    void updateDataSourceFromCollection(Domain::DataSource::Ptr dataSource, Akonadi::Collection collection, DataSourceNameScheme naming) Q_DECL_OVERRIDE;
    virtual Akonadi::Collection createCollectionFromDataSource(Domain::DataSource::Ptr dataSource) Q_DECL_OVERRIDE;
    virtual bool isSelectedCollection(Akonadi::Collection collection) Q_DECL_OVERRIDE;
    virtual bool isNoteCollection(Akonadi::Collection collection) Q_DECL_OVERRIDE;
    virtual bool isTaskCollection(Akonadi::Collection collection) Q_DECL_OVERRIDE;

    bool isTaskItem(Akonadi::Item item) Q_DECL_OVERRIDE;
    Domain::Task::Ptr createTaskFromItem(Akonadi::Item item) Q_DECL_OVERRIDE;
    void updateTaskFromItem(Domain::Task::Ptr task, Akonadi::Item item) Q_DECL_OVERRIDE;
    Akonadi::Item createItemFromTask(Domain::Task::Ptr task) Q_DECL_OVERRIDE;
    bool isTaskChild(Domain::Task::Ptr task, Akonadi::Item item) Q_DECL_OVERRIDE;
    QString relatedUidFromItem(Akonadi::Item item) Q_DECL_OVERRIDE;
    void updateItemParent(Akonadi::Item item, Domain::Task::Ptr parent) Q_DECL_OVERRIDE;
    void updateItemProject(Akonadi::Item item, Domain::Project::Ptr project) Q_DECL_OVERRIDE;
    void removeItemParent(Akonadi::Item item) Q_DECL_OVERRIDE;
    Akonadi::Item::List filterDescendantItems(const Akonadi::Item::List &potentialChildren, const Akonadi::Item &ancestorItem) Q_DECL_OVERRIDE;

    bool isNoteItem(Akonadi::Item item) Q_DECL_OVERRIDE;
    Domain::Note::Ptr createNoteFromItem(Akonadi::Item item) Q_DECL_OVERRIDE;
    void updateNoteFromItem(Domain::Note::Ptr note, Akonadi::Item item) Q_DECL_OVERRIDE;
    Akonadi::Item createItemFromNote(Domain::Note::Ptr note) Q_DECL_OVERRIDE;

    bool isProjectItem(Akonadi::Item item) Q_DECL_OVERRIDE;
    Domain::Project::Ptr createProjectFromItem(Akonadi::Item item) Q_DECL_OVERRIDE;
    void updateProjectFromItem(Domain::Project::Ptr project, Akonadi::Item item) Q_DECL_OVERRIDE;
    Akonadi::Item createItemFromProject(Domain::Project::Ptr project) Q_DECL_OVERRIDE;
    bool isProjectChild(Domain::Project::Ptr project, Akonadi::Item item) Q_DECL_OVERRIDE;

    Domain::Context::Ptr createContextFromTag(Akonadi::Tag tag) Q_DECL_OVERRIDE;
    void updateContextFromTag(Domain::Context::Ptr context, Akonadi::Tag tag) Q_DECL_OVERRIDE;
    Akonadi::Tag createTagFromContext(Domain::Context::Ptr context) Q_DECL_OVERRIDE;
    bool isContextTag(const Domain::Context::Ptr &context, const Akonadi::Tag &tag) const Q_DECL_OVERRIDE;
    bool isContextChild(Domain::Context::Ptr context, Akonadi::Item item) const Q_DECL_OVERRIDE;

    bool hasContextTags(Akonadi::Item item) const Q_DECL_OVERRIDE;
    bool hasAkonadiTags(Akonadi::Item item) const Q_DECL_OVERRIDE;

private:
    bool isContext(const Akonadi::Tag &tag) const;
    bool isAkonadiTag(const Akonadi::Tag &tag) const;
};

}

#endif // AKONADI_SERIALIZERINTERFACE_H
