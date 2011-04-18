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

#ifndef ZANSHIN_TESTLIB_MODELNODE_H
#define ZANSHIN_TESTLIB_MODELNODE_H

#include <QtCore/QVariant>

#include <testlib/c.h>
#include <testlib/cat.h>
#include <testlib/indent.h>
#include <testlib/t.h>
#include <testlib/v.h>

namespace Zanshin
{
namespace Test
{

class ModelNode
{
public:
    ModelNode();
    ModelNode(const C &collection, const Indent &indent = Indent());
    ModelNode(const T &todo, const Indent &indent = Indent());
    ModelNode(const Cat &category, const Indent &indent = Indent());
    ModelNode(const V &virt, const Indent &indent = Indent());

    quint64 indent() const;
    QVariant entity() const;

private:
    QVariant m_entity;
    Indent m_indent;
};

} // namespace Test
} // namespace Zanshin

Zanshin::Test::ModelNode operator+(const Zanshin::Test::Indent& indent, const Zanshin::Test::C &collection);
Zanshin::Test::ModelNode operator+(const Zanshin::Test::Indent& indent, const Zanshin::Test::T &todo);
Zanshin::Test::ModelNode operator+(const Zanshin::Test::Indent& indent, const Zanshin::Test::Cat &category);
Zanshin::Test::ModelNode operator+(const Zanshin::Test::Indent& indent, const Zanshin::Test::V &virt);

Q_DECLARE_METATYPE(Zanshin::Test::ModelNode)

#endif

