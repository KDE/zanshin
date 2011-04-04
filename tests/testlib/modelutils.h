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

#ifndef ZANSHIN_TESTLIB_MODELUTILS_H
#define ZANSHIN_TESTLIB_MODELUTILS_H

#include <QtGui/QStandardItemModel>
#include <KDE/Akonadi/EntityTreeModel>

#include <testlib/modelpath.h>
#include <testlib/modelstructure.h>

#include <testlib/modelpath.h>

namespace Zanshin
{
namespace Test
{

enum Roles {
    TestDslRole = Akonadi::EntityTreeModel::UserRole + 101,
    UserRole = Akonadi::EntityTreeModel::UserRole + 200
};

class ModelBuilderBehaviorBase;

class ModelUtils
{
public:
    static void create(QStandardItemModel *model,
                       const ModelStructure &structure,
                       const ModelPath &root = ModelPath(),
                       ModelBuilderBehaviorBase *behavior = 0);

    static QModelIndex locateItem(QAbstractItemModel *model, const ModelPath &root);
    static bool destroy(QAbstractItemModel *model, const ModelPath::List &paths);
    static bool destroy(QAbstractItemModel *model, const ModelPath &path);

private:
    static QList< QList<QStandardItem*> > createItems(const ModelStructure &structure, ModelBuilderBehaviorBase *behavior);
    static QList<QStandardItem*> createItem(const ModelStructureTreeNode *node, ModelBuilderBehaviorBase *behavior);
};

} // namespace Test
} // namespace Zanshin

#endif

