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

#ifndef AKONADITAGREPOSITORY_H
#define AKONADITAGREPOSITORY_H

#include "domain/tagrepository.h"

#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

namespace Akonadi {

class SerializerInterface;
class StorageInterface;

class TagRepository : public QObject, public Domain::TagRepository
{
    Q_OBJECT
public:
    typedef QSharedPointer<TagRepository> Ptr;

    TagRepository(const StorageInterface::Ptr &storage,
                  const SerializerInterface::Ptr &serializer);

    KJob *create(Domain::Tag::Ptr tag) Q_DECL_OVERRIDE;
    KJob *remove(Domain::Tag::Ptr tag) Q_DECL_OVERRIDE;

    KJob *associate(Domain::Tag::Ptr parent, Domain::Artifact::Ptr child) Q_DECL_OVERRIDE;
    KJob *dissociate(Domain::Tag::Ptr parent, Domain::Artifact::Ptr child) Q_DECL_OVERRIDE;
    KJob *dissociateAll(Domain::Note::Ptr child) Q_DECL_OVERRIDE;

private:
    StorageInterface::Ptr m_storage;
    SerializerInterface::Ptr m_serializer;
};
}
#endif // AKONADITAGREPOSITORY_H
