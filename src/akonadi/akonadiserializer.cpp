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
#include <KCalCore/Todo>
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

QString Serializer::itemUid(const Item &item)
{
    if (item.hasPayload<KCalCore::Todo::Ptr>()) {
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
    if (!isTaskCollection(collection))
        return false;

    if (!collection.hasAttribute<Akonadi::ApplicationSelectedAttribute>())
        return true;

    return collection.attribute<Akonadi::ApplicationSelectedAttribute>()->isSelected();
}

bool Akonadi::Serializer::isTaskCollection(Akonadi::Collection collection)
{
    return collection.contentMimeTypes().contains(KCalCore::Todo::todoMimeType());
}

bool Serializer::isTaskItem(Item item)
{
    if (!item.hasPayload<KCalCore::Todo::Ptr>())
        return false;

    return !isProjectItem(item) && !isContext(item);
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
    task->setDoneDate(todo->completed().toLocalTime().date());
    task->setStartDate(todo->dtStart().toLocalTime().date());
    task->setDueDate(todo->dtDue().toLocalTime().date());
    task->setProperty("itemId", item.id());
    task->setProperty("parentCollectionId", item.parentCollection().id());
    task->setProperty("todoUid", todo->uid());
    task->setProperty("relatedUid", todo->relatedTo());
    task->setRunning(todo->customProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsRunning()) == QLatin1String("1"));

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
    case KCalCore::Recurrence::rYearlyMonth:
        task->setRecurrence(Domain::Task::RecursYearly);
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
                           attachment.setUri(QUrl(attach->uri()));
                       else
                           attachment.setData(attach->decodedData());
                       attachment.setLabel(attach->label());
                       attachment.setMimeType(attach->mimeType());
                       attachment.setIconName(mimeDb.mimeTypeForName(attach->mimeType()).iconName());
                       return attachment;
                   });
    task->setAttachments(attachments);
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

    // We only support all-day todos, so ignore timezone information and possible effect from timezone on dates
    // KCalCore reads "DUE;VALUE=DATE:20171130" as QDateTime(QDate(2017, 11, 30), QTime(), Qt::LocalTime), for lack of timezone information
    // so we should never call toUtc() on that, it would mess up the date.
    // If one day we want to support time information, we need to add a task->isAllDay()/setAllDay().
    todo->setDtStart(QDateTime(task->startDate()));
    todo->setDtDue(QDateTime(task->dueDate()));
    todo->setAllDay(true);

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
    case Domain::Task::RecursYearly:
        todo->recurrence()->setYearly(1);
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

    if (task->isRunning()) {
        todo->setCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsRunning(), "1");
    } else {
        todo->removeCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsRunning());
    }

    // Needs to be done after all other dates are positioned
    // since this applies the recurrence logic
    if (task->isDone())
        todo->setCompleted(QDateTime(task->doneDate(), QTime(), Qt::UTC));
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
    todo->setCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsProject(), QStringLiteral("1"));
}

void Serializer::clearItem(Akonadi::Item *item)
{
    Q_ASSERT(item);
    if (!isTaskItem(*item))
        return;

    auto todo = item->payload<KCalCore::Todo::Ptr>();
    todo->removeCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyContextList());
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
    itemsRemoved.reserve(std::distance(itemsToProcess.begin(), bound));
    std::copy(itemsToProcess.begin(), bound, std::back_inserter(itemsRemoved));
    itemsToProcess.erase(itemsToProcess.begin(), bound);

    auto result = std::accumulate(itemsToProcess.begin(), itemsToProcess.end(), Akonadi::Item::List(),
                                 [this, itemsRemoved](Akonadi::Item::List result, Akonadi::Item currentItem) {
                                     result << currentItem;
                                     return result += filterDescendantItems(itemsRemoved, currentItem);
                                 });

    return result;
}

bool Serializer::isProjectItem(Item item)
{
    if (!item.hasPayload<KCalCore::Todo::Ptr>())
        return false;

    auto todo = item.payload<KCalCore::Todo::Ptr>();
    return !todo->customProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsProject()).isEmpty();
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
    todo->setCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsProject(), QStringLiteral("1"));

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

static QStringList extractContexts(KCalCore::Todo::Ptr todo)
{
    const QString contexts = todo->customProperty(Serializer::customPropertyAppName(), Serializer::customPropertyContextList());
    return contexts.split(',', QString::SkipEmptyParts);
}

