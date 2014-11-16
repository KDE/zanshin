/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Franck Arrecot<franck.arrecot@kgmail.com>

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

#ifndef AKONADICONTEXTREPOSITORY_H
#define AKONADICONTEXTREPOSITORY_H

#include "domain/contextrepository.h"

namespace Akonadi {

class SerializerInterface;
class StorageInterface;

class ContextRepository : public QObject, public Domain::ContextRepository
{
    Q_OBJECT
public:
    typedef QSharedPointer<ContextRepository> Ptr;

    explicit ContextRepository(QObject *parent = 0);
    ContextRepository(StorageInterface *storage, SerializerInterface *serializer);
    virtual ~ContextRepository();

    KJob *create(Domain::Context::Ptr context) Q_DECL_OVERRIDE;
    KJob *update(Domain::Context::Ptr context) Q_DECL_OVERRIDE;
    KJob *remove(Domain::Context::Ptr context) Q_DECL_OVERRIDE;

    KJob *associate(Domain::Context::Ptr parent, Domain::Task::Ptr child) Q_DECL_OVERRIDE;
    KJob *dissociate(Domain::Context::Ptr parent, Domain::Task::Ptr child) Q_DECL_OVERRIDE;
    KJob *dissociateAll(Domain::Task::Ptr child) Q_DECL_OVERRIDE;

private:
    StorageInterface *m_storage;
    SerializerInterface *m_serializer;
    bool m_ownInterfaces;

};

}

#endif // AKONADICONTEXTREPOSITORY_H
