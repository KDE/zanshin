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

#include "collectionitem.h"

#include <Akonadi/CollectionModifyJob>
#include <Akonadi/EntityDisplayAttribute>

CollectionItem::CollectionItem()
{
}

CollectionItem::CollectionItem(const Akonadi::Collection &collection)
    : m_collection(collection)
{
}

Akonadi::Collection CollectionItem::collection() const
{
    return m_collection;
}

PimItem::ItemType CollectionItem::itemType() const
{
    return m_collection.isValid() ? PimItem::Collection : PimItem::NoType;
}

QString CollectionItem::mimeType() const
{
    return Akonadi::Collection::mimeType();
}

PimItem::ItemStatus CollectionItem::status() const
{
    return Complete;
}

QString CollectionItem::uid() const
{
    return QString::number(m_collection.id());
}

QString CollectionItem::iconName() const
{
    return "folder";
}

void CollectionItem::setText(const QString &, bool)
{

}

QString CollectionItem::text() const
{
    return QString();
}

void CollectionItem::setTitle(const QString &, bool)
{

}

QString CollectionItem::title() const
{
    return QString();
}

KDateTime CollectionItem::date(PimItem::DateRole) const
{
    return KDateTime();
}

bool CollectionItem::setDate(PimItem::DateRole, const KDateTime &)
{
    return false;
}

void CollectionItem::setRelations(const QList<PimItemRelation> &)
{

}

QList<PimItemRelation> CollectionItem::relations() const
{
    return QList<PimItemRelation>();
}

KJob *CollectionItem::saveItem()
{
    return new Akonadi::CollectionModifyJob(m_collection);
}

