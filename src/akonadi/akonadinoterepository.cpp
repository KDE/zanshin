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


#include "akonadinoterepository.h"

#include <Akonadi/Item>

#include "akonadiitemfetchjobinterface.h"
#include "akonadiserializer.h"
#include "akonadistorage.h"
#include "akonadistoragesettings.h"

using namespace Akonadi;

NoteRepository::NoteRepository(QObject *parent)
    : QObject(parent),
      m_storage(new Storage),
      m_serializer(new Serializer),
      m_ownInterfaces(true)
{
}

NoteRepository::NoteRepository(StorageInterface *storage, SerializerInterface *serializer)
    : m_storage(storage),
      m_serializer(serializer),
      m_ownInterfaces(false)
{
}

NoteRepository::~NoteRepository()
{
    if (m_ownInterfaces) {
        delete m_storage;
        delete m_serializer;
    }
}

bool NoteRepository::isDefaultSource(Domain::DataSource::Ptr source) const
{
    auto settingsCollection = StorageSettings::instance().defaultNoteCollection();
    auto sourceCollection = m_serializer->createCollectionFromDataSource(source);
    return settingsCollection == sourceCollection;
}

void NoteRepository::setDefaultSource(Domain::DataSource::Ptr source)
{
    auto collection = m_serializer->createCollectionFromDataSource(source);
    StorageSettings::instance().setDefaultNoteCollection(collection);
}

KJob *NoteRepository::save(Domain::Note::Ptr note)
{
    auto item = m_serializer->createItemFromNote(note);

    if (note->property("itemId").isValid()) {
        item.setId(note->property("itemId").toLongLong());
        return m_storage->updateItem(item);
    } else {
        return m_storage->createItem(item, m_storage->defaultNoteCollection());
    }
}

KJob *NoteRepository::remove(Domain::Note::Ptr note)
{
    auto item = m_serializer->createItemFromNote(note);
    return m_storage->removeItem(item);
}
