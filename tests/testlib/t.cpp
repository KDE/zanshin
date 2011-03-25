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

#include "t.h"

#include <algorithm>

using namespace Zanshin::Test;

T::T()
{
}

T::T(qint64 i,
     qint64 pI,
     const QString &u,
     const QString &pU,
     const QString &s,
     TodoState st,
     const QString &d,
     const QString &c)
    : id(i),
      parentId(pI),
      uid(u),
      parentUid(pU),
      state(st),
      summary(s)
{
    if (!d.isEmpty()) {
        dueDate = KDateTime::fromString(d);
        Q_ASSERT(dueDate.isValid());
    }

    categories = c.split(QRegExp("\\s*,\\s*"), QString::SkipEmptyParts);
}

T::T(const T &other)
    : id(other.id),
      parentId(other.parentId),
      uid(other.uid),
      parentUid(other.parentUid),
      state(other.state),
      summary(other.summary),
      dueDate(other.dueDate),
      categories(other.categories)
{
}

T &T::operator=(const T &other)
{
    T t(other);
    std::swap(*this, t);
    return *this;
}

bool T::operator==(const T &other) const
{
    return id==other.id
        && parentId==other.parentId
        && uid==other.uid
        && parentUid==other.parentUid
        && state==other.state
        && summary==other.summary
        && dueDate==other.dueDate
        && categories==other.categories;
}

