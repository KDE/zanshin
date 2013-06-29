/* This file is part of Zanshin Todo.

   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>
   Copyright 2013 Kevin Ottens <ervin@kde.org>

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

#include "akonadibaseitem.h"

#include <Akonadi/ItemModifyJob>
#include <Akonadi/EntityDisplayAttribute>

AkonadiBaseItem::AkonadiBaseItem()
{
}

AkonadiBaseItem::AkonadiBaseItem(const Akonadi::Item &item)
    : m_item(item)
{
}

PimItemIndex::ItemType AkonadiBaseItem::typeFromItem(const Akonadi::Item &item)
{
    //this works only if the mimetype of the akonadi item has been saved already
    Q_ASSERT(!item.mimeType().isEmpty());
    if (item.mimeType() == mimeType(PimItemIndex::Note)) {
        return PimItemIndex::Note;
    } else if (item.mimeType() == mimeType(PimItemIndex::Event)) {
        return PimItemIndex::Event;
    } else if (item.mimeType() == mimeType(PimItemIndex::Todo)) {
        return PimItemIndex::Todo;
    }
    kWarning() << "unknown type" << item.mimeType();
    return PimItemIndex::NoType;
}

QString AkonadiBaseItem::title() const
{
    if (m_item.hasAttribute<Akonadi::EntityDisplayAttribute>()) {
        Akonadi::EntityDisplayAttribute *att = m_item.attribute<Akonadi::EntityDisplayAttribute>();
        return att->displayName();
    }
    return QString();
}

KDateTime AkonadiBaseItem::lastModifiedDate() const
{
    if (!m_item.isValid()) {
        kWarning() << "invalid item";
        return KDateTime();
    }
    return KDateTime(m_item.modificationTime(), KDateTime::LocalZone);
}

const Akonadi::Item &AkonadiBaseItem::getItem() const
{
    return m_item;
}

void AkonadiBaseItem::setItem(const Akonadi::Item &item)
{
    Q_ASSERT(item.hasPayload() && item.mimeType() == mimeType());
    m_item = item;
}

KJob *AkonadiBaseItem::saveItem()
{
    if (!m_item.hasPayload()) {
        kWarning() << "tried to save item without payload";
        return 0;
    }
    return new Akonadi::ItemModifyJob(m_item);
}

