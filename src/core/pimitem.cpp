/* This file is part of Zanshin Todo.

   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>

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


#include "pimitem.h"

#include <Akonadi/ItemModifyJob>
#include <Akonadi/EntityDisplayAttribute>

PimItem::PimItem()
{

}

PimItem::PimItem(const Akonadi::Item &item)
    : m_item(item)
{
}

PimItem::~PimItem()
{

}

PimItem::ItemType PimItem::itemType(const Akonadi::Item &item)
{
    //this works only if the mimetype of the akonadi item has been saved already
    Q_ASSERT(!item.mimeType().isEmpty());
    if (item.mimeType() == mimeType(Note)) {
        return Note;
    } else if (item.mimeType() == mimeType(Event)) {
        return Event;
    } else if (item.mimeType() == mimeType(Todo)) {
        return Todo;
    }
    kWarning() << "unknown type" << item.mimeType();
    return Unknown;
}

QString PimItem::getTitle()
{
    if (m_item.hasAttribute<Akonadi::EntityDisplayAttribute>()) {
        Akonadi::EntityDisplayAttribute *att = m_item.attribute<Akonadi::EntityDisplayAttribute>();
        return att->displayName();
    }
    return QString();
}

KDateTime PimItem::getLastModifiedDate()
{
    if (!m_item.isValid()) {
        kWarning() << "invalid item";
        return KDateTime();
    }
    return KDateTime(m_item.modificationTime(), KDateTime::LocalZone);
}

QString PimItem::mimeType(PimItem::ItemType type)
{
    switch (type) {
        case Note:
            return QString::fromLatin1( "text/x-vnd.akonadi.note" );
        case Event:
            return QString::fromLatin1( "application/x-vnd.akonadi.calendar.event" );
        case Todo:
            return QString::fromLatin1( "application/x-vnd.akonadi.calendar.todo" );
        case Incidence:
            return QString::fromLatin1( "text/calendar" );
        default:
            kWarning() << "not implemented";
            Q_ASSERT(0);
    }
    return QString();
}

QStringList PimItem::mimeTypes()
{
    QStringList list;
    list << mimeType(Note);
    list << mimeType(Event);
    list << mimeType(Todo);
    return list;
}

const Akonadi::Item& PimItem::getItem() const
{
    return m_item;
}

void PimItem::setItem(const Akonadi::Item &item)
{
    m_item = item;
    Q_ASSERT(hasValidPayload());
}

KJob *PimItem::saveItem()
{
    if (!hasValidPayload()) {
        kWarning() << "tried to save item without payload";
        return 0;
    }
    return new Akonadi::ItemModifyJob(m_item);
}

bool PimItem::textIsRich()
{
    return false;
}

bool PimItem::titleIsRich()
{
    return false;
}

const KCalCore::Attachment::List PimItem::getAttachments()
{
    return KCalCore::Attachment::List();
}

void PimItem::setContexts(const QStringList& )
{

}

QStringList PimItem::getContexts()
{
    return QStringList();
}

