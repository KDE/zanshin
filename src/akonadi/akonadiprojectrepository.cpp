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


#include "akonadiprojectrepository.h"

#include "akonadiserializer.h"
#include "akonadistorage.h"

using namespace Akonadi;

ProjectRepository::ProjectRepository(QObject *parent)
    : QObject(parent),
      m_storage(new Storage),
      m_serializer(new Serializer),
      m_ownInterfaces(true)
{
}

ProjectRepository::ProjectRepository(StorageInterface *storage, SerializerInterface *serializer)
    : m_storage(storage),
      m_serializer(serializer),
      m_ownInterfaces(false)
{
}

ProjectRepository::~ProjectRepository()
{
    if (m_ownInterfaces) {
        delete m_storage;
        delete m_serializer;
    }
}

KJob *ProjectRepository::create(Domain::Project::Ptr project, Domain::DataSource::Ptr source)
{
    Q_UNUSED(project);
    Q_UNUSED(source);
    qFatal("Not implemented yet");
    return 0;
}

KJob *ProjectRepository::update(Domain::Project::Ptr project)
{
    auto item = m_serializer->createItemFromProject(project);
    Q_ASSERT(item.isValid());
    return m_storage->updateItem(item);
}

KJob *ProjectRepository::remove(Domain::Project::Ptr project)
{
    Q_UNUSED(project);
    qFatal("Not implemented yet");
    return 0;
}

KJob *ProjectRepository::associate(Domain::Project::Ptr parent, Domain::Artifact::Ptr child)
{
    Q_UNUSED(parent);
    Q_UNUSED(child);
    qFatal("Not implemented yet");
    return 0;
}

KJob *ProjectRepository::dissociate(Domain::Project::Ptr parent, Domain::Artifact::Ptr child)
{
    Q_UNUSED(parent);
    Q_UNUSED(child);
    qFatal("Not implemented yet");
    return 0;
}
