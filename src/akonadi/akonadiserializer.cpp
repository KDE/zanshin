/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#include "akonadiserializer.h"

#include <Akonadi/Collection>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/Item>
#include <Akonadi/Notes/NoteUtils>
#include <KCalCore/Todo>

using namespace Akonadi;

Serializer::Serializer()
{
}

Serializer::~Serializer()
{
}

Domain::DataSource::Ptr Serializer::createDataSourceFromCollection(Collection collection)
{
    if (!collection.isValid())
        return Domain::DataSource::Ptr();

    auto dataSource = Domain::DataSource::Ptr::create();
    updateDataSourceFromCollection(dataSource, collection);
    return dataSource;
}

void Serializer::updateDataSourceFromCollection(Domain::DataSource::Ptr dataSource, Collection collection)
{
    if (!collection.isValid())
        return;

    QString name = collection.displayName();

    auto parent = collection.parentCollection();
    while (parent.isValid() && parent != Akonadi::Collection::root()) {
        name = parent.displayName() + "/" + name;
        parent = parent.parentCollection();
    }

    dataSource->setName(name);

    if (collection.hasAttribute<Akonadi::EntityDisplayAttribute>()) {
        auto iconName = collection.attribute<Akonadi::EntityDisplayAttribute>()->iconName();
        dataSource->setIconName(iconName);
    }
}

bool Akonadi::Serializer::isNoteCollection(Akonadi::Collection collection)
{
    return collection.contentMimeTypes().contains(NoteUtils::noteMimeType());
}

bool Akonadi::Serializer::isTaskCollection(Akonadi::Collection collection)
{
    return collection.contentMimeTypes().contains(KCalCore::Todo::todoMimeType());
}

Domain::Task::Ptr Serializer::createTaskFromItem(Item item)
{
    if (!item.hasPayload<KCalCore::Todo::Ptr>())
        return Domain::Task::Ptr();

    auto task = Domain::Task::Ptr::create();
    updateTaskFromItem(task, item);
    return task;
}

void Serializer::updateTaskFromItem(Domain::Task::Ptr task, Item item)
{
    if (!item.hasPayload<KCalCore::Todo::Ptr>())
        return;

    auto todo = item.payload<KCalCore::Todo::Ptr>();

    task->setTitle(todo->summary());
    task->setText(todo->description());
    task->setDone(todo->isCompleted());
    task->setStartDate(todo->dtStart().dateTime());
    task->setDueDate(todo->dtDue().dateTime());
    task->setProperty("todoUid", todo->uid());
}

bool Serializer::isTaskChild(Domain::Task::Ptr task, Akonadi::Item item)
{
    if (!item.hasPayload<KCalCore::Todo::Ptr>())
        return false;

    auto todo = item.payload<KCalCore::Todo::Ptr>();
    if (todo->relatedTo() == task->property("todoUid"))
        return true;

    return false;
}

Akonadi::Item Serializer::createItemFromTask(Domain::Task::Ptr task)
{
    auto todo = KCalCore::Todo::Ptr::create();

    todo->setSummary(task->title());
    todo->setDescription(task->text());
    todo->setCompleted(task->isDone());
    todo->setDtStart(KDateTime(task->startDate()));
    todo->setDtDue(KDateTime(task->dueDate()));

    Akonadi::Item item;
    item.setMimeType(KCalCore::Todo::todoMimeType());
    item.setPayload(todo);
    return item;
}

QString Serializer::relatedUidFromItem(Akonadi::Item item)
{
    if (!item.hasPayload<KCalCore::Todo::Ptr>())
        return QString();

    auto todo = item.payload<KCalCore::Todo::Ptr>();
    return todo->relatedTo();
}

Domain::Note::Ptr Serializer::createNoteFromItem(Akonadi::Item item)
{
    Q_UNUSED(item)
    qFatal("Not implemented yet");
    return Domain::Note::Ptr();
}

void Serializer::updateNoteFromItem(Domain::Note::Ptr note, Item item)
{
    Q_UNUSED(note)
    Q_UNUSED(item)
    qFatal("Not implemented yet");
}
