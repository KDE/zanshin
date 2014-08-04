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
#include <KMime/Message>

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

    dataSource->setProperty("collectionId", collection.id());
}

Collection Serializer::createCollectionFromDataSource(Domain::DataSource::Ptr dataSource)
{
    const auto id = dataSource->property("collectionId").value<Collection::Id>();
    return Collection(id);
}

bool Akonadi::Serializer::isNoteCollection(Akonadi::Collection collection)
{
    return collection.contentMimeTypes().contains(NoteUtils::noteMimeType());
}

bool Akonadi::Serializer::isTaskCollection(Akonadi::Collection collection)
{
    return collection.contentMimeTypes().contains(KCalCore::Todo::todoMimeType());
}

bool Serializer::isTaskItem(Item item)
{
    if (!item.hasPayload<KCalCore::Todo::Ptr>())
        return false;

    auto todo = item.payload<KCalCore::Todo::Ptr>();
    return todo->customProperty("Zanshin", "Project").isEmpty();
}

Domain::Task::Ptr Serializer::createTaskFromItem(Item item)
{
    if (!isTaskItem(item))
        return Domain::Task::Ptr();

    auto task = Domain::Task::Ptr::create();
    updateTaskFromItem(task, item);
    return task;
}

void Serializer::updateTaskFromItem(Domain::Task::Ptr task, Item item)
{
    if (!isTaskItem(item))
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
    if (!isTaskItem(item))
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
    if (task->property("itemId").isValid()) {
        item.setId(task->property("itemId").value<Akonadi::Item::Id>());
    }
    item.setMimeType(KCalCore::Todo::todoMimeType());
    item.setPayload(todo);
    return item;
}

QString Serializer::relatedUidFromItem(Akonadi::Item item)
{
    if (isTaskItem(item)) {
        const auto todo = item.payload<KCalCore::Todo::Ptr>();
        return todo->relatedTo();

    } else if (isNoteItem(item)) {
        const auto message = item.payload<KMime::Message::Ptr>();
        const auto relatedHeader = message->headerByType("X-Zanshin-RelatedProjectUid");
        return relatedHeader ? relatedHeader->asUnicodeString() : QString();

    } else {
        return QString();
    }
}

void Serializer::updateItemParent(Akonadi::Item item, Domain::Task::Ptr parent)
{
    if (!isTaskItem(item))
        return;

    auto todo = item.payload<KCalCore::Todo::Ptr>();
    todo->setRelatedTo(parent->property("todoUid").toString());
}

void Serializer::removeItemParent(Akonadi::Item item)
{
    if (!isTaskItem(item))
        return;

    auto todo = item.payload<KCalCore::Todo::Ptr>();
    todo->setRelatedTo(QString());
}

Akonadi::Item::List Serializer::filterDescendantItems(const Akonadi::Item::List &potentialChildren, const Akonadi::Item &ancestorItem)
{
    if (potentialChildren.isEmpty())
        return Akonadi::Item::List();

    Akonadi::Item::List itemsToProcess = potentialChildren;
    Q_ASSERT(ancestorItem.isValid() && ancestorItem.hasPayload<KCalCore::Todo::Ptr>());
    KCalCore::Todo::Ptr todo = ancestorItem.payload<KCalCore::Todo::Ptr>();

    const auto bound = std::partition(itemsToProcess.begin(), itemsToProcess.end(),
                                      [ancestorItem, todo](Akonadi::Item currentItem) {
                                          return (!currentItem.hasPayload<KCalCore::Todo::Ptr>()
                                               || currentItem == ancestorItem
                                               || currentItem.payload<KCalCore::Todo::Ptr>()->relatedTo() != todo->uid());
                                      });

    Akonadi::Item::List itemsRemoved;
    std::copy(itemsToProcess.begin(), bound, std::back_inserter(itemsRemoved));
    itemsToProcess.erase(itemsToProcess.begin(), bound);

    auto result = std::accumulate(itemsToProcess.begin(), itemsToProcess.end(), Akonadi::Item::List(),
                                 [this, itemsRemoved](Akonadi::Item::List result, Akonadi::Item currentItem) {
                                     result << currentItem;
                                     return result += filterDescendantItems(itemsRemoved, currentItem);
                                 });

    return result;
}

