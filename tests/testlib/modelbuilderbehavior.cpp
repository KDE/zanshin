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
#include "modelutils.h"

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
    , m_metadataCreationEnabled(true)
    , m_singleColumnEnabled(false)
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
    todo->setCategories(t.contexts);
    todo->setCompleted(t.state==Done);
    if (t.dueDate.isValid()) {
        todo->setDtDue(t.dueDate);
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
    addTodoMetadata(item, t);
    row << item;

    if (m_singleColumnEnabled) {
        return row;
    }

    item = new QStandardItem(t.parentUid);
    item->setData(QVariant::fromValue(it), Akonadi::EntityTreeModel::ItemRole);
    addTodoMetadata(item, t);
    row << item;

    item = new QStandardItem(t.contexts.join(", "));
    item->setData(QVariant::fromValue(it), Akonadi::EntityTreeModel::ItemRole);
    addTodoMetadata(item, t);
    row << item;

    item = new QStandardItem(t.dueDate.toString());
    item->setData(QVariant::fromValue(it), Akonadi::EntityTreeModel::ItemRole);
    addTodoMetadata(item, t);
    row << item;

    item = new QStandardItem;
    item->setData(QVariant::fromValue(it), Akonadi::EntityTreeModel::ItemRole);
    addTodoMetadata(item, t);
    row << item;

    return row;
}

QList<QStandardItem*> Zanshin::Test::StandardModelBuilderBehavior::expandContext(const Cat &c)
{
    QList<QStandardItem*> row;

    QStandardItem *item = new QStandardItem(c.name);
//     item->setData(QVariant::fromValue(QString(c.parentPath + c.name)), Zanshin::ContextPathRole);
    addContextMetadata(item);
    row << item;

    if (m_singleColumnEnabled) {
        return row;
    }

    for (int i=0; i<4; i++) {
        item = new QStandardItem;
        addContextMetadata(item);
        row << item;
    }

    return row;
}

QList<QStandardItem*> Zanshin::Test::StandardModelBuilderBehavior::expandCollection(const C &c)
{
    Akonadi::Collection col(c.id);
    col.setName(c.name);

    QList<QStandardItem*> row;

    QStandardItem *item = new QStandardItem(c.name);
    item->setData(QVariant::fromValue(col), Akonadi::EntityTreeModel::CollectionRole);
    item->setData(QVariant::fromValue(col.id()), Akonadi::EntityTreeModel::CollectionIdRole);
    addCollectionMetadata(item);
    row << item;

    if (m_singleColumnEnabled) {
        return row;
    }

    for (int i=0; i<4; i++) {
        item = new QStandardItem;
        item->setData(QVariant::fromValue(col), Akonadi::EntityTreeModel::CollectionRole);
        item->setData(QVariant::fromValue(col.id()), Akonadi::EntityTreeModel::CollectionIdRole);
        row << item;
    }

    return row;
}

QList<QStandardItem*> Zanshin::Test::StandardModelBuilderBehavior::expandVirtual(const V &virt)
{
    QList<QStandardItem*> row;

    QStandardItem *item = new QStandardItem(virt.name);
    item->setData(QVariant::fromValue(virt.name), Qt::DisplayRole);

    if (m_metadataCreationEnabled) {
        int type = 0;
        switch (virt.type) {
        case Inbox:
        case NoContext:
            type = Zanshin::Inbox;
            break;
        case Contexts:
            type = Zanshin::ContextRoot;
            break;
        }

        item->setData(QVariant::fromValue(type), Zanshin::ItemTypeRole);
    }

    row << item;

    if (m_singleColumnEnabled) {
        return row;
    }

    for (int i=0; i<4; i++) {
        item = new QStandardItem;
        row << item;
    }

    return row;
}

QList<QStandardItem*> Zanshin::Test::StandardModelBuilderBehavior::expandGeneric(const G &generic)
{
    QList<QStandardItem*> row;
    
    QStandardItem *item = new QStandardItem("generic"+QString::number(generic.id));
    item->setData(generic.id, IdRole);
    foreach(int role, generic.data.keys()) {
        item->setData(generic.data.value(role), role);
    }
    
    row << item;
    
    if (m_singleColumnEnabled) {
        return row;
    }
    
    for (int i=0; i<4; i++) {
        item = new QStandardItem;
        row << item;
    }
    
    return row;
}

void Zanshin::Test::StandardModelBuilderBehavior::setMetadataCreationEnabled(bool enabled)
{
    m_metadataCreationEnabled = enabled;
}

bool Zanshin::Test::StandardModelBuilderBehavior::isMetadataCreationEnabled()
{
    return m_metadataCreationEnabled;
}

void Zanshin::Test::StandardModelBuilderBehavior::addTodoMetadata(QStandardItem *item, const T &todo)
{
    if (m_metadataCreationEnabled) {
        if (!todo.parentUid.isEmpty()) {
            item->setData(QStringList() << todo.parentUid, Zanshin::ParentUidRole);
        }
        item->setData(todo.uid, Zanshin::UidRole);
        if (todo.todoTag==ProjectTag || todo.todoTag==ReferencedTag) {
            item->setData(QVariant::fromValue((int)Zanshin::ProjectTodo), Zanshin::ItemTypeRole);
        } else {
            item->setData(QVariant::fromValue((int)Zanshin::StandardTodo), Zanshin::ItemTypeRole);
        }
    }
}

void Zanshin::Test::StandardModelBuilderBehavior::addContextMetadata(QStandardItem *item)
{
    if (m_metadataCreationEnabled) {
        item->setData(QVariant::fromValue((int)Zanshin::Context), Zanshin::ItemTypeRole);
    }
}

void Zanshin::Test::StandardModelBuilderBehavior::addCollectionMetadata(QStandardItem *item)
{
    if (m_metadataCreationEnabled) {
        item->setData(QVariant::fromValue((int)Zanshin::Collection), Zanshin::ItemTypeRole);
    }
}

void Zanshin::Test::StandardModelBuilderBehavior::setSingleColumnEnabled(bool singleColumnEnabled)
{
    m_singleColumnEnabled = singleColumnEnabled;
}

bool Zanshin::Test::StandardModelBuilderBehavior::isSingleColumnEnabled()
{
    return m_singleColumnEnabled;
}
