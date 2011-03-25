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

#ifndef ZANSHIN_TESTLIB_MODELPATH_H
#define ZANSHIN_TESTLIB_MODELPATH_H

#include <QtCore/QVariant>

#include <testlib/c.h>
#include <testlib/t.h>

namespace Zanshin
{
namespace Test
{

class ModelUtils;

class ModelPath
{
public:
    typedef QList<ModelPath> List;

    ModelPath();
    ModelPath(const ModelPath &other);
    ModelPath &operator=(const ModelPath &other);

    ModelPath(const C &collection1, const C &collection2);
    ModelPath(const C &collection, const T &todo);
    ModelPath(const ModelPath &path, const C &collection);
    ModelPath(const ModelPath &path, const T &todo);
    ModelPath(const ModelPath &path1, const ModelPath &path2);

private:
    friend class ModelUtils;

    QVariantList m_path;
};

} // namespace Test
} // namespace Zanshin

Zanshin::Test::ModelPath operator%(const Zanshin::Test::C &collection1, const Zanshin::Test::C &collection2);
Zanshin::Test::ModelPath operator%(const Zanshin::Test::C &collection, const Zanshin::Test::T &todo);
Zanshin::Test::ModelPath operator%(const Zanshin::Test::ModelPath &path, const Zanshin::Test::C &collection);
Zanshin::Test::ModelPath operator%(const Zanshin::Test::ModelPath &path, const Zanshin::Test::T &todo);
Zanshin::Test::ModelPath operator%(const Zanshin::Test::ModelPath &path1, const Zanshin::Test::ModelPath &path2);

Q_DECLARE_METATYPE(Zanshin::Test::ModelPath)
Q_DECLARE_METATYPE(Zanshin::Test::ModelPath::List)

#endif

