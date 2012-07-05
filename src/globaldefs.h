/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>

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

#ifndef ZANSHIN_GLOBALDEFS_H
#define ZANSHIN_GLOBALDEFS_H

#include <KDE/Akonadi/EntityTreeModel>
#include <QModelIndex>

namespace Zanshin
{
    enum ApplicationMode {
        ProjectMode = 0,
        CategoriesMode = 1,
        KnowledgeMode = 2
    };

    enum ItemType
    {
        StandardTodo = 0,
        ProjectTodo,
        Category,
        Collection,
        Inbox,
        CategoryRoot,
        Topic,
        TopicRoot
    };

    enum Roles {
        UidRole = Akonadi::EntityTreeModel::UserRole + 1,
        ParentUidRole,
        AncestorsUidRole,
        ItemTypeRole,
        DataTypeRole,
        ChildUidsRole,
        ChildIndexesRole,
        RelationIdRole, //Id of node
        AncestorsRole, //List of all parents
        UriRole,
        UserRole = Akonadi::EntityTreeModel::UserRole + 100
    };

    enum DataType {
        StandardType = 0,
        CategoryType,
        ProjectType
    };

}
typedef qint64 Id;
typedef QList<qint64> IdList;
Q_DECLARE_METATYPE(IdList)
Q_DECLARE_METATYPE(QModelIndexList)

#endif

