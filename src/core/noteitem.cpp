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

#include "noteitem.h"
#include "pimitemrelations.h"

#include <Akonadi/EntityDisplayAttribute>
#include <akonadi/notes/noteutils.h>

#include <KMime/Message>
#include <QCoreApplication>
#include <quuid.h>

typedef Akonadi::NoteUtils::NoteMessageWrapper NoteWrapper;
typedef QSharedPointer<Akonadi::NoteUtils::NoteMessageWrapper> NoteWrapperPtr;

NoteWrapperPtr unpack(const Akonadi::Item &item)
{
    Q_ASSERT(item.hasPayload<KMime::Message::Ptr>());
    return NoteWrapperPtr(new Akonadi::NoteUtils::NoteMessageWrapper(item.payload<KMime::Message::Ptr>()));
}

NoteItem::NoteItem()
:   PimItem(),
    messageWrapper(new Akonadi::NoteUtils::NoteMessageWrapper)
{
    messageWrapper->setUid(QUuid::createUuid());
    commitData();
}

NoteItem::NoteItem(const Akonadi::Item &item)
:   PimItem(item),
    messageWrapper(unpack(item))
{
}

void NoteItem::setItem(const Akonadi::Item &item)
{
    PimItem::setItem(item);
    messageWrapper = unpack(item);
}

bool NoteItem::hasValidPayload()
{
    return m_item.hasPayload<KMime::Message::Ptr>();
}

QString NoteItem::getUid()
{
    return messageWrapper->uid();
}

void NoteItem::setText(const QString &text, bool isRich)
{ 
    messageWrapper->setText(text, isRich ? Qt::RichText : Qt::PlainText);
    commitData();
}

QString NoteItem::getText()
{
    return messageWrapper->text();
}

void NoteItem::setTitle(const QString &title, bool isRich)
{
    Q_UNUSED(isRich);
    messageWrapper->setTitle(title);
    Akonadi::EntityDisplayAttribute *eda = m_item.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Entity::AddIfMissing);
    eda->setIconName(getIconName());
    eda->setDisplayName(title);
    commitData();
}

QString NoteItem::getTitle()
{
    return messageWrapper->title();
}

void NoteItem::setCreationDate(const KDateTime &date)
{
    messageWrapper->setCreationDate(date);
    commitData();
}

KDateTime NoteItem::getCreationDate()
{
    return messageWrapper->creationDate();
}

void NoteItem::commitData()
{
    m_item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    messageWrapper->setFrom(QCoreApplication::applicationName()+QCoreApplication::applicationVersion());
    messageWrapper->setLastModifiedDate(KDateTime::currentUtcDateTime());
    m_item.setPayload(messageWrapper->message());
}

QString NoteItem::mimeType()
{
    Q_ASSERT(PimItem::mimeType(PimItem::Note) == Akonadi::NoteUtils::noteMimeType());
    return PimItem::mimeType(PimItem::Note);
}

PimItem::ItemStatus NoteItem::getStatus() const
{
    return PimItem::Later;
}

KDateTime NoteItem::getPrimaryDate()
{
    return getLastModifiedDate();
}

QString NoteItem::getIconName()
{
    return Akonadi::NoteUtils::noteIconName();
}

KDateTime NoteItem::getLastModifiedDate()
{
    const KDateTime lastMod = messageWrapper->lastModifiedDate();
    if (lastMod.isValid()) {
        return lastMod.toLocalZone();
    }
    return PimItem::getLastModifiedDate();
}

PimItem::ItemType NoteItem::itemType()
{
    return PimItem::Note;
}

QList< PimItemRelation > NoteItem::getRelations()
{
    const QList<QString> xml = messageWrapper->custom().values("x-related");
    QList< PimItemRelation > relations;
    foreach(const QString &x, xml) {
        relations << relationFromXML(x.toLatin1());
    }
    return relations;
}

void NoteItem::setRelations(const QList< PimItemRelation > &relations)
{
    messageWrapper->custom().remove("x-related");
    foreach(const PimItemRelation &rel, relations) {
        messageWrapper->custom().insert("x-related", relationToXML(removeDuplicates(rel)));
    }
    commitData();
}
