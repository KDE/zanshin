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


#include "akonaditaskrepository.h"

#include <Akonadi/Item>

#include "akonadiserializer.h"
#include "akonadistorage.h"

using namespace Akonadi;

TaskRepository::TaskRepository()
    : m_storage(new Storage),
      m_serializer(new Serializer),
      m_ownInterfaces(true)
{
}

TaskRepository::TaskRepository(StorageInterface *storage, SerializerInterface *serializer)
    : m_storage(storage),
      m_serializer(serializer),
      m_ownInterfaces(false)
{
}

TaskRepository::~TaskRepository()
{
    if (m_ownInterfaces) {
        delete m_storage;
        delete m_serializer;
    }
}

KJob *TaskRepository::save(Domain::Task::Ptr task)
{
    auto item = m_serializer->createItemFromTask(task);

    if (task->property("itemId").isValid()) {
        item.setId(task->property("itemId").toLongLong());
        return m_storage->updateItem(item);
    } else {
        return m_storage->createItem(item, m_storage->defaultTaskCollection());
    }
}

KJob *TaskRepository::remove(Domain::Task::Ptr task)
{
    Q_UNUSED(task);
    qFatal("Not implemented yet");
    return 0;
}

KJob *TaskRepository::associate(Domain::Task::Ptr parent, Domain::Task::Ptr child)
{
    Q_UNUSED(parent);
    Q_UNUSED(child);
    qFatal("Not implemented yet");
    return 0;
}

KJob *TaskRepository::dissociate(Domain::Task::Ptr parent, Domain::Task::Ptr child)
{
    Q_UNUSED(parent);
    Q_UNUSED(child);
    qFatal("Not implemented yet");
    return 0;
}
