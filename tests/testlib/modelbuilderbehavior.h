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

#ifndef ZANSHIN_TESTLIB_MODELBUILDERBEHAVIOR_H
#define ZANSHIN_TESTLIB_MODELBUILDERBEHAVIOR_H

#include <QtCore/QList>
#include <QtGui/QStandardItemModel>

#include <testlib/c.h>
#include <testlib/cat.h>
#include <testlib/t.h>
#include <testlib/v.h>
#include <testlib/g.h>

namespace Zanshin
{
namespace Test
{


class ModelBuilderBehaviorBase
{
public:
    ModelBuilderBehaviorBase();
    virtual ~ModelBuilderBehaviorBase();

    virtual QList<QStandardItem*> expandTodo(const T &todo) = 0;
    virtual QList<QStandardItem*> expandCollection(const C &collection) = 0;
    virtual QList<QStandardItem*> expandCategory(const Cat &category) = 0;
    virtual QList<QStandardItem*> expandVirtual(const V &virt) = 0;
    virtual QList<QStandardItem*> expandGeneric(const G &generic) = 0;
};

class StandardModelBuilderBehavior : public ModelBuilderBehaviorBase
{
public:
    StandardModelBuilderBehavior();
    virtual ~StandardModelBuilderBehavior();

    virtual QList<QStandardItem*> expandTodo(const T &todo);
    virtual QList<QStandardItem*> expandCollection(const C &collection);
    virtual QList<QStandardItem*> expandCategory(const Cat &category);
    virtual QList<QStandardItem*> expandVirtual(const V &virt);
    virtual QList<QStandardItem*> expandGeneric(const G &generic);

    void setMetadataCreationEnabled(bool enabled);
    bool isMetadataCreationEnabled();

    void setSingleColumnEnabled(bool singleColumnEnabled);
    bool isSingleColumnEnabled();

private:
    void addTodoMetadata(QStandardItem*, const T &todo);
    void addCategoryMetadata(QStandardItem*);
    void addCollectionMetadata(QStandardItem*);
    bool m_metadataCreationEnabled;
    bool m_singleColumnEnabled;
};

} // namespace Test
} // namespace Zanshin

#endif

