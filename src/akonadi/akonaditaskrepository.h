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

namespace Akonadi {

class SerializerInterface;
class StorageInterface;

class TaskRepository : public Domain::TaskRepository
{
public:
    TaskRepository();
    TaskRepository(StorageInterface *storage, SerializerInterface *serializer);
    virtual ~TaskRepository();

    Akonadi::Collection defaultCollection() const;
    void setDefaultCollection(const Akonadi::Collection &defaultCollection);

    virtual KJob *save(Domain::Task::Ptr task);
    virtual KJob *remove(Domain::Task::Ptr task);

    virtual KJob *associate(Domain::Task::Ptr parent, Domain::Artifact::Ptr child);
    virtual KJob *dissociate(Domain::Task::Ptr parent, Domain::Artifact::Ptr child);

private:
    StorageInterface *m_storage;
    SerializerInterface *m_serializer;
    bool m_ownInterfaces;
};

}

#endif // AKONADI_TASKREPOSITORY_H