bool Serializer::isNoteItem(Item item)
{
    return item.hasPayload<KMime::Message::Ptr>();
}

Domain::Note::Ptr Serializer::createNoteFromItem(Akonadi::Item item)
{
    if (!isNoteItem(item))
        return Domain::Note::Ptr();

    Domain::Note::Ptr note = Domain::Note::Ptr::create();
    updateNoteFromItem(note, item);

    return note;
}

void Serializer::updateNoteFromItem(Domain::Note::Ptr note, Item item)
{
    if (!isNoteItem(item))
        return;

    NoteUtils::NoteMessageWrapper wrappedNote(item.payload<KMime::Message::Ptr>());

    note->setTitle(wrappedNote.title());
    note->setText(wrappedNote.text());
}

Item Serializer::createItemFromNote(Domain::Note::Ptr note)
{
    NoteUtils::NoteMessageWrapper builder;
    builder.setTitle(note->title());
    builder.setText(note->text());

    Akonadi::Item item;
    item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    item.setPayload(builder.message());
    return item;
}

bool Serializer::isProjectItem(Item item)
{
    if (!item.hasPayload<KCalCore::Todo::Ptr>())
        return false;

    return !isTaskItem(item);
}

Domain::Project::Ptr Serializer::createProjectFromItem(Item item)
{
    if (!isProjectItem(item))
        return Domain::Project::Ptr();

    auto project = Domain::Project::Ptr::create();
    updateProjectFromItem(project, item);
    return project;
}

void Serializer::updateProjectFromItem(Domain::Project::Ptr project, Item item)
{
    if (!isProjectItem(item))
        return;

    auto todo = item.payload<KCalCore::Todo::Ptr>();

    project->setName(todo->summary());
    project->setProperty("itemId", item.id());
    project->setProperty("todoUid", todo->uid());
}

Item Serializer::createItemFromProject(Domain::Project::Ptr project)
{
    auto todo = KCalCore::Todo::Ptr::create();

    todo->setSummary(project->name());

    if (project->property("todoUid").isValid()) {
        todo->setUid(project->property("todoUid").toString());
    }

    Akonadi::Item item;
    if (project->property("itemId").isValid()) {
        item.setId(project->property("itemId").value<Akonadi::Item::Id>());
    }
    item.setMimeType(KCalCore::Todo::todoMimeType());
    item.setPayload(todo);
    return item;
}

Domain::Context::Ptr Serializer::createContextFromTag(Akonadi::Tag tag)
{
    if (!isContext(tag))
        return Domain::Context::Ptr();

    auto context = Domain::Context::Ptr::create();
    updateContextFromTag(context, tag);
    return context;
}

void Serializer::updateContextFromTag(Domain::Context::Ptr context, Akonadi::Tag tag)
{
    if (!isContext(tag))
        return;

    context->setProperty("tagId", tag.id());
    context->setName(tag.name());
}

bool Serializer::hasContextTags(Item item) const
{
    using namespace std::placeholders;
    Tag::List tags = item.tags();
    return std::any_of(tags.constBegin(), tags.constEnd(),
                       std::bind(std::mem_fn(&Serializer::isContext), this, _1));
}

bool Serializer::hasTopicTags(Item item) const
{
    using namespace std::placeholders;
    Tag::List tags = item.tags();
    return std::any_of(tags.constBegin(), tags.constEnd(),
                       std::bind(std::mem_fn(&Serializer::isTopic), this, _1));
}

bool Serializer::isContext(const Akonadi::Tag &tag) const
{
    return (tag.type() == Akonadi::SerializerInterface::contextTagType());
}

bool Serializer::isTopic(const Tag &tag) const
{
    return (tag.type() == Akonadi::SerializerInterface::topicTagType());
}

bool Serializer::isContextTag(const Domain::Context::Ptr &context, const Akonadi::Tag &tag) const
{
    return (context->property("tagId").value<Akonadi::Tag::Id>() == tag.id());
}

bool Serializer::isContextChild(const Domain::Context::Ptr &context, const Tag &tag) const
{
    // NOTE : Hierarchy support will be done later, waiting for a proper support of tag Parent()
    Q_UNUSED(context);
    Q_UNUSED(tag);
    return false;
}
