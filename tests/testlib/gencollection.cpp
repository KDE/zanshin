/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include "gencollection.h"

#include <Akonadi/EntityDisplayAttribute>

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

GenCollection &GenCollection::selected(bool value)
{
    if (!value) {
        auto attr = m_collection.attribute<Akonadi::ApplicationSelectedAttribute>(Akonadi::Collection::AddIfMissing);
        attr->setSelected(false);
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
    const auto noteMime = QString("text/x-vnd.akonadi.note");
    auto mimeTypes = m_collection.contentMimeTypes();
    if (!value) {
        mimeTypes.removeAll(noteMime);
    } else if (!mimeTypes.contains(noteMime)) {
        mimeTypes.append(noteMime);
    }
    m_collection.setContentMimeTypes(mimeTypes);
    return *this;
}