bool Serializer::isContextChild(Domain::Context::Ptr context, Item item) const
{
    if (!context->property("todoUid").isValid())
        return false;

    if (!item.hasPayload<KCalCore::Todo::Ptr>())
        return false;

    auto contextUid = context->property("todoUid").toString();
    auto todo = item.payload<KCalCore::Todo::Ptr>();
    const auto contextList = extractContexts(todo);
    return contextList.contains(contextUid);
}

bool Serializer::isContext(Item item)
{
    if (!item.hasPayload<KCalCore::Todo::Ptr>())
        return false;

    auto todo = item.payload<KCalCore::Todo::Ptr>();
    return !todo->customProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsContext()).isEmpty();
}

Domain::Context::Ptr Serializer::createContextFromItem(Item item)
{
    if (!isContext(item))
        return {};

    auto context = Domain::Context::Ptr::create();
    updateContextFromItem(context, item);
    return context;
}

Akonadi::Item Serializer::createItemFromContext(Domain::Context::Ptr context)
{
    auto todo = KCalCore::Todo::Ptr::create();

    todo->setSummary(context->name());
    todo->setCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsContext(), QStringLiteral("1"));

    if (context->property("todoUid").isValid()) {
        todo->setUid(context->property("todoUid").toString());
    }

    auto item = Akonadi::Item();
    if (context->property("itemId").isValid()) {
        item.setId(context->property("itemId").value<Akonadi::Item::Id>());
    }
    if (context->property("parentCollectionId").isValid()) {
        auto parentId = context->property("parentCollectionId").value<Akonadi::Collection::Id>();
        item.setParentCollection(Akonadi::Collection(parentId));
    }
    item.setMimeType(KCalCore::Todo::todoMimeType());
    item.setPayload(todo);
    return item;
}

void Serializer::updateContextFromItem(Domain::Context::Ptr context, Item item)
{
    if (!isContext(item))
        return;

    auto todo = item.payload<KCalCore::Todo::Ptr>();

    context->setName(todo->summary());
    context->setProperty("itemId", item.id());
    context->setProperty("parentCollectionId", item.parentCollection().id());
    context->setProperty("todoUid", todo->uid());
}

void Serializer::addContextToTask(Domain::Context::Ptr context, Item item)
{
    if (!isTaskItem(item)) {
        qWarning() << "Cannot add context to a non-task" << item.id();
        return;
    }

    auto todo = item.payload<KCalCore::Todo::Ptr>();

    if (!context->property("todoUid").isValid())
        return;

    auto contextUid = context->property("todoUid").toString();
    auto contextList = extractContexts(todo);
    if (!contextList.contains(contextUid))
        contextList.append(contextUid);
    todo->setCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyContextList(), contextList.join(','));

    item.setPayload<KCalCore::Todo::Ptr>(todo);
}

void Serializer::removeContextFromTask(Domain::Context::Ptr context, Item item)
{
    if (!isTaskItem(item)) {
        qWarning() << "Cannot remove context from a non-task" << item.id();
        return;
    }

    auto todo = item.payload<KCalCore::Todo::Ptr>();

    if (!context->property("todoUid").isValid())
        return;

    auto contextUid = context->property("todoUid").toString();
    QStringList contextList = extractContexts(todo);
    contextList.removeAll(contextUid);
    if (contextList.isEmpty())
        todo->removeCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyContextList());
    else
        todo->setCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyContextList(), contextList.join(','));

    item.setPayload<KCalCore::Todo::Ptr>(todo);
}

QString Serializer::contextUid(Item item)
{
    if (!isContext(item))
        return {};

    auto todo = item.payload<KCalCore::Todo::Ptr>();
    return todo->uid();
}

// KCalCore's CustomProperties doesn't implement case insensitivity
// and some CALDAV servers make everything uppercase. So do like most of kdepim
// and use uppercase property names.

QByteArray Serializer::customPropertyAppName()
{
    return QByteArrayLiteral("ZANSHIN");
}

QByteArray Serializer::customPropertyIsProject()
{
    return QByteArrayLiteral("ISPROJECT");
}

QByteArray Serializer::customPropertyIsContext()
{
    return QByteArrayLiteral("ISCONTEXT");
}

QByteArray Serializer::customPropertyIsRunning()
{
    return QByteArrayLiteral("ISRUNNING");
}

QByteArray Serializer::customPropertyContextList()
{
    return QByteArrayLiteral("CONTEXTLIST");
}
