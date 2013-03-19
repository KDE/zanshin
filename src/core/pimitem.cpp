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
#include <Akonadi/Monitor>
#include <Akonadi/Session>
#include <qdom.h>


PimItem::PimItem(QObject *parent)
: QObject(parent),
m_dataFetched(false),
m_textIsRich(false),
m_titleIsRich(false),
m_itemOutdated(false)
{

}

PimItem::PimItem(const Akonadi::Item &item, QObject *parent)
: QObject(parent),
m_dataFetched(false),
m_textIsRich(false),
m_titleIsRich(false),
m_itemOutdated(false)
{
    m_item = item;
}

PimItem::PimItem(PimItem &item, QObject* parent)
:   QObject(parent),
m_dataFetched(false),
m_textIsRich(false),
m_titleIsRich(false),
m_itemOutdated(false)
{
    m_title = item.getTitle();
    m_textIsRich = item.textIsRich();
    m_text = item.getText();
    m_titleIsRich = item.titleIsRich();
    m_attachments = item.getAttachments();
    m_dataFetched = true;
}

PimItem::~PimItem()
{

}

PimItem::ItemType PimItem::itemType(const Akonadi::Item &item)
{
    Q_ASSERT(!item.mimeType().isEmpty());
    if (item.mimeType() == mimeType(Note)) {
        return Note;
    } else if (item.mimeType() == mimeType(Event)) {
        return Event;
    } else if (item.mimeType() == mimeType(Todo)) {
        return Todo;
    }
    kWarning() << "attention, unknown type" << item.mimeType();
    //Q_ASSERT(false);
    return Unknown;
}

QString PimItem::getUid()
{
    fetchData();
    return m_uid;
}

void PimItem::setText(const QString &text, bool isRich)
{
    m_textIsRich = isRich;
    m_text = text;
}

QString PimItem::getText()
{
    fetchData();
    return m_text;
}

void PimItem::setTitle(const QString &text, bool isRich)
{
    m_titleIsRich = isRich;
    m_title = text;
    if (m_item.hasAttribute<Akonadi::EntityDisplayAttribute>()) {
        commitData(); //We need to commit already to update the EDA
    }
}

QString PimItem::getTitle()
{
    if (m_item.hasAttribute<Akonadi::EntityDisplayAttribute>()) {
        Akonadi::EntityDisplayAttribute *att = m_item.attribute<Akonadi::EntityDisplayAttribute>();
        m_title = att->displayName();
        return att->displayName();
    }
    fetchData();
    return m_title;
}

KDateTime PimItem::getLastModifiedDate()
{
    if (!m_item.isValid()) {
        kWarning() << "invalid item";
        return KDateTime();
    }
    return KDateTime(m_item.modificationTime(), KDateTime::LocalZone);
}

void PimItem::setCreationDate(const KDateTime &creationDate)
{
    m_creationDate = creationDate;
}

KDateTime PimItem::getCreationDate()
{
    fetchData();
    return m_creationDate;
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
    if (m_itemOutdated) {
        kWarning() << "the item is outdated";
    }
    if (!m_item.isValid()) {
        kWarning() << "invalid item";
    }
    return m_item;
}



bool PimItem::textIsRich()
{
    fetchData();
    return m_textIsRich;
}

bool PimItem::titleIsRich()
{
    fetchData();
    return m_titleIsRich;
}

const KCalCore::Attachment::List PimItem::getAttachments()
{
    fetchData();
    return m_attachments;
}


void PimItem::setCategories(const QStringList& )
{

}

QStringList PimItem::getCategories()
{
    return QStringList();
}

//TODO return false if this fails, so the user is notified. Otherwise this could result in dataloss
void PimItem::saveItem()
{
    kDebug();
    if (m_itemOutdated) {
        kWarning() << "item fetch in progress, cannot save without conflict";
        return;
    }
    
    if (!m_item.isValid()) {
        commitData(); //We still commit the data also to an invalid item (so we can create the item afterwards
        kWarning() << "invalid item";
        return;
    }

    if (!hasValidPayload()) { //TODO is this really invalid, or couldn't we save also if ther is no payload?
        kWarning() << "tried to save item without payload";
        return;
    }
    if (!m_dataFetched) {
        kDebug() << "data not fetched from payload yet, fetching";
        fetchData();
    }
    commitData();
    m_itemOutdated = true;

    //TODO only commit but don't write to akonadi?
    new Akonadi::ItemModifyJob(m_item);
}





