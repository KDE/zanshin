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

#ifndef ZANSHIN_TESTLIB_T_H
#define ZANSHIN_TESTLIB_T_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <KDE/KDateTime>

namespace Zanshin
{
namespace Test
{

enum TodoState {
    InProgress = 0,
    Done
};

enum TodoTag {
    NoTag = 0,
    ProjectTag,
    ReferencedTag
};

struct T // Stands for todo
{
public:
    typedef QList<T> List;

    T();
    T(qint64 id,
      qint64 parentId,
      const QString &uid,
      const QString &parentUid,
      const QString &summary,
      TodoState state = Done,
      TodoTag todoTag = NoTag,
      const QString &date = QString(),
      const QString &categories = QString());

    bool operator==(const T &other) const;

    qint64 id;
    qint64 parentId;
    QString uid;
    QString parentUid;
    TodoState state;
    TodoTag todoTag;
    QString summary;
    KDateTime dueDate;
    QStringList categories;
};

} // namespace Test
} // namespace Zanshin

Q_DECLARE_METATYPE(Zanshin::Test::T)
Q_DECLARE_METATYPE(Zanshin::Test::T::List)
Q_DECLARE_METATYPE(Zanshin::Test::TodoState)

#endif

