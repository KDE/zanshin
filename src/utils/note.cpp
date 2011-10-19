/*
    Copyright (c) 2011 <chrigi_1@fastmail.fm>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "note.h"

#include <Akonadi/EntityDisplayAttribute>
#include <akonadi/notes/noteutils.h>

#include <KMime/Message>
#include <QCoreApplication>

Note::Note(QObject *parent)
:   AbstractPimItem(parent)
{
    //init payload, mimetype, and displayattribute
    commitData();
}
/*
Note::Note(const Note &note)
:   AbstractPimItem(note.getItem())
{
    m_text = note.m_text;
    m_title = note.m_title;
    m_creationDate = note.m_creationDate;
}*/

Note::Note(const Akonadi::Item &item, QObject *parent)
:   AbstractPimItem(item, parent)
{
    fetchData();
}

Note::Note(AbstractPimItem &item, QObject* parent)
:   AbstractPimItem(item, parent)
{
    commitData();
}


bool Note::hasValidPayload()
{
    if (m_item.hasPayload<KMime::Message::Ptr>()) {
        return true;
    }
    return false;
}


void Note::commitData()
{
    m_item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    Akonadi::NoteUtils::NoteMessageWrapper messageWrapper;
    messageWrapper.setTitle(m_title);
    messageWrapper.setText(m_text, m_textIsRich ? Qt::RichText : Qt::PlainText);
    messageWrapper.setCreationDate(m_creationDate);
    messageWrapper.setFrom(QCoreApplication::applicationName()+QCoreApplication::applicationVersion());
    m_item.setPayload(messageWrapper.message());
    
    Akonadi::EntityDisplayAttribute *eda = new Akonadi::EntityDisplayAttribute();
    eda->setIconName(getIconName());
    eda->setDisplayName(m_title);
    m_item.addAttribute(eda);
}

void Note::fetchData()
{
    if (m_dataFetched) {
        return;
    }
    
    if ( !hasValidPayload()) {
        kDebug() << "invalid payload";
        return;
    }
    
    KMime::Message::Ptr msg = m_item.payload<KMime::Message::Ptr>();
    Q_ASSERT(msg.get());
    Akonadi::NoteUtils::NoteMessageWrapper messageWrapper(msg);
    m_textIsRich = messageWrapper.textFormat() == Qt::RichText;
    m_titleIsRich = false;
    m_title = messageWrapper.title();
    m_text = messageWrapper.text();
    m_creationDate = messageWrapper.creationDate();

    m_dataFetched = true;
}


QString Note::mimeType()
{
    Q_ASSERT(AbstractPimItem::mimeType(AbstractPimItem::Note) == Akonadi::NoteUtils::noteMimeType());
    return AbstractPimItem::mimeType(AbstractPimItem::Note);
}

AbstractPimItem::ItemStatus Note::getStatus() const
{
    return AbstractPimItem::Later;
}


KDateTime Note::getPrimaryDate()
{
    return getLastModifiedDate();
}

QString Note::getIconName()
{
    return Akonadi::NoteUtils::noteIconName();
}


AbstractPimItem::ItemType Note::itemType()
{
    return AbstractPimItem::Note;
}
