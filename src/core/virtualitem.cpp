/* This file is part of Zanshin Todo.

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

#include "virtualitem.h"

VirtualItem::VirtualItem()
    : m_type(NoType),
      m_relationId(0)
{
}

VirtualItem::VirtualItem(ItemType type, const QString &title)
    : m_type(type),
      m_title(title),
      m_relationId(0)
{
}

qint64 VirtualItem::relationId() const
{
    return m_relationId;
}

void VirtualItem::setRelationId(qint64 id)
{
    m_relationId = id;
}

PimItem::ItemType VirtualItem::itemType() const
{
    return m_type;
}

QString VirtualItem::mimeType() const
{
    return QString();
}

PimItem::ItemStatus VirtualItem::status() const
{
    return Complete;
}

QString VirtualItem::uid() const
{
    return m_title;
}

QString VirtualItem::iconName() const
{
    switch (m_type)
    {
    case Inbox:
        return "mail-folder-inbox";
    case FolderRoot:
        return "document-multiple";
    default:
        return "folder";
    }
}

void VirtualItem::setText(const QString &, bool)
{

}

QString VirtualItem::text() const
{
    return QString();
}

void VirtualItem::setTitle(const QString &, bool)
{

}

QString VirtualItem::title() const
{
    return m_title;
}

KDateTime VirtualItem::date(PimItem::DateRole) const
{
    return KDateTime();
}

bool VirtualItem::setDate(PimItem::DateRole, const KDateTime &)
{
    return false;
}

void VirtualItem::setRelations(const QList<PimItemRelation> &)
{

}

QList<PimItemRelation> VirtualItem::relations() const
{
    return QList<PimItemRelation>();
}

KJob *VirtualItem::saveItem()
{
    return 0;
}

