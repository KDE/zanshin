/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Rémi Benoit <r3m1.benoit@gmail.com>

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

#include <AkonadiCore/Collection>
#include <AkonadiCore/EntityDisplayAttribute>
#include <AkonadiCore/Item>
#include <Akonadi/Notes/NoteUtils>
#include <KCalCore/Todo>
#include <KMime/Message>
#include <QMimeDatabase>

#include <numeric>

#include "utils/mem_fn.h"

#include "akonadi/akonadiapplicationselectedattribute.h"
#include "akonadi/akonaditimestampattribute.h"

using namespace Akonadi;

Serializer::Serializer()
{
}

Serializer::~Serializer()
{
}

bool Serializer::representsCollection(SerializerInterface::QObjectPtr object, Collection collection)
{
    return object->property("collectionId").toLongLong() == collection.id();
}

bool Serializer::representsItem(QObjectPtr object, Item item)
{
    return object->property("itemId").toLongLong() == item.id();
}

bool Serializer::representsAkonadiTag(Domain::Tag::Ptr tag, Tag akonadiTag) const
{
    return tag->property("tagId").toLongLong() == akonadiTag.id();
}

QString Serializer::objectUid(SerializerInterface::QObjectPtr object)
{
    return object->property("todoUid").toString();
}

QString Serializer::itemUid(const Item &item)
{
    if (isTaskItem(item)) {
        const auto todo = item.payload<KCalCore::Todo::Ptr>();
        return todo->uid();
    } else {
        return QString();
    }
}

Domain::DataSource::Ptr Serializer::createDataSourceFromCollection(Collection collection, DataSourceNameScheme naming)
{
    if (!collection.isValid())
        return Domain::DataSource::Ptr();

    auto dataSource = Domain::DataSource::Ptr::create();
    updateDataSourceFromCollection(dataSource, collection, naming);
    return dataSource;
}

void Serializer::updateDataSourceFromCollection(Domain::DataSource::Ptr dataSource, Collection collection, DataSourceNameScheme naming)
{
    if (!collection.isValid())
        return;

    QString name = collection.displayName();

    if (naming == FullPath) {
        auto parent = collection.parentCollection();
        while (parent.isValid() && parent != Akonadi::Collection::root()) {
            name = parent.displayName() + " » " + name;
            parent = parent.parentCollection();
        }
    }

    dataSource->setName(name);

    const auto mimeTypes = collection.contentMimeTypes();
    auto types = Domain::DataSource::ContentTypes();
    if (mimeTypes.contains(NoteUtils::noteMimeType()))
        types |= Domain::DataSource::Notes;
    if (mimeTypes.contains(KCalCore::Todo::todoMimeType()))
        types |= Domain::DataSource::Tasks;
    dataSource->setContentTypes(types);

    if (collection.hasAttribute<Akonadi::EntityDisplayAttribute>()) {
        auto iconName = collection.attribute<Akonadi::EntityDisplayAttribute>()->iconName();
        dataSource->setIconName(iconName);
    }

    if (!collection.hasAttribute<Akonadi::ApplicationSelectedAttribute>()) {
        dataSource->setSelected(true);
    } else {
        auto isSelected = collection.attribute<Akonadi::ApplicationSelectedAttribute>()->isSelected();
        dataSource->setSelected(isSelected);
    }

    dataSource->setProperty("collectionId", collection.id());
}

Collection Serializer::createCollectionFromDataSource(Domain::DataSource::Ptr dataSource)
{
    const auto id = dataSource->property("collectionId").value<Collection::Id>();
    auto collection = Collection(id);

    collection.attribute<Akonadi::TimestampAttribute>(Akonadi::Collection::AddIfMissing);

    auto selectedAttribute = collection.attribute<Akonadi::ApplicationSelectedAttribute>(Akonadi::Collection::AddIfMissing);
    selectedAttribute->setSelected(dataSource->isSelected());

    return collection;
}

