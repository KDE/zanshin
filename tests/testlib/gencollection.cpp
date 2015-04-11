/* This file is part of Zanshin

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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

#include "gencollection.h"

#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/Notes/NoteUtils>

#include <KCalCore/Todo>

#include "akonadi/akonadiapplicationselectedattribute.h"

using namespace Testlib;

GenCollection::GenCollection(const Akonadi::Collection &collection)
    : m_collection(collection)
{
}

GenCollection::operator Akonadi::Collection()
{
    return m_collection;
}

GenCollection &GenCollection::withId(Akonadi::Collection::Id id)
{
    m_collection.setId(id);
    return *this;
}

GenCollection &GenCollection::withParent(Akonadi::Collection::Id id)
{
    m_collection.setParentCollection(Akonadi::Collection(id));
    return *this;
}

GenCollection &GenCollection::withRootAsParent()
{
    m_collection.setParentCollection(Akonadi::Collection::root());
    return *this;
}

GenCollection &GenCollection::withName(const QString &name)
{
    m_collection.setName(name);
    return *this;
}

GenCollection &GenCollection::withIcon(const QString &iconName)
{
    auto attr = m_collection.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Collection::AddIfMissing);
    attr->setIconName(iconName);
    return *this;
}

GenCollection &GenCollection::referenced(bool value)
{
    m_collection.setReferenced(value);
    return *this;
}

GenCollection &GenCollection::enabled(bool value)
{
    m_collection.setEnabled(value);
    return *this;
}

GenCollection &GenCollection::selected(bool value)
{
    if (value) {
        auto attr = m_collection.attribute<Akonadi::ApplicationSelectedAttribute>(Akonadi::Collection::AddIfMissing);
        attr->setSelected(true);
    } else {
        m_collection.removeAttribute<Akonadi::ApplicationSelectedAttribute>();
    }
    return *this;
}

GenCollection &GenCollection::withTaskContent(bool value)
{
    auto mimeTypes = m_collection.contentMimeTypes();
    if (!value) {
        mimeTypes.removeAll(KCalCore::Todo::todoMimeType());
    } else if (!mimeTypes.contains(KCalCore::Todo::todoMimeType())) {
        mimeTypes.append(KCalCore::Todo::todoMimeType());
    }
    m_collection.setContentMimeTypes(mimeTypes);
    return *this;
}

GenCollection &GenCollection::withNoteContent(bool value)
{
    auto mimeTypes = m_collection.contentMimeTypes();
    if (!value) {
        mimeTypes.removeAll(Akonadi::NoteUtils::noteMimeType());
    } else if (!mimeTypes.contains(Akonadi::NoteUtils::noteMimeType())) {
        mimeTypes.append(Akonadi::NoteUtils::noteMimeType());
    }
    m_collection.setContentMimeTypes(mimeTypes);
    return *this;
}
