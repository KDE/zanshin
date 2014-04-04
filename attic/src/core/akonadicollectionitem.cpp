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

#include "akonadicollectionitem.h"

#include <Akonadi/CollectionModifyJob>
#include <Akonadi/EntityDisplayAttribute>

AkonadiCollectionItem::AkonadiCollectionItem()
{
}

AkonadiCollectionItem::AkonadiCollectionItem(const Akonadi::Collection &collection)
    : m_collection(collection)
{
}

Akonadi::Collection AkonadiCollectionItem::collection() const
{
    return m_collection;
}

PimItem::ItemType AkonadiCollectionItem::itemType() const
{
    return m_collection.isValid() ? PimItem::Collection : PimItem::NoType;
}

QString AkonadiCollectionItem::mimeType() const
{
    return Akonadi::Collection::mimeType();
}

PimItem::ItemStatus AkonadiCollectionItem::status() const
{
    return Complete;
}

QString AkonadiCollectionItem::uid() const
{
    return QString::number(m_collection.id());
}

QString AkonadiCollectionItem::iconName() const
{
    return "folder";
}

void AkonadiCollectionItem::setText(const QString &, bool)
{

}

QString AkonadiCollectionItem::text() const
{
    return QString();
}

void AkonadiCollectionItem::setTitle(const QString &, bool)
{

}

QString AkonadiCollectionItem::title() const
{
    return QString();
}

KDateTime AkonadiCollectionItem::date(PimItem::DateRole) const
{
    return KDateTime();
}

bool AkonadiCollectionItem::setDate(PimItem::DateRole, const KDateTime &)
{
    return false;
}

void AkonadiCollectionItem::setRelations(const QList<PimItemRelation> &)
{

}

QList<PimItemRelation> AkonadiCollectionItem::relations() const
{
    return QList<PimItemRelation>();
}

KJob *AkonadiCollectionItem::saveItem()
{
    return new Akonadi::CollectionModifyJob(m_collection);
}

