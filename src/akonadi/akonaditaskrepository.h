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

#ifndef AKONADI_TASKREPOSITORY_H
#define AKONADI_TASKREPOSITORY_H

#include "domain/taskrepository.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>

namespace Akonadi {

class SerializerInterface;
class StorageInterface;

class TaskRepository : public QObject, public Domain::TaskRepository
{
    Q_OBJECT
public:
    explicit TaskRepository(QObject *parent = 0);
    TaskRepository(StorageInterface *storage, SerializerInterface *serializer);
    virtual ~TaskRepository();

    virtual bool isDefaultSource(Domain::DataSource::Ptr source) const;
    virtual void setDefaultSource(Domain::DataSource::Ptr source);

    virtual KJob *create(Domain::Task::Ptr task);
    virtual KJob *createInProject(Domain::Task::Ptr task, Domain::Project::Ptr project);
    virtual KJob *createInContext(Domain::Task::Ptr task, Domain::Context::Ptr context);
    virtual KJob *createInTag(Domain::Task::Ptr task, Domain::Tag::Ptr tag);

    virtual KJob *update(Domain::Task::Ptr task);
    virtual KJob *remove(Domain::Task::Ptr task);

    virtual KJob *associate(Domain::Task::Ptr parent, Domain::Task::Ptr child);
    virtual KJob *dissociate(Domain::Task::Ptr child);

    virtual KJob *delegate(Domain::Task::Ptr task, Domain::Task::Delegate delegate);

private:
    StorageInterface *m_storage;
    SerializerInterface *m_serializer;
    bool m_ownInterfaces;

    KJob *createItem(const Akonadi::Item &item);
};

}

#endif // AKONADI_TASKREPOSITORY_H
