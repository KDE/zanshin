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

#include "gentag.h"

#include "akonadi/akonadiserializerinterface.h"

using namespace Testlib;

GenTag::GenTag(const Akonadi::Tag &tag)
    : m_tag(tag)
{
}

GenTag::operator Akonadi::Tag()
{
    return m_tag;
}

GenTag &GenTag::withId(Akonadi::Tag::Id id)
{
    m_tag.setId(id);
    return *this;
}

GenTag &GenTag::withName(const QString &name)
{
    m_tag.setName(name);
    m_tag.setGid(name.toLatin1());
    return *this;
}

GenTag &GenTag::asContext()
{
    m_tag.setType(Akonadi::SerializerInterface::contextTagType());
    return *this;
}

GenTag &GenTag::asPlain()
{
    m_tag.setType(Akonadi::Tag::PLAIN);
    return *this;
}
