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

#include <KCalCore/Attachment>
#include <KDebug>

PimItem::PimItem()
{

}

PimItem::~PimItem()
{

}

QString PimItem::mimeType(ItemType type)
{
    switch (type) {
        case Note:
            return QString::fromLatin1( "text/x-vnd.akonadi.note" );
        case Event:
            return QString::fromLatin1( "application/x-vnd.akonadi.calendar.event" );
        case Todo:
            return QString::fromLatin1( "application/x-vnd.akonadi.calendar.todo" );
        default:
            kWarning() << "not implemented";
            Q_ASSERT(0);
    }
    return QString();
}

bool PimItem::isTextRich() const
{
    return false;
}

bool PimItem::isTitleRich() const
{
    return false;
}

PimItem::DateRoles PimItem::supportedDateRolesForType(ItemType type)
{
    DateRoles roles = NoDateRole;

    switch (type) {
    case NoType:
    case Inbox:
    case FolderRoot:
    case Context:
    case Topic:
    case Collection:
        break;
    case Project:
    case Note:
        roles = CreationDate | LastModifiedDate;
        break;
    case Journal:
        roles = CreationDate | LastModifiedDate | StartDate;
        break;
    case Event:
        roles = CreationDate | LastModifiedDate | StartDate | EndDate;
        break;
    case Todo:
        roles = CreationDate | LastModifiedDate | StartDate | DueDate;
        break;
    }

    return roles;
}

PimItem::DateRoles PimItem::supportedDateRoles() const
{
    return supportedDateRolesForType(itemType());
}

KDateTime PimItem::primaryDate() const
{
    switch (itemType()) {
    case NoType:
    case Inbox:
    case FolderRoot:
    case Context:
    case Topic:
    case Collection:
    case Project:
        return KDateTime();

    case Note:
        return date(LastModifiedDate);

    case Journal:
    case Event:
        return date(StartDate);

    case Todo:
        return date(DueDate);
    }

    return KDateTime();
}

const KCalCore::Attachment::List PimItem::attachments() const
{
    return KCalCore::Attachment::List();
}

void PimItem::setContexts(const QStringList& )
{

}

QStringList PimItem::contexts() const
{
    return QStringList();
}

