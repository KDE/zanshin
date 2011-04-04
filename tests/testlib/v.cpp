/* This file is part of Zanshin Todo.

   Copyright 2011 Kevin Ottens <ervin@kde.org>

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

#include "v.h"

#include <algorithm>

using namespace Zanshin::Test;

V::V()
{
}

V::V(VirtualType t)
    : type(t)
{
    switch (t) {
    case Inbox:
        name = "Inbox";
        break;
    case NoCategory:
        name = "No Category";
        break;
    case Categories:
        name = "Categories";
        break;
    }
}

V::V(const V &other)
    : type(other.type)
    , name(other.name)
{
}

V &V::operator=(const V &other)
{
    V v(other);
    std::swap(*this, v);
    return *this;
}

bool V::operator==(const V &other) const
{
    return type==other.type;
}

