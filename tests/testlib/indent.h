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

#ifndef ZANSHIN_TESTLIB_INDENT_H
#define ZANSHIN_TESTLIB_INDENT_H

#include <QtCore/QVariant>

namespace Zanshin
{
namespace Test
{

struct Indent
{
public:
    explicit Indent(quint64 size = 0);

    Indent(const Indent &other);
    Indent &operator=(const Indent &other);

    quint64 size;
};

const Indent _(1);
const Indent __(2);
const Indent ___(3);
const Indent ____(4);
const Indent _____(5);
const Indent ______(6);
const Indent _______(7);
const Indent ________(8);
const Indent _________(9);
const Indent __________(10);

} // namespace Test
} // namespace Zanshin

Q_DECLARE_METATYPE(Zanshin::Test::Indent)

#endif

