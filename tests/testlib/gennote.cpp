/* This file is part of Zanshin

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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

#include "gennote.h"

#include <algorithm>

#include <Akonadi/Notes/NoteUtils>
#include <KMime/Message>

using namespace Testlib;

GenNote::GenNote(const Akonadi::Item &item)
    : m_item(item)
{
    m_item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    if (!m_item.hasPayload<KMime::Message::Ptr>())
        m_item.setPayload(KMime::Message::Ptr(new KMime::Message));
}

GenNote::operator Akonadi::Item()
{
    return m_item;
}

GenNote &GenNote::withId(Akonadi::Item::Id id)
{
    m_item.setId(id);
    return *this;
}

GenNote &GenNote::withParent(Akonadi::Collection::Id id)
{
    m_item.setParentCollection(Akonadi::Collection(id));
    return *this;
}

GenNote &GenNote::withTags(const QList<Akonadi::Tag::Id> &ids)
{
    auto tags = Akonadi::Tag::List();
    std::transform(ids.constBegin(), ids.constEnd(),
                   std::back_inserter(tags),
                   [] (Akonadi::Tag::Id id) {
                       return Akonadi::Tag(id);
                   });
    m_item.setTags(tags);
    return *this;
}

GenNote &GenNote::withParentUid(const QString &uid)
{
    auto message = m_item.payload<KMime::Message::Ptr>();
    if (!uid.isEmpty()) {
        auto relatedHeader = new KMime::Headers::Generic("X-Zanshin-RelatedProjectUid");
        relatedHeader->from7BitString(uid.toUtf8());
        message->appendHeader(relatedHeader);
    } else {
        (void)message->removeHeader("X-Zanshin-RelatedProjectUid");
    }
    message->assemble();
    m_item.setPayload(message);
    return *this;
}

GenNote &GenNote::withTitle(const QString &title)
{
    auto message = m_item.payload<KMime::Message::Ptr>();
    Akonadi::NoteUtils::NoteMessageWrapper wrapper(message);
    wrapper.setTitle(title);
    m_item.setPayload(wrapper.message());
    return *this;
}

GenNote &GenNote::withText(const QString &text)
{
    auto message = m_item.payload<KMime::Message::Ptr>();
    Akonadi::NoteUtils::NoteMessageWrapper wrapper(message);
    wrapper.setText(text);
    m_item.setPayload(wrapper.message());
    return *this;
}
