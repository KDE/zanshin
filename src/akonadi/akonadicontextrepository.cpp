/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Franck Arrecot <franck.arrecot@gmail.com>

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

#include "akonadicontextrepository.h"

#include "akonadiserializer.h"
#include "akonadistorage.h"

#include "utils/compositejob.h"

using namespace Akonadi;

ContextRepository::ContextRepository(QObject *parent) :
    QObject(parent),
    m_storage(new Storage),
    m_serializer(new Serializer),
    m_ownInterfaces(true)
{
}

ContextRepository::ContextRepository(StorageInterface *storage, SerializerInterface *serializer):
    m_storage(storage),
    m_serializer(serializer),
    m_ownInterfaces(false)
{

}

ContextRepository::~ContextRepository()
{
    if (m_ownInterfaces) {
        delete m_storage;
        delete m_serializer;
    }
}

KJob *ContextRepository::save(Domain::Context::Ptr context)
{
    auto tag = m_serializer->createTagFromContext(context);
    Q_ASSERT(!tag.isValid());
    return m_storage->createTag(tag);
}

KJob *ContextRepository::remove(Domain::Context::Ptr context)
{
    Q_UNUSED(context);
    qFatal("not implemented yet");
    return 0;
}

KJob *ContextRepository::associate(Domain::Context::Ptr parent, Domain::Task::Ptr child)
{
    Q_UNUSED(parent);
    Q_UNUSED(child);
    qFatal("not implemented yet");
    return 0;
}


KJob *ContextRepository::dissociate(Domain::Context::Ptr parent, Domain::Task::Ptr child)
{
    Q_UNUSED(parent);
    Q_UNUSED(child);
    qFatal("not implemented yet");
    return 0;
}
