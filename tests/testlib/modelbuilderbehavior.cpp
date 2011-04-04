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

#include "modelbuilderbehavior.h"

#include <KDE/Akonadi/EntityTreeModel>
#include <KDE/KCalCore/Todo>
#include "../../src/globaldefs.h"

using namespace Zanshin::Test;

ModelBuilderBehaviorBase::ModelBuilderBehaviorBase()
{

}

ModelBuilderBehaviorBase::~ModelBuilderBehaviorBase()
{

}

StandardModelBuilderBehavior::StandardModelBuilderBehavior()
    : ModelBuilderBehaviorBase()
{

}

StandardModelBuilderBehavior::~StandardModelBuilderBehavior()
{

}

QList<QStandardItem*> Zanshin::Test::StandardModelBuilderBehavior::expandTodo(const T &t)
{
    KCalCore::Todo::Ptr todo(new KCalCore::Todo);
    todo->setUid(t.uid);
    todo->setRelatedTo(t.parentUid);
    todo->setSummary(t.summary);
    todo->setCategories(t.categories);
    todo->setCompleted(t.state==Done);
    if (t.dueDate.isValid()) {
        todo->setDtDue(t.dueDate);
        todo->setHasDueDate(true);
        todo->setAllDay(true);
    }

    if (t.todoTag==ProjectTag) {
        todo->addComment("X-Zanshin-Project");
    }

    Akonadi::Item it(t.id);
    it.setParentCollection(Akonadi::Collection(t.parentId));
    it.setMimeType("application/x-vnd.akonadi.calendar.todo");
    it.setPayload<KCalCore::Todo::Ptr>(todo);


    QList<QStandardItem*> row;

    QStandardItem *item = new QStandardItem(t.summary);
    item->setData(QVariant::fromValue(it), Akonadi::EntityTreeModel::ItemRole);
    row << item;

    item = new QStandardItem(t.parentUid);
    item->setData(QVariant::fromValue(it), Akonadi::EntityTreeModel::ItemRole);
    row << item;

    item = new QStandardItem(t.categories.join(", "));
    item->setData(QVariant::fromValue(it), Akonadi::EntityTreeModel::ItemRole);
    row << item;

    item = new QStandardItem(t.dueDate.toString());
    item->setData(QVariant::fromValue(it), Akonadi::EntityTreeModel::ItemRole);
    row << item;

    item = new QStandardItem;
    item->setData(QVariant::fromValue(it), Akonadi::EntityTreeModel::ItemRole);
    row << item;

    return row;
}

QList<QStandardItem*> Zanshin::Test::StandardModelBuilderBehavior::expandCollection(const C &c)
{
    Akonadi::Collection col(c.id);
    col.setName(c.name);

    QList<QStandardItem*> row;

    QStandardItem *item = new QStandardItem(c.name);
    item->setData(QVariant::fromValue(col), Akonadi::EntityTreeModel::CollectionRole);
    row << item;

    for (int i=0; i<4; i++) {
        item = new QStandardItem;
        item->setData(QVariant::fromValue(col), Akonadi::EntityTreeModel::CollectionRole);
        row << item;
    }

    return row;
}

QList<QStandardItem*> Zanshin::Test::StandardModelBuilderBehavior::expandVirtual(const V &virt)
{
    QList<QStandardItem*> row;

    QStandardItem *item = new QStandardItem(virt.name);
    item->setData(QVariant::fromValue(virt.name), Qt::DisplayRole);
    row << item;

    item = new QStandardItem;
    item->setData(QVariant::fromValue((int)virt.type), Zanshin::ItemTypeRole);
    row << item;

    for (int i=0; i<3; i++) {
        item = new QStandardItem;
        row << item;
    }

    return row;
}