bool Serializer::isSelectedCollection(Collection collection)
{
    if (!isNoteCollection(collection) && !isTaskCollection(collection))
        return false;

    if (!collection.hasAttribute<Akonadi::ApplicationSelectedAttribute>())
        return true;

    return collection.attribute<Akonadi::ApplicationSelectedAttribute>()->isSelected();
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
    task->setDoneDate(todo->completed().dateTime().toUTC());
    task->setStartDate(todo->dtStart().dateTime().toUTC());
    task->setDueDate(todo->dtDue().dateTime().toUTC());
    task->setProperty("itemId", item.id());
    task->setProperty("parentCollectionId", item.parentCollection().id());
    task->setProperty("todoUid", todo->uid());
    task->setProperty("relatedUid", todo->relatedTo());
    task->setRunning(todo->customProperty("Zanshin", "Running") == QLatin1String("1"));

    switch (todo->recurrence()->recurrenceType()) {
    case KCalCore::Recurrence::rDaily:
        task->setRecurrence(Domain::Task::RecursDaily);
        break;
    case KCalCore::Recurrence::rWeekly:
        task->setRecurrence(Domain::Task::RecursWeekly);
        break;
    case KCalCore::Recurrence::rMonthlyDay:
        task->setRecurrence(Domain::Task::RecursMonthly);
        break;
    default:
        // Other cases are not supported for now and as such just ignored
        break;
    }

    QMimeDatabase mimeDb;
    const auto attachmentsInput = todo->attachments();
    Domain::Task::Attachments attachments;
    attachments.reserve(attachmentsInput.size());
    std::transform(attachmentsInput.cbegin(), attachmentsInput.cend(),
                   std::back_inserter(attachments),
                   [&mimeDb] (const KCalCore::Attachment::Ptr &attach) {
                       Domain::Task::Attachment attachment;
                       if (attach->isUri())
                           attachment.setUri(attach->uri());
                       else
                           attachment.setData(attach->decodedData());
                       attachment.setLabel(attach->label());
                       attachment.setMimeType(attach->mimeType());
                       attachment.setIconName(mimeDb.mimeTypeForName(attach->mimeType()).iconName());
                       return attachment;
                   });
    task->setAttachments(attachments);

    if (todo->attendeeCount() > 0) {
        const auto attendees = todo->attendees();
        const auto delegate = std::find_if(attendees.begin(), attendees.end(),
                                           [] (const KCalCore::Attendee::Ptr &attendee) {
                                               return attendee->status() == KCalCore::Attendee::Accepted;
                                           });
        if (delegate != attendees.end()) {
            task->setDelegate(Domain::Task::Delegate((*delegate)->name(), (*delegate)->email()));
        }
    }
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

    todo->setDtStart(KDateTime(task->startDate(), KDateTime::UTC));
    todo->setDtDue(KDateTime(task->dueDate(), KDateTime::UTC));

    if (task->property("todoUid").isValid()) {
        todo->setUid(task->property("todoUid").toString());
    }

    if (task->property("relatedUid").isValid()) {
        todo->setRelatedTo(task->property("relatedUid").toString());
    }

    switch (task->recurrence()) {
    case Domain::Task::NoRecurrence:
        break;
    case Domain::Task::RecursDaily:
        todo->recurrence()->setDaily(1);
        break;
    case Domain::Task::RecursWeekly:
        todo->recurrence()->setWeekly(1);
        break;
    case Domain::Task::RecursMonthly:
        todo->recurrence()->setMonthly(1);
        break;
    }

    for (const auto &attachment : task->attachments()) {
        KCalCore::Attachment::Ptr attach(new KCalCore::Attachment(QByteArray()));
        if (attachment.isUri())
            attach->setUri(attachment.uri().toString());
        else
            attach->setDecodedData(attachment.data());
        attach->setMimeType(attachment.mimeType());
        attach->setLabel(attachment.label());
        todo->addAttachment(attach);
    }

    if (task->delegate().isValid()) {
        KCalCore::Attendee::Ptr attendee(new KCalCore::Attendee(task->delegate().name(),
                                                                task->delegate().email(),
                                                                true,
                                                                KCalCore::Attendee::Accepted));
        todo->addAttendee(attendee);
    }
    if (task->isRunning()) {
        todo->setCustomProperty("Zanshin", "Running", "1");
    } else {
        todo->removeCustomProperty("Zanshin", "Running");
    }

    // Needs to be done after all other dates are positioned
    // since this applies the recurrence logic
    if (task->isDone())
        todo->setCompleted(KDateTime(task->doneDate()));
    else
        todo->setCompleted(false);

    Akonadi::Item item;
    if (task->property("itemId").isValid()) {
        item.setId(task->property("itemId").value<Akonadi::Item::Id>());
    }
    if (task->property("parentCollectionId").isValid()) {
        auto parentId = task->property("parentCollectionId").value<Akonadi::Collection::Id>();
        item.setParentCollection(Akonadi::Collection(parentId));
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

void Serializer::updateItemProject(Item item, Domain::Project::Ptr project)
{
    if (isTaskItem(item)) {
        auto todo = item.payload<KCalCore::Todo::Ptr>();
        todo->setRelatedTo(project->property("todoUid").toString());

    } else if (isNoteItem(item)) {
        auto note = item.payload<KMime::Message::Ptr>();
        note->removeHeader("X-Zanshin-RelatedProjectUid");
        const QByteArray parentUid = project->property("todoUid").toString().toUtf8();
        if (!parentUid.isEmpty()) {
            auto relatedHeader = new KMime::Headers::Generic("X-Zanshin-RelatedProjectUid");
            relatedHeader->from7BitString(parentUid);
            note->appendHeader(relatedHeader);
        }
        note->assemble();
    }
}

void Serializer::removeItemParent(Akonadi::Item item)
{
    if (!isTaskItem(item))
        return;

    auto todo = item.payload<KCalCore::Todo::Ptr>();
    todo->setRelatedTo(QString());
}

void Serializer::promoteItemToProject(Akonadi::Item item)
{
    if (!isTaskItem(item))
        return;

    auto todo = item.payload<KCalCore::Todo::Ptr>();
    todo->setRelatedTo(QString());
    todo->setCustomProperty("Zanshin", "Project", QStringLiteral("1"));
}

void Serializer::clearItem(Akonadi::Item *item)
{
    Q_ASSERT(item);
    if (!isTaskItem(*item))
        return;

//    NOTE : Currently not working, when akonadistorage test will make it pass, we will use it
//    item->clearTags();

    foreach (const Tag& tag, item->tags())
        item->clearTag(tag);
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

    auto message = item.payload<KMime::Message::Ptr>();

    note->setTitle(message->subject(true)->asUnicodeString());
    note->setText(message->mainBodyPart()->decodedText());
    note->setProperty("itemId", item.id());

    if (auto relatedHeader = message->headerByType("X-Zanshin-RelatedProjectUid")) {
        note->setProperty("relatedUid", relatedHeader->asUnicodeString());
    } else {
        note->setProperty("relatedUid", QVariant());
    }
}

Item Serializer::createItemFromNote(Domain::Note::Ptr note)
{
    NoteUtils::NoteMessageWrapper builder;
    builder.setTitle(note->title());
    builder.setText(note->text() + '\n'); // Adding an extra '\n' because KMime always removes it...

    KMime::Message::Ptr message = builder.message();

    if (!note->property("relatedUid").toString().isEmpty()) {
        auto relatedHeader = new KMime::Headers::Generic("X-Zanshin-RelatedProjectUid");
        relatedHeader->from7BitString(note->property("relatedUid").toString().toUtf8());
        message->appendHeader(relatedHeader);
    }

    Akonadi::Item item;
    if (note->property("itemId").isValid()) {
        item.setId(note->property("itemId").value<Akonadi::Item::Id>());
    }
    item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    item.setPayload(message);
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
    project->setProperty("parentCollectionId", item.parentCollection().id());
    project->setProperty("todoUid", todo->uid());
}

Item Serializer::createItemFromProject(Domain::Project::Ptr project)
{
    auto todo = KCalCore::Todo::Ptr::create();

    todo->setSummary(project->name());
    todo->setCustomProperty("Zanshin", "Project", QStringLiteral("1"));

    if (project->property("todoUid").isValid()) {
        todo->setUid(project->property("todoUid").toString());
    }

    Akonadi::Item item;
    if (project->property("itemId").isValid()) {
        item.setId(project->property("itemId").value<Akonadi::Item::Id>());
    }
    if (project->property("parentCollectionId").isValid()) {
        auto parentId = project->property("parentCollectionId").value<Akonadi::Collection::Id>();
        item.setParentCollection(Akonadi::Collection(parentId));
    }
    item.setMimeType(KCalCore::Todo::todoMimeType());
    item.setPayload(todo);
    return item;
}

bool Serializer::isProjectChild(Domain::Project::Ptr project, Item item)
{
    const QString todoUid = project->property("todoUid").toString();
    const QString relatedUid = relatedUidFromItem(item);

    return !todoUid.isEmpty()
        && !relatedUid.isEmpty()
        && todoUid == relatedUid;
}

Domain::Context::Ptr Serializer::createContextFromTag(Akonadi::Tag tag)
{
    if (!isContext(tag))
        return Domain::Context::Ptr();

    auto context = Domain::Context::Ptr::create();
    updateContextFromTag(context, tag);
    return context;
}

Akonadi::Tag Serializer::createTagFromContext(Domain::Context::Ptr context)
{
    auto tag = Akonadi::Tag();
    tag.setName(context->name());
    tag.setType(Akonadi::SerializerInterface::contextTagType());
    tag.setGid(QByteArray(context->name().toLatin1()));

    if (context->property("tagId").isValid())
        tag.setId(context->property("tagId").value<Akonadi::Tag::Id>());

    return tag;
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
                       std::bind(Utils::mem_fn(&Serializer::isContext), this, _1));
}

bool Serializer::hasAkonadiTags(Item item) const
{
    using namespace std::placeholders;
    Tag::List tags = item.tags();
    return std::any_of(tags.constBegin(), tags.constEnd(),
                       std::bind(Utils::mem_fn(&Serializer::isAkonadiTag), this, _1));
}

bool Serializer::isContext(const Akonadi::Tag &tag) const
{
    return (tag.type() == Akonadi::SerializerInterface::contextTagType());
}

bool Serializer::isAkonadiTag(const Tag &tag) const
{
    return tag.type() == Akonadi::Tag::PLAIN;
}

bool Serializer::isContextTag(const Domain::Context::Ptr &context, const Akonadi::Tag &tag) const
{
    return (context->property("tagId").value<Akonadi::Tag::Id>() == tag.id());
}

bool Serializer::isContextChild(Domain::Context::Ptr context, Item item) const
{
    if (!context->property("tagId").isValid())
        return false;

    auto tagId = context->property("tagId").value<Akonadi::Tag::Id>();
    Akonadi::Tag tag(tagId);

    return item.hasTag(tag);
}

Domain::Tag::Ptr Serializer::createTagFromAkonadiTag(Akonadi::Tag akonadiTag)
{
    if (!isAkonadiTag(akonadiTag))
        return Domain::Tag::Ptr();

    auto tag = Domain::Tag::Ptr::create();
    updateTagFromAkonadiTag(tag, akonadiTag);
    return tag;
}

void Serializer::updateTagFromAkonadiTag(Domain::Tag::Ptr tag, Akonadi::Tag akonadiTag)
{
    if (!isAkonadiTag(akonadiTag))
        return;

    tag->setProperty("tagId", akonadiTag.id());
    tag->setName(akonadiTag.name());
}

Akonadi::Tag Serializer::createAkonadiTagFromTag(Domain::Tag::Ptr tag)
{
    auto akonadiTag = Akonadi::Tag();
    akonadiTag.setName(tag->name());
    akonadiTag.setType(Akonadi::Tag::PLAIN);
    akonadiTag.setGid(QByteArray(tag->name().toLatin1()));

    const auto tagProperty = tag->property("tagId");
    if (tagProperty.isValid())
        akonadiTag.setId(tagProperty.value<Akonadi::Tag::Id>());

    return akonadiTag;
}

bool Serializer::isTagChild(Domain::Tag::Ptr tag, Akonadi::Item item)
{
    if (!tag->property("tagId").isValid())
        return false;

    auto tagId = tag->property("tagId").value<Akonadi::Tag::Id>();
    Akonadi::Tag akonadiTag(tagId);

    return item.hasTag(akonadiTag);
}
