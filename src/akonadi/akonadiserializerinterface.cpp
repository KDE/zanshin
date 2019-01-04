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


#include "akonadiserializerinterface.h"

using namespace Akonadi;

SerializerInterface::SerializerInterface()
{
}

SerializerInterface::~SerializerInterface()
{
}

Domain::Artifact::Ptr SerializerInterface::createArtifactFromItem(const Item &item)
{
    if (isTaskItem(item)) {
        auto task = createTaskFromItem(item);
        return Domain::Artifact::Ptr(task);
    } else {
        return Domain::Artifact::Ptr();
    }
}

void SerializerInterface::updateArtifactFromItem(const Domain::Artifact::Ptr &artifact, const Item &item)
{
    if (auto task = artifact.dynamicCast<Domain::Task>()) {
        updateTaskFromItem(task, item);
    }
}

QByteArray SerializerInterface::contextTagType()
{
    return QByteArray("Zanshin-Context");
}
