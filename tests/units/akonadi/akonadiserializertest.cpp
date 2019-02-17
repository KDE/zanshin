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

#include <testlib/qtest_zanshin.h>

#include "akonadi/akonadiserializer.h"
#include "akonadi/akonadiapplicationselectedattribute.h"
#include "akonadi/akonaditimestampattribute.h"

#include <AkonadiCore/Collection>
#include <AkonadiCore/EntityDisplayAttribute>
#include <AkonadiCore/Item>
#include <KCalCore/Todo>

Q_DECLARE_METATYPE(Akonadi::Item*)

static void setTodoDates(KCalCore::Todo::Ptr todo, const QDate &start, const QDate &due) {
    todo->setDtStart(QDateTime(start));
    todo->setDtDue(QDateTime(due));
}

static const char s_contextListProperty[] = "ContextList";
static const char s_appName[] = "Zanshin";

class AkonadiSerializerTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldKnowWhenAnObjectRepresentsACollection()
    {
        // GIVEN
        Akonadi::Serializer serializer;
        auto object = Akonadi::Serializer::QObjectPtr::create();
        Akonadi::Collection collection(42);

        // WHEN
        // Nothing yet

        // THEN
        QVERIFY(!serializer.representsCollection(object, collection));

        // WHEN
        object->setProperty("collectionId", 42);

        // THEN
        QVERIFY(serializer.representsCollection(object, collection));

        // WHEN
        object->setProperty("collectionId", 43);

        // THEN
        QVERIFY(!serializer.representsCollection(object, collection));
    }

    void shouldKnowWhenAnObjectRepresentsAnItem()
    {
        // GIVEN
        Akonadi::Serializer serializer;
        auto object = Akonadi::Serializer::QObjectPtr::create();
        Akonadi::Item item(42);

        // WHEN
        // Nothing yet

        // THEN
        QVERIFY(!serializer.representsItem(object, item));

        // WHEN
        object->setProperty("itemId", 42);

        // THEN
        QVERIFY(serializer.representsItem(object, item));

        // WHEN
        object->setProperty("itemId", 43);

        // THEN
        QVERIFY(!serializer.representsItem(object, item));
    }

    void shouldKnowTaskItemUid_data()
    {
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<QString>("expectedUid");

        Akonadi::Item item1;
        KCalCore::Todo::Ptr todo1(new KCalCore::Todo);
        todo1->setUid(QString());
        item1.setPayload<KCalCore::Todo::Ptr>(todo1);

        Akonadi::Item item2;
        KCalCore::Todo::Ptr todo2(new KCalCore::Todo);
        todo2->setUid(QStringLiteral("1"));
        item2.setPayload<KCalCore::Todo::Ptr>(todo2);

        QTest::newRow("task without uid") << item1 << QString();
        QTest::newRow("task with uid") << item2 << "1";
    }

    void shouldKnowTaskItemUid()
    {
        // GIVEN
        QFETCH(Akonadi::Item, item);
        QFETCH(QString, expectedUid);

        // WHEN
        Akonadi::Serializer serializer;
        QString uid = serializer.itemUid(item);

        // THEN
        QCOMPARE(uid, expectedUid);
    }

    void shouldCreateDataSourceFromCollection_data()
    {
        QTest::addColumn<QString>("name");
        QTest::addColumn<QString>("iconName");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<bool>("hasSelectedAttribute");
        QTest::addColumn<bool>("isSelected");

        const auto noteMimeTypes = QStringList() << QStringLiteral("text/x-vnd.akonadi.note");
        const auto taskMimeTypes = QStringList() << QStringLiteral("application/x-vnd.akonadi.calendar.todo");
        const auto bogusMimeTypes = QStringList() << QStringLiteral("foo/bar");
        const auto allMimeTypes = noteMimeTypes + taskMimeTypes + bogusMimeTypes;

        QTest::newRow("nominal case") << "name" << "icon" << allMimeTypes << true << false;

        QTest::newRow("only notes") << "name" << "icon" << noteMimeTypes << true << false;
        QTest::newRow("only tasks") << "name" << "icon" << taskMimeTypes << true << false;
        QTest::newRow("only bogus") << "name" << "icon" << bogusMimeTypes << true << false;

        QTest::newRow("no selected attribute") << "name" << "icon" << allMimeTypes << false << false;
        QTest::newRow("selected attribute (false)") << "name" << "icon" << allMimeTypes << true << false;
        QTest::newRow("selected attribute (true)") << "name" << "icon" << allMimeTypes << true << true;

        QTest::newRow("empty case") << QString() << QString() << QStringList() << false << false;
    }

    void shouldCreateDataSourceFromCollection()
    {
        // GIVEN

        // Data...
        QFETCH(QString, name);
        QFETCH(QString, iconName);
        QFETCH(QStringList, mimeTypes);
        QFETCH(bool, hasSelectedAttribute);
        QFETCH(bool, isSelected);

        Domain::DataSource::ContentTypes expectedContentTypes;
        if (mimeTypes.contains(QStringLiteral("application/x-vnd.akonadi.calendar.todo"))) {
            expectedContentTypes |= Domain::DataSource::Tasks;
        }

        // ... stored in a collection
        Akonadi::Collection collection(42);
        collection.setContentMimeTypes(mimeTypes);
        collection.setName(name);
        auto displayAttribute = new Akonadi::EntityDisplayAttribute;
        displayAttribute->setIconName(iconName);
        collection.addAttribute(displayAttribute);
        if (hasSelectedAttribute) {
            auto selectedAttribute = new Akonadi::ApplicationSelectedAttribute;
            selectedAttribute->setSelected(isSelected);
            collection.addAttribute(selectedAttribute);
        }

        // WHEN
        Akonadi::Serializer serializer;
        auto dataSource = serializer.createDataSourceFromCollection(collection, Akonadi::SerializerInterface::BaseName);

        // THEN
        QCOMPARE(dataSource->name(), name);
        QCOMPARE(dataSource->iconName(), iconName);
        QCOMPARE(dataSource->contentTypes(), expectedContentTypes);
        QCOMPARE(dataSource->isSelected(), !hasSelectedAttribute || isSelected);
        QCOMPARE(dataSource->property("collectionId").value<Akonadi::Collection::Id>(), collection.id());
    }

    void shouldCreateNullDataSourceFromInvalidCollection()
    {
        // GIVEN
        Akonadi::Collection collection;

        // WHEN
        Akonadi::Serializer serializer;
        auto dataSource = serializer.createDataSourceFromCollection(collection, Akonadi::SerializerInterface::BaseName);

        // THEN
        QVERIFY(dataSource.isNull());
    }

    void shouldUpdateDataSourceFromCollection_data()
    {
        QTest::addColumn<QString>("updatedName");

        QTest::newRow("no change") << "name";
        QTest::newRow("changed") << "new name";
    }

    void shouldUpdateDataSourceFromCollection()
    {
        // GIVEN

        // A collection...
        Akonadi::Collection originalCollection(42);
        originalCollection.setName(QStringLiteral("name"));

        // ... deserialized as a data source
        Akonadi::Serializer serializer;
        auto dataSource = serializer.createDataSourceFromCollection(originalCollection, Akonadi::SerializerInterface::BaseName);

        // WHEN

        // Data...
        QFETCH(QString, updatedName);

        // ... in a new collection
        Akonadi::Collection updatedCollection(42);
        updatedCollection.setName(updatedName);

        serializer.updateDataSourceFromCollection(dataSource, updatedCollection, Akonadi::SerializerInterface::BaseName);

        // THEN
        QCOMPARE(dataSource->name(), updatedName);
    }

    void shouldNotUpdateDataSourceFromInvalidCollection()
    {
        // GIVEN

        // Data...
        const QString name = QStringLiteral("name");

        // ... stored in a collection...
        Akonadi::Collection originalCollection(42);
        originalCollection.setName(name);

        // ... deserialized as a data source
        Akonadi::Serializer serializer;
        auto dataSource = serializer.createDataSourceFromCollection(originalCollection, Akonadi::SerializerInterface::BaseName);

        // WHEN
        Akonadi::Collection invalidCollection;
        invalidCollection.setName(QStringLiteral("foo"));
        serializer.updateDataSourceFromCollection(dataSource, invalidCollection, Akonadi::SerializerInterface::BaseName);

        // THEN
        QCOMPARE(dataSource->name(), name);
    }

    void shouldNameDataSourceFromCollectionPathIfRequested()
    {
        // GIVEN

        // Data...
        const QString name = QStringLiteral("name");
        const QString parentName = QStringLiteral("parent");

        // ... stored in a collection with a parent
        Akonadi::Collection collection(42);
        collection.setName(name);
        Akonadi::Collection parentCollection(41);
        parentCollection.setName(QStringLiteral("Foo"));
        auto attribute = new Akonadi::EntityDisplayAttribute;
        attribute->setDisplayName(parentName);
        parentCollection.addAttribute(attribute);
        collection.setParentCollection(parentCollection);

        // WHEN
        Akonadi::Serializer serializer;
        auto dataSource1 = serializer.createDataSourceFromCollection(collection, Akonadi::SerializerInterface::FullPath);
        auto dataSource2 = serializer.createDataSourceFromCollection(collection, Akonadi::SerializerInterface::BaseName);

        // Give it another try with the root
        parentCollection.setParentCollection(Akonadi::Collection::root());
        collection.setParentCollection(parentCollection);
        auto dataSource3 = serializer.createDataSourceFromCollection(collection, Akonadi::SerializerInterface::FullPath);
        auto dataSource4 = serializer.createDataSourceFromCollection(collection, Akonadi::SerializerInterface::BaseName);

        // THEN
        QCOMPARE(dataSource1->name(), QString(parentName + " » " + name));
        QCOMPARE(dataSource2->name(), name);
        QCOMPARE(dataSource3->name(), QString(parentName + " » " + name));
        QCOMPARE(dataSource4->name(), name);
    }

    void shouldCreateCollectionFromDataSource_data()
    {
        QTest::addColumn<QString>("name");
        QTest::addColumn<QString>("iconName");
        QTest::addColumn<Domain::DataSource::ContentTypes>("contentTypes");
        QTest::addColumn<bool>("isSelected");

        const auto noType = Domain::DataSource::ContentTypes(Domain::DataSource::NoContent);
        const auto taskType = Domain::DataSource::ContentTypes(Domain::DataSource::Tasks);

        QTest::newRow("only tasks") << "name" << "icon-name" << taskType << true;
        QTest::newRow("only nothing ;)") << "name" << "icon-name" << noType << true;

        QTest::newRow("not selected") << "name" << "icon-name" << taskType << false;
        QTest::newRow("selected") << "name" << "icon-name" << taskType << true;

        QTest::newRow("empty case") << QString() << QString() << noType << true;
    }

    void shouldCreateCollectionFromDataSource()
    {
        // GIVEN
        const auto timestamp = QDateTime::currentMSecsSinceEpoch();

        // Data...
        QFETCH(QString, name);
        QFETCH(QString, iconName);
        QFETCH(Domain::DataSource::ContentTypes, contentTypes);
        QFETCH(bool, isSelected);

        QStringList mimeTypes;
        if (contentTypes & Domain::DataSource::Tasks)
            mimeTypes << QStringLiteral("application/x-vnd.akonadi.calendar.todo");


        // ... stored in a data source
        auto source = Domain::DataSource::Ptr::create();
        source->setName(name);
        source->setIconName(iconName);
        source->setContentTypes(contentTypes);
        source->setSelected(isSelected);
        source->setProperty("collectionId", 42);

        // WHEN
        Akonadi::Serializer serializer;
        auto collection = serializer.createCollectionFromDataSource(source);

        // THEN
        QCOMPARE(collection.id(), source->property("collectionId").value<Akonadi::Collection::Id>());
        QVERIFY(collection.hasAttribute<Akonadi::ApplicationSelectedAttribute>());
        QCOMPARE(collection.attribute<Akonadi::ApplicationSelectedAttribute>()->isSelected(), isSelected);
        QVERIFY(collection.hasAttribute<Akonadi::TimestampAttribute>());
        QVERIFY(collection.attribute<Akonadi::TimestampAttribute>()->timestamp() >= timestamp);
    }

    void shouldVerifyIfCollectionIsSelected_data()
    {
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<bool>("hasSelectedAttribute");
        QTest::addColumn<bool>("isSelected");
        QTest::addColumn<bool>("expectedSelected");

        const auto taskMimeTypes = QStringList() << QStringLiteral("application/x-vnd.akonadi.calendar.todo");
        const auto bogusMimeTypes = QStringList() << QStringLiteral("foo/bar");
        const auto allMimeTypes = taskMimeTypes + bogusMimeTypes;

        QTest::newRow("nominal case") << allMimeTypes << true << false << false;

        QTest::newRow("only tasks") << taskMimeTypes << true << false << false;
        QTest::newRow("only bogus") << bogusMimeTypes << true << false << false;

        QTest::newRow("selected, only tasks") << taskMimeTypes << true << true << true;
        QTest::newRow("selected, only bogus") << bogusMimeTypes << true << true << false;

        QTest::newRow("no selected attribute") << allMimeTypes << false << false << true;
        QTest::newRow("selected attribute (false)") << allMimeTypes << true << false << false;
        QTest::newRow("selected attribute (true)") << allMimeTypes << true << true << true;

        QTest::newRow("empty case") << QStringList() << false << false << false;
    }

    void shouldVerifyIfCollectionIsSelected()
    {
        // GIVEN
        QFETCH(QStringList, mimeTypes);
        QFETCH(bool, hasSelectedAttribute);
        QFETCH(bool, isSelected);

        Domain::DataSource::ContentTypes expectedContentTypes;
        if (mimeTypes.contains(QStringLiteral("application/x-vnd.akonadi.calendar.todo"))) {
            expectedContentTypes |= Domain::DataSource::Tasks;
        }

        // ... stored in a collection
        Akonadi::Collection collection(42);
        collection.setContentMimeTypes(mimeTypes);
        if (hasSelectedAttribute) {
            auto selectedAttribute = new Akonadi::ApplicationSelectedAttribute;
            selectedAttribute->setSelected(isSelected);
            collection.addAttribute(selectedAttribute);
        }

        // WHEN
        Akonadi::Serializer serializer;

        // THEN
        QFETCH(bool, expectedSelected);
        QCOMPARE(serializer.isSelectedCollection(collection), expectedSelected);
    }

    void shouldVerifyCollectionContents_data()
    {
        QTest::addColumn<QString>("mimeType");
        QTest::addColumn<bool>("expectedTasks");

        QTest::newRow("task collection") << "application/x-vnd.akonadi.calendar.todo" << true;
        QTest::newRow("note collection") << "text/x-vnd.akonadi.note" << false;
    }

    void shouldVerifyCollectionContents()
    {
        // GIVEN

        // Data...
        QFETCH(QString, mimeType);

        // ... stored in a collection
        Akonadi::Collection collection(42);
        collection.setContentMimeTypes(QStringList() << mimeType);

        // WHEN
        Akonadi::Serializer serializer;
        QFETCH(bool, expectedTasks);

        // THEN
        QCOMPARE(serializer.isTaskCollection(collection), expectedTasks);
    }

    void shouldCreateTaskFromItem_data()
    {
        QTest::addColumn<QString>("summary");
        QTest::addColumn<QString>("content");
        QTest::addColumn<bool>("isDone");
        QTest::addColumn<QDate>("doneDate");
        QTest::addColumn<QDate>("startDate");
        QTest::addColumn<QDate>("dueDate");

        QTest::newRow("nominal case") << "summary" << "content" << false << QDate() << QDate(2013, 11, 24) << QDate(2014, 03, 01);
        QTest::newRow("done case") << "summary" << "content" << true << QDate(2013, 11, 30) << QDate(2013, 11, 24) << QDate(2014, 03, 01);
        QTest::newRow("done without doneDate case") << "summary" << "content" << true << QDate() << QDate(2013, 11, 24) << QDate(2014, 03, 01);
        QTest::newRow("empty case") << QString() << QString() << false << QDate() << QDate() << QDate();
    }

    void shouldCreateTaskFromItem()
    {
        // GIVEN

        // Data...
        QFETCH(QString, summary);
        QFETCH(QString, content);
        QFETCH(bool, isDone);
        QFETCH(QDate, doneDate);
        QFETCH(QDate, startDate);
        QFETCH(QDate, dueDate);

        // ... stored in a todo...
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setSummary(summary);
        todo->setDescription(content);

        if (isDone)
            todo->setCompleted(QDateTime(doneDate));
        else
            todo->setCompleted(isDone);

        setTodoDates(todo, startDate, dueDate);
        todo->setRelatedTo(QStringLiteral("my-uid"));


        // ... as payload of an item
        Akonadi::Item item;
        item.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        item.setPayload<KCalCore::Todo::Ptr>(todo);

        // which has a parent collection
        Akonadi::Collection collection(43);
        item.setParentCollection(collection);

        // WHEN
        Akonadi::Serializer serializer;
        auto task = serializer.createTaskFromItem(item);

        // THEN
        QVERIFY(!task.isNull());
        QCOMPARE(task->title(), summary);
        QCOMPARE(task->text(), content);
        QCOMPARE(task->isDone(), isDone);
        QCOMPARE(task->doneDate(), doneDate);
        QCOMPARE(task->startDate(), startDate);
        QCOMPARE(task->dueDate(), dueDate);
        QCOMPARE(task->property("todoUid").toString(), todo->uid());
        QCOMPARE(task->property("relatedUid").toString(), todo->relatedTo());
        QCOMPARE(task->property("itemId").toLongLong(), item.id());
        QCOMPARE(task->property("parentCollectionId").toLongLong(), collection.id());
    }

    void shouldCreateNullTaskFromInvalidItem()
    {
        // GIVEN
        Akonadi::Item item;

        // WHEN
        Akonadi::Serializer serializer;
        auto task = serializer.createTaskFromItem(item);

        // THEN
        QVERIFY(task.isNull());
    }

    void shouldCreateNullTaskFromProjectItem()
    {
        // GIVEN

        // A todo with the project flag
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setSummary(QStringLiteral("foo"));
        todo->setCustomProperty("Zanshin", "Project", QStringLiteral("1"));

        // ... as payload of an item
        Akonadi::Item item;
        item.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        item.setPayload<KCalCore::Todo::Ptr>(todo);

        // WHEN
        Akonadi::Serializer serializer;
        auto task = serializer.createTaskFromItem(item);

        // THEN
        QVERIFY(task.isNull());
    }

    void shouldUpdateTaskFromItem_data()
    {
        QTest::addColumn<QString>("updatedSummary");
        QTest::addColumn<QString>("updatedContent");
        QTest::addColumn<bool>("updatedDone");
        QTest::addColumn<QDate>("updatedDoneDate");
        QTest::addColumn<QDate>("updatedStartDate");
        QTest::addColumn<QDate>("updatedDueDate");
        QTest::addColumn<QString>("updatedRelated");
        QTest::addColumn<bool>("updatedRecurs");
        QTest::addColumn<QByteArrayList>("updatedAttachmentData");
        QTest::addColumn<QStringList>("updatedAttachmentUris");
        QTest::addColumn<QStringList>("updatedAttachmentLabels");
        QTest::addColumn<QStringList>("updatedAttachmentMimeTypes");
        QTest::addColumn<QStringList>("updatedAttachmentIconNames");
        QTest::addColumn<bool>("updatedRunning");

        QTest::newRow("no change") << "summary" << "content" << false << QDate() <<  QDate(2013, 11, 24) << QDate(2014, 03, 01) << "my-uid" << false << QByteArrayList() << QStringList() << QStringList() << QStringList() << QStringList() << false;
        QTest::newRow("changed") << "new summary" << "new content" << true << QDate(2013, 11, 28) << QDate(2013, 11, 25) << QDate(2014, 03, 02) << "my-new-uid" << true << QByteArrayList({"foo", "# bar", QByteArray()}) << QStringList({QString(), QString(), "https://www.kde.org"}) << QStringList({"label1", "label2", "label3"}) << QStringList({"text/plain", "text/markdown", "text/html"}) << QStringList({"text-plain", "text-markdown", "text-html"}) << false;
        QTest::newRow("set_to_running") << "summary" << "content" << false << QDate() <<  QDate(2013, 11, 24) << QDate(2014, 03, 01) << "my-uid" << false << QByteArrayList() << QStringList() << QStringList() << QStringList() << QStringList() << true;
    }

    void shouldUpdateTaskFromItem()
    {
        // GIVEN

        // A todo...
        KCalCore::Todo::Ptr originalTodo(new KCalCore::Todo);
        originalTodo->setSummary(QStringLiteral("summary"));
        originalTodo->setDescription(QStringLiteral("content"));
        originalTodo->setCompleted(false);
        setTodoDates(originalTodo, QDate(2013, 11, 24), QDate(2014, 03, 01));

        originalTodo->setRelatedTo(QStringLiteral("my-uid"));
        KCalCore::Attendee::Ptr originalAttendee(new KCalCore::Attendee(QStringLiteral("John Doe"),
                                                                        QStringLiteral("j@d.com"),
                                                                        true,
                                                                        KCalCore::Attendee::Accepted));
        originalTodo->addAttendee(originalAttendee);

        // ... as payload of an item...
        Akonadi::Item originalItem;
        originalItem.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        originalItem.setPayload<KCalCore::Todo::Ptr>(originalTodo);

        // ... which has a parent collection...
        Akonadi::Collection originalCollection(43);
        originalItem.setParentCollection(originalCollection);

        // ... deserialized as a task
        Akonadi::Serializer serializer;
        auto task = serializer.createTaskFromItem(originalItem);

        // WHEN

        // Data...
        QFETCH(QString, updatedSummary);
        QFETCH(QString, updatedContent);
        QFETCH(bool, updatedDone);
        QFETCH(QDate, updatedDoneDate);
        QFETCH(QDate, updatedStartDate);
        QFETCH(QDate, updatedDueDate);
        QFETCH(QString, updatedRelated);
        QFETCH(bool, updatedRecurs);
        QFETCH(QByteArrayList, updatedAttachmentData);
        QFETCH(QStringList, updatedAttachmentUris);
        QFETCH(QStringList, updatedAttachmentLabels);
        QFETCH(QStringList, updatedAttachmentMimeTypes);
        QFETCH(QStringList, updatedAttachmentIconNames);
        QFETCH(bool, updatedRunning);

        // ... in a new todo...
        KCalCore::Todo::Ptr updatedTodo(new KCalCore::Todo);
        updatedTodo->setSummary(updatedSummary);
        updatedTodo->setDescription(updatedContent);

        if (updatedDone)
            updatedTodo->setCompleted(QDateTime(updatedDoneDate));
        else
            updatedTodo->setCompleted(updatedDone);

        setTodoDates(updatedTodo, updatedStartDate, updatedDueDate);
        updatedTodo->setRelatedTo(updatedRelated);

        if (updatedRecurs)
            updatedTodo->recurrence()->setDaily(1);

        for (int i = 0; i < updatedAttachmentData.size(); i++) {
            KCalCore::Attachment::Ptr attachment(new KCalCore::Attachment(QByteArray()));
            if (!updatedAttachmentData.at(i).isEmpty())
                attachment->setDecodedData(updatedAttachmentData.at(i));
            else
                attachment->setUri(updatedAttachmentUris.at(i));
            attachment->setMimeType(updatedAttachmentMimeTypes.at(i));
            attachment->setLabel(updatedAttachmentLabels.at(i));
            updatedTodo->addAttachment(attachment);
        }

        if (updatedRunning) {
            updatedTodo->setCustomProperty("Zanshin", "Running", "1");
        } else {
            updatedTodo->removeCustomProperty("Zanshin", "Running");
        }

        // ... as payload of a new item
        Akonadi::Item updatedItem;
        updatedItem.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        updatedItem.setPayload<KCalCore::Todo::Ptr>(updatedTodo);

        // ... which has a new parent collection
        Akonadi::Collection updatedCollection(45);
        updatedItem.setParentCollection(updatedCollection);

        serializer.updateTaskFromItem(task, updatedItem);

        // THEN
        QCOMPARE(task->title(), updatedSummary);
        QCOMPARE(task->text(), updatedContent);
        QCOMPARE(task->isDone(), updatedDone);
        QCOMPARE(task->doneDate(), updatedDoneDate);
        QCOMPARE(task->startDate(), updatedStartDate);
        QCOMPARE(task->dueDate(), updatedDueDate);
        QCOMPARE(task->property("todoUid").toString(), updatedTodo->uid());
        QCOMPARE(task->property("relatedUid").toString(), updatedTodo->relatedTo());
        QCOMPARE(task->property("itemId").toLongLong(), updatedItem.id());
        QCOMPARE(task->property("parentCollectionId").toLongLong(), updatedCollection.id());
        QCOMPARE(task->recurrence(), (updatedRecurs ? Domain::Task::RecursDaily : Domain::Task::NoRecurrence));
        QCOMPARE(task->attachments().size(), updatedAttachmentData.size());
        for (int i = 0; i < task->attachments().size(); i++) {
            const auto attachment = task->attachments().at(i);
            QCOMPARE(attachment.data(), updatedAttachmentData.at(i));
            QCOMPARE(attachment.uri(), QUrl(updatedAttachmentUris.at(i)));
            QCOMPARE(attachment.label(), updatedAttachmentLabels.at(i));
            QCOMPARE(attachment.mimeType(), updatedAttachmentMimeTypes.at(i));
            QCOMPARE(attachment.iconName(), updatedAttachmentIconNames.at(i));
        }
        QCOMPARE(task->isRunning(), updatedRunning);
    }

    void shouldUpdateTaskRecurrenceFromItem_data()
    {
        QTest::addColumn<int>("todoRecurrence");
        QTest::addColumn<Domain::Task::Recurrence>("expectedRecurrence");

        QTest::newRow("none") << int(KCalCore::Recurrence::rNone) << Domain::Task::NoRecurrence;
        QTest::newRow("minutely") << int(KCalCore::Recurrence::rMinutely) << Domain::Task::NoRecurrence;
        QTest::newRow("hourly") << int(KCalCore::Recurrence::rHourly) << Domain::Task::NoRecurrence;
        QTest::newRow("daily") << int(KCalCore::Recurrence::rDaily) << Domain::Task::RecursDaily;
        QTest::newRow("weekly") << int(KCalCore::Recurrence::rWeekly) << Domain::Task::RecursWeekly;
        QTest::newRow("monthly") << int(KCalCore::Recurrence::rMonthlyDay) << Domain::Task::RecursMonthly;
    }

    void shouldUpdateTaskRecurrenceFromItem()
    {
        // GIVEN

        // A todo...
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setSummary(QStringLiteral("summary"));

        QFETCH(int, todoRecurrence);
        switch (todoRecurrence) {
        case KCalCore::Recurrence::rNone:
            break;
        case KCalCore::Recurrence::rMinutely:
            todo->recurrence()->setMinutely(1);
            break;
        case KCalCore::Recurrence::rHourly:
            todo->recurrence()->setHourly(1);
            break;
        case KCalCore::Recurrence::rDaily:
            todo->recurrence()->setDaily(1);
            break;
        case KCalCore::Recurrence::rWeekly:
            todo->recurrence()->setWeekly(1);
            break;
        case KCalCore::Recurrence::rMonthlyDay:
            todo->recurrence()->setMonthly(1);
            break;
        default:
            qFatal("Shouldn't happen");
        }

        // ... as payload of an item...
        Akonadi::Item item;
        item.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        item.setPayload<KCalCore::Todo::Ptr>(todo);

        // ... which has a parent collection...
        Akonadi::Collection collection(43);
        item.setParentCollection(collection);

        // WHEN
        // ... deserialized as a task
        Akonadi::Serializer serializer;
        auto task = serializer.createTaskFromItem(item);

        // THEN
        QFETCH(Domain::Task::Recurrence, expectedRecurrence);
        QCOMPARE(task->recurrence(), expectedRecurrence);
    }

    void shouldNotBreakRecurrenceDuringSerialization()
    {
        // GIVEN

        // Data...
        const QDate today(QDate::currentDate());
        const QDate doneDate(2013, 11, 20);
        const QDate startDate(2013, 11, 10);

        // ... stored in a todo...
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setSummary(QStringLiteral("summary"));
        todo->setDtStart(QDateTime(startDate));
        todo->recurrence()->setMonthly(1);

        // ... as payload of an item...
        Akonadi::Item item;
        item.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        item.setPayload<KCalCore::Todo::Ptr>(todo);

        // ... deserialized as a task
        Akonadi::Serializer serializer;
        auto task = serializer.createTaskFromItem(item);

        // WHEN
        // Task is marked done...
        task->setDoneDate(doneDate);
        task->setDone(true);

        // and goes through serialization and back
        const auto newItem = serializer.createItemFromTask(task);
        serializer.updateTaskFromItem(task, newItem);

        // THEN
        QCOMPARE(task->recurrence(), Domain::Task::RecursMonthly);
        QVERIFY(!task->isDone());
        const QDate lastOccurrence(QDate(today.year(), today.month(), 10));
        if (today.day() >= 10)
            QCOMPARE(task->startDate(), lastOccurrence.addMonths(1));
        else
            QCOMPARE(task->startDate(), lastOccurrence);
    }

    void shouldNotUpdateTaskFromInvalidItem()
    {
        // GIVEN

        // Data...
        const QString summary = QStringLiteral("summary");
        const QString content = QStringLiteral("content");
        const bool isDone = true;
        const QDate doneDate(2013, 11, 30);
        const QDate startDate(2013, 11, 24);
        const QDate dueDate(2014, 03, 01);

        // ... stored in a todo...
        KCalCore::Todo::Ptr originalTodo(new KCalCore::Todo);
        originalTodo->setSummary(summary);
        originalTodo->setDescription(content);

        if (originalTodo)
            originalTodo->setCompleted(QDateTime(doneDate));
        else
            originalTodo->setCompleted(isDone);
        setTodoDates(originalTodo, startDate, dueDate);

        // ... as payload of an item...
        Akonadi::Item originalItem;
        originalItem.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        originalItem.setPayload<KCalCore::Todo::Ptr>(originalTodo);

        // ... deserialized as a task
        Akonadi::Serializer serializer;
        auto task = serializer.createTaskFromItem(originalItem);

        // WHEN
        Akonadi::Item invalidItem;
        serializer.updateTaskFromItem(task, invalidItem);

        // THEN
        QCOMPARE(task->title(), summary);
        QCOMPARE(task->text(), content);
        QCOMPARE(task->isDone(), isDone);
        QCOMPARE(task->doneDate(), doneDate);
        QCOMPARE(task->startDate(), startDate);
        QCOMPARE(task->dueDate(), dueDate);
        QCOMPARE(task->property("itemId").toLongLong(), originalItem.id());
    }

    void shouldNotUpdateTaskFromProjectItem()
    {
        // GIVEN

        // Data...
        const QString summary = QStringLiteral("summary");
        const QString content = QStringLiteral("content");
        const bool isDone = true;
        const QDate doneDate(2013, 11, 30);
        const QDate startDate(2013, 11, 24);
        const QDate dueDate(2014, 03, 01);

        // ... stored in a todo...
        KCalCore::Todo::Ptr originalTodo(new KCalCore::Todo);
        originalTodo->setSummary(summary);
        originalTodo->setDescription(content);

        if (originalTodo)
            originalTodo->setCompleted(QDateTime(doneDate));
        else
            originalTodo->setCompleted(isDone);
        setTodoDates(originalTodo, startDate, dueDate);

        // ... as payload of an item...
        Akonadi::Item originalItem;
        originalItem.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        originalItem.setPayload<KCalCore::Todo::Ptr>(originalTodo);

        // ... deserialized as a task
        Akonadi::Serializer serializer;
        auto task = serializer.createTaskFromItem(originalItem);

        // WHEN
        // A todo with the project flag
        KCalCore::Todo::Ptr projectTodo(new KCalCore::Todo);
        projectTodo->setSummary(QStringLiteral("foo"));
        projectTodo->setCustomProperty("Zanshin", "Project", QStringLiteral("1"));

        // ... as payload of an item
        Akonadi::Item projectItem;
        projectItem.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        projectItem.setPayload<KCalCore::Todo::Ptr>(projectTodo);
        serializer.updateTaskFromItem(task, projectItem);

        // THEN
        QCOMPARE(task->title(), summary);
        QCOMPARE(task->text(), content);
        QCOMPARE(task->isDone(), isDone);
        QCOMPARE(task->doneDate(), doneDate);
        QCOMPARE(task->startDate(), startDate);
        QCOMPARE(task->dueDate(), dueDate);
        QCOMPARE(task->property("itemId").toLongLong(), originalItem.id());
    }

    void shouldCreateItemFromTask_data()
    {
        QTest::addColumn<QString>("summary");
        QTest::addColumn<QString>("content");
        QTest::addColumn<bool>("isDone");
        QTest::addColumn<QDate>("doneDate");
        QTest::addColumn<QDate>("startDate");
        QTest::addColumn<QDate>("dueDate");
        QTest::addColumn<qint64>("itemId");
        QTest::addColumn<qint64>("parentCollectionId");
        QTest::addColumn<QString>("todoUid");
        QTest::addColumn<Domain::Task::Recurrence>("recurrence");
        QTest::addColumn<Domain::Task::Attachments>("attachments");
        QTest::addColumn<bool>("running");

        Domain::Task::Attachments attachments;

        Domain::Task::Attachment dataAttachment;
        dataAttachment.setData("foo");
        dataAttachment.setLabel("dataAttachment");
        dataAttachment.setMimeType("text/plain");
        dataAttachment.setIconName("text-plain");
        attachments.append(dataAttachment);

        Domain::Task::Attachment uriAttachment;
        uriAttachment.setUri(QUrl("https://www.kde.org"));
        uriAttachment.setLabel("uriAttachment");
        uriAttachment.setMimeType("text/html");
        uriAttachment.setIconName("text-html");
        attachments.append(uriAttachment);

        QTest::newRow("nominal case (no id)") << "summary" << "content" << false << QDate()
                                              << QDate(2013, 11, 24) << QDate(2014, 03, 01)
                                              << qint64(-1) << qint64(-1) << QString()
                                              << Domain::Task::NoRecurrence
                                              << attachments
                                              << false;
        QTest::newRow("nominal case (daily)") << "summary" << "content" << false << QDate()
                                              << QDate(2013, 11, 24) << QDate(2014, 03, 01)
                                              << qint64(-1) << qint64(-1) << QString()
                                              << Domain::Task::RecursDaily
                                              << Domain::Task::Attachments()
                                              << false;
        QTest::newRow("nominal case (weekly)") << "summary" << "content" << false << QDate()
                                               << QDate(2013, 11, 24) << QDate(2014, 03, 01)
                                               << qint64(-1) << qint64(-1) << QString()
                                               << Domain::Task::RecursWeekly
                                               << Domain::Task::Attachments()
                                               << false;
        QTest::newRow("nominal case (monthly)") << "summary" << "content" << false << QDate()
                                                << QDate(2013, 11, 24) << QDate(2014, 03, 01)
                                                << qint64(-1) << qint64(-1) << QString()
                                                << Domain::Task::RecursMonthly
                                                << Domain::Task::Attachments()
                                                << false;
        QTest::newRow("done case (no id)") << "summary" << "content" << true << QDate(2013, 11, 30)
                                           << QDate(2013, 11, 24) << QDate(2014, 03, 01)
                                           << qint64(-1) << qint64(-1) << QString()
                                           << Domain::Task::NoRecurrence
                                           << Domain::Task::Attachments()
                                           << false;
        QTest::newRow("empty case (no id)") << QString() << QString() << false << QDate()
                                            << QDate() << QDate()
                                            << qint64(-1) << qint64(-1) << QString()
                                            << Domain::Task::NoRecurrence
                                            << Domain::Task::Attachments()
                                            << false;
#if 0 // if we ever need time info, then we need a Task::setAllDay(bool) just like KCalCore::Todo has.
      QTest::newRow("nominal_with_time_info_noid") << "summary" << "content" << true << QDateTime(QDate(2015, 3, 1), QTime(1, 2, 3), Qt::UTC)
                                              << QDateTime(QDate(2013, 11, 24), QTime(0, 1, 2), Qt::UTC) << QDateTime(QDate(2016, 3, 1), QTime(4, 5, 6), Qt::UTC)
                                              << qint64(-1) << qint64(-1) << QString()
                                              << Domain::Task::NoRecurrence
                                              << Domain::Task::Attachments()
                                              << false;
#endif

        QTest::newRow("nominal case (with id)") << "summary" << "content" << false << QDate()
                                                << QDate(2013, 11, 24) << QDate(2014, 03, 01)
                                                << qint64(42) << qint64(43) << "my-uid"
                                                << Domain::Task::NoRecurrence
                                                << Domain::Task::Attachments()
                                                << false;
        QTest::newRow("done case (with id)") << "summary" << "content" << true << QDate(2013, 11, 30)
                                             << QDate(2013, 11, 24) << QDate(2014, 03, 01)
                                             << qint64(42) << qint64(43) << "my-uid"
                                             << Domain::Task::NoRecurrence
                                             << Domain::Task::Attachments()
                                             << false;
        QTest::newRow("empty case (with id)") << QString() << QString() << false << QDate()
                                              << QDate() << QDate()
                                              << qint64(42) << qint64(43) << "my-uid"
                                              << Domain::Task::NoRecurrence
                                              << Domain::Task::Attachments()
                                              << false;
        QTest::newRow("nominal case (running)") << "running" << QString() << false << QDate()
                                              << QDate(2013, 11, 24) << QDate(2014, 03, 01)
                                              << qint64(-1) << qint64(-1) << QString()
                                              << Domain::Task::NoRecurrence
                                              << Domain::Task::Attachments()
                                              << true;
    }

    void shouldCreateItemFromTask()
    {
        // GIVEN

        // Data...
        QFETCH(QString, summary);
        QFETCH(QString, content);
        QFETCH(bool, isDone);
        QFETCH(QDate, doneDate);
        QFETCH(QDate, startDate);
        QFETCH(QDate, dueDate);
        QFETCH(qint64, itemId);
        QFETCH(qint64, parentCollectionId);
        QFETCH(QString, todoUid);
        QFETCH(Domain::Task::Recurrence, recurrence);
        QFETCH(Domain::Task::Attachments, attachments);
        QFETCH(bool, running);

        // ... stored in a task
        auto task = Domain::Task::Ptr::create();
        task->setTitle(summary);
        task->setText(content);
        task->setDone(isDone);
        task->setDoneDate(doneDate);
        task->setStartDate(startDate);
        task->setDueDate(dueDate);
        task->setRecurrence(recurrence);
        task->setAttachments(attachments);
        task->setRunning(running);

        if (itemId > 0)
            task->setProperty("itemId", itemId);

        if (parentCollectionId > 0)
            task->setProperty("parentCollectionId", parentCollectionId);

        if (!todoUid.isEmpty())
            task->setProperty("todoUid", todoUid);

        task->setProperty("relatedUid", "parent-uid");

        // WHEN
        Akonadi::Serializer serializer;
        auto item = serializer.createItemFromTask(task);

        // THEN
        QCOMPARE(item.mimeType(), KCalCore::Todo::todoMimeType());

        QCOMPARE(item.isValid(), itemId > 0);
        if (itemId > 0) {
            QCOMPARE(item.id(), itemId);
        }

        QCOMPARE(item.parentCollection().isValid(), parentCollectionId > 0);
        if (parentCollectionId > 0) {
            QCOMPARE(item.parentCollection().id(), parentCollectionId);
        }

        auto todo = item.payload<KCalCore::Todo::Ptr>();
        QCOMPARE(todo->summary(), summary);
        QCOMPARE(todo->description(), content);
        QCOMPARE(todo->isCompleted(), isDone);
        QCOMPARE(todo->completed().toLocalTime().date(), doneDate);
        QCOMPARE(todo->dtStart().toLocalTime().date(), startDate);
        QCOMPARE(todo->dtDue().toLocalTime().date(), dueDate);
        if (todo->dtStart().isValid()) {
            QCOMPARE(int(todo->dtStart().timeSpec()), int(Qt::LocalTime));
        }
        QVERIFY(todo->allDay()); // this is always true currently...
        const ushort expectedRecurrence = recurrence == Domain::Task::NoRecurrence ? KCalCore::Recurrence::rNone
                                        : recurrence == Domain::Task::RecursDaily ? KCalCore::Recurrence::rDaily
                                        : recurrence == Domain::Task::RecursWeekly ? KCalCore::Recurrence::rWeekly
                                        : recurrence == Domain::Task::RecursMonthly ? KCalCore::Recurrence::rMonthlyDay
                                        : KCalCore::Recurrence::rNone; // Shouldn't happen though
        QCOMPARE(todo->recurrence()->recurrenceType(), expectedRecurrence);
        if (recurrence != Domain::Task::NoRecurrence)
            QCOMPARE(todo->recurrence()->frequency(), 1);

        QCOMPARE(todo->attachments().size(), attachments.size());
        for (int i = 0; i < attachments.size(); i++) {
            auto attachment = todo->attachments().at(i);
            QCOMPARE(attachment->isUri(), attachments.at(i).isUri());
            QCOMPARE(QUrl(attachment->uri()), attachments.at(i).uri());
            QCOMPARE(attachment->decodedData(), attachments.at(i).data());
            QCOMPARE(attachment->label(), attachments.at(i).label());
            QCOMPARE(attachment->mimeType(), attachments.at(i).mimeType());
        }

        if (!todoUid.isEmpty()) {
            QCOMPARE(todo->uid(), todoUid);
        }

        QCOMPARE(todo->relatedTo(), QStringLiteral("parent-uid"));
        QCOMPARE(todo->customProperty("Zanshin", "Running"), running ? QStringLiteral("1") : QString());
    }

    void shouldVerifyIfAnItemIsATaskChild_data()
    {
        QTest::addColumn<Domain::Task::Ptr>("task");
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<bool>("isParent");

        // Create task
        const QString summary = QStringLiteral("summary");
        const QString content = QStringLiteral("content");
        const bool isDone = true;
        const QDate doneDate(QDate(2013, 11, 30));
        const QDate startDate(QDate(2013, 11, 24));
        const QDate dueDate(QDate(2014, 03, 01));

        // ... create a task
        Domain::Task::Ptr task(new Domain::Task);
        task->setTitle(summary);
        task->setText(content);
        task->setDone(isDone);
        task->setDoneDate(doneDate);
        task->setStartDate(startDate);
        task->setDueDate(dueDate);
        task->setProperty("todoUid", "1");

        // Create Child item
        KCalCore::Todo::Ptr childTodo(new KCalCore::Todo);
        childTodo->setSummary(summary);
        childTodo->setDescription(content);

        if (isDone)
            childTodo->setCompleted(QDateTime(doneDate));
        else
            childTodo->setCompleted(isDone);

        setTodoDates(childTodo, startDate, dueDate);

        Akonadi::Item childItem;
        childItem.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        childItem.setPayload<KCalCore::Todo::Ptr>(childTodo);

        QTest::newRow("without parent") << task << childItem << false;

        // Create Child Item with parent
        KCalCore::Todo::Ptr childTodo2(new KCalCore::Todo);
        childTodo2->setSummary(summary);
        childTodo2->setDescription(content);

        if (isDone)
            childTodo2->setCompleted(QDateTime(doneDate));
        else
            childTodo2->setCompleted(isDone);
        setTodoDates(childTodo2, startDate, dueDate);
        childTodo2->setRelatedTo(QStringLiteral("1"));

        Akonadi::Item childItem2;
        childItem2.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        childItem2.setPayload<KCalCore::Todo::Ptr>(childTodo2);

        QTest::newRow("with parent") << task << childItem2 << true;

        Domain::Task::Ptr invalidTask(new Domain::Task);
        QTest::newRow("with invalid task") << invalidTask << childItem << false;

        Akonadi::Item invalidItem;
        QTest::newRow("with invalid item") << task << invalidItem << false;

    }

    void shouldVerifyIfAnItemIsATaskChild()
    {
        // GIVEN
        QFETCH(Domain::Task::Ptr, task);
        QFETCH(Akonadi::Item, item);
        QFETCH(bool, isParent);

        // WHEN
        Akonadi::Serializer serializer;
        bool value = serializer.isTaskChild(task, item);

        // THEN
        QCOMPARE(value, isParent);
    }

    void shouldRetrieveRelatedUidFromItem_data()
    {
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<QString>("expectedUid");

        Akonadi::Item item1;
        KCalCore::Todo::Ptr todo1(new KCalCore::Todo);
        item1.setPayload<KCalCore::Todo::Ptr>(todo1);

        Akonadi::Item item2;
        KCalCore::Todo::Ptr todo2(new KCalCore::Todo);
        todo2->setRelatedTo(QStringLiteral("1"));
        item2.setPayload<KCalCore::Todo::Ptr>(todo2);

        QTest::newRow("task without related") << item1 << QString();
        QTest::newRow("task with related") << item2 << "1";
    }

    void shouldRetrieveRelatedUidFromItem()
    {
        // GIVEN
        QFETCH(Akonadi::Item, item);
        QFETCH(QString, expectedUid);

        // WHEN
        Akonadi::Serializer serializer;
        QString uid = serializer.relatedUidFromItem(item);

        // THEN
        QCOMPARE(uid, expectedUid);
    }

    void shouldCreateNoteFromItem_data()
    {
        QTest::addColumn<QString>("title");
        QTest::addColumn<QString>("text");
        QTest::addColumn<QString>("relatedUid");

        QTest::newRow("nominal case (no related)") << "A note title" << "A note content.\nWith two lines." << QString();
        QTest::newRow("nominal case (with related)") << "A note title" << "A note content.\nWith two lines." << "parent-uid";
        QTest::newRow("trailing new lines") << "A note title" << "Empty lines at the end.\n\n\n" << QString();
        QTest::newRow("empty case") << QString() << QString() << QString();
    }

    void shouldCreateProjectFromItem_data()
    {
        QTest::addColumn<QString>("summary");

        QTest::newRow("nominal case") << "summary";
        QTest::newRow("empty case") << QString();
    }

    void shouldCreateProjectFromItem()
    {
        // GIVEN

        // Data...
        QFETCH(QString, summary);

        // ... stored in a todo...
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setSummary(summary);
        todo->setCustomProperty("Zanshin", "Project", QStringLiteral("1"));
        QVERIFY(!todo->uid().isEmpty());

        // ... as payload of an item
        Akonadi::Item item(42);
        item.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        item.setPayload<KCalCore::Todo::Ptr>(todo);

        // which has a parent collection
        Akonadi::Collection collection(43);
        item.setParentCollection(collection);

        // WHEN
        Akonadi::Serializer serializer;
        Domain::Project::Ptr project = serializer.createProjectFromItem(item);

        // THEN
        QCOMPARE(project->name(), summary);
        QCOMPARE(project->property("itemId").toLongLong(), item.id());
        QCOMPARE(project->property("parentCollectionId").toLongLong(), collection.id());
        QCOMPARE(project->property("todoUid").toString(), todo->uid());
    }

    void shouldCreateNullProjectFromInvalidItem()
    {
        // GIVEN
        Akonadi::Item item;

        // WHEN
        Akonadi::Serializer serializer;
        Domain::Project::Ptr project = serializer.createProjectFromItem(item);

        // THEN
        QVERIFY(project.isNull());
    }

    void shouldCreateNullProjectFromTaskItem()
    {
        // GIVEN

        // A todo without the project flag
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setSummary(QStringLiteral("foo"));

        // ... as payload of an item
        Akonadi::Item item;
        item.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        item.setPayload<KCalCore::Todo::Ptr>(todo);

        // WHEN
        Akonadi::Serializer serializer;
        Domain::Project::Ptr project = serializer.createProjectFromItem(item);

        // THEN
        QVERIFY(project.isNull());
    }

    void shouldUpdateProjectFromItem_data()
    {
        QTest::addColumn<QString>("updatedSummary");

        QTest::newRow("no change") << "summary";
        QTest::newRow("changed") << "new summary";
    }

    void shouldUpdateProjectFromItem()
    {
        // GIVEN

        // A todo...
        KCalCore::Todo::Ptr originalTodo(new KCalCore::Todo);
        originalTodo->setSummary(QStringLiteral("summary"));
        originalTodo->setCustomProperty("Zanshin", "Project", QStringLiteral("1"));

        // ... as payload of an item...
        Akonadi::Item originalItem(42);
        originalItem.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        originalItem.setPayload<KCalCore::Todo::Ptr>(originalTodo);

        // ... which has a parent collection...
        Akonadi::Collection originalCollection(43);
        originalItem.setParentCollection(originalCollection);

        // ... deserialized as a project
        Akonadi::Serializer serializer;
        auto project = serializer.createProjectFromItem(originalItem);

        // WHEN

        // Data...
        QFETCH(QString, updatedSummary);

        // ... in a new todo...
        KCalCore::Todo::Ptr updatedTodo(new KCalCore::Todo);
        updatedTodo->setSummary(updatedSummary);
        updatedTodo->setCustomProperty("Zanshin", "Project", QStringLiteral("1"));
        QVERIFY(!updatedTodo->uid().isEmpty());

        // ... as payload of a new item
        Akonadi::Item updatedItem(44);
        updatedItem.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        updatedItem.setPayload<KCalCore::Todo::Ptr>(updatedTodo);

        // ... which has a new parent collection
        Akonadi::Collection updatedCollection(45);
        updatedItem.setParentCollection(updatedCollection);

        serializer.updateProjectFromItem(project, updatedItem);

        // THEN
        QCOMPARE(project->name(), updatedSummary);
        QCOMPARE(project->property("itemId").toLongLong(), updatedItem.id());
        QCOMPARE(project->property("parentCollectionId").toLongLong(), updatedCollection.id());
        QCOMPARE(project->property("todoUid").toString(), updatedTodo->uid());
    }

    void shouldNotUpdateProjectFromInvalidItem()
    {
        // GIVEN

        // Data...
        const QString summary = QStringLiteral("summary");

        // ... stored in a todo...
        KCalCore::Todo::Ptr originalTodo(new KCalCore::Todo);
        originalTodo->setSummary(summary);
        originalTodo->setCustomProperty("Zanshin", "Project", QStringLiteral("1"));

        // ... as payload of an item...
        Akonadi::Item originalItem;
        originalItem.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        originalItem.setPayload<KCalCore::Todo::Ptr>(originalTodo);

        // ... deserialized as a project
        Akonadi::Serializer serializer;
        auto project = serializer.createProjectFromItem(originalItem);

        // WHEN
        Akonadi::Item invalidItem;
        serializer.updateProjectFromItem(project, invalidItem);

        // THEN
        QCOMPARE(project->name(), summary);
    }

    void shouldNotUpdateProjectFromTaskItem()
    {
        // GIVEN

        // Data...
        const QString summary = QStringLiteral("summary");

        // ... stored in a todo...
        KCalCore::Todo::Ptr originalTodo(new KCalCore::Todo);
        originalTodo->setSummary(summary);
        originalTodo->setCustomProperty("Zanshin", "Project", QStringLiteral("1"));

        // ... as payload of an item...
        Akonadi::Item originalItem;
        originalItem.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        originalItem.setPayload<KCalCore::Todo::Ptr>(originalTodo);

        // ... deserialized as a project
        Akonadi::Serializer serializer;
        auto project = serializer.createProjectFromItem(originalItem);

        // WHEN
        // A todo without the project flag
        KCalCore::Todo::Ptr projectTodo(new KCalCore::Todo);
        projectTodo->setSummary(QStringLiteral("foo"));

        // ... as payload of an item
        Akonadi::Item projectItem;
        projectItem.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        projectItem.setPayload<KCalCore::Todo::Ptr>(projectTodo);
        serializer.updateProjectFromItem(project, projectItem);

        // THEN
        QCOMPARE(project->name(), summary);
    }

    void shouldCreateItemFromProject_data()
    {
        QTest::addColumn<QString>("summary");
        QTest::addColumn<qint64>("itemId");
        QTest::addColumn<qint64>("parentCollectionId");

        QTest::newRow("nominal case (no id)") << "summary" << qint64(-1) << qint64(-1);
        QTest::newRow("empty case (no id)") << QString() << qint64(-1) << qint64(-1);

        QTest::newRow("nominal case (with id)") << "summary" << qint64(42) << qint64(43);
        QTest::newRow("empty case (with id)") << QString() << qint64(42) << qint64(43);
    }

    void shouldCreateItemFromProject()
    {
        // GIVEN

        // Data...
        QFETCH(QString, summary);
        QFETCH(qint64, itemId);
        QFETCH(qint64, parentCollectionId);
        const QString todoUid = QStringLiteral("test-uid");

        // ... stored in a project
        auto project = Domain::Project::Ptr::create();
        project->setName(summary);
        project->setProperty("todoUid", todoUid);

        if (itemId > 0)
            project->setProperty("itemId", itemId);

        if (parentCollectionId > 0)
            project->setProperty("parentCollectionId", parentCollectionId);

        // WHEN
        Akonadi::Serializer serializer;
        auto item = serializer.createItemFromProject(project);

        // THEN
        QCOMPARE(item.mimeType(), KCalCore::Todo::todoMimeType());

        QCOMPARE(item.isValid(), itemId > 0);
        if (itemId > 0) {
            QCOMPARE(item.id(), itemId);
        }

        QCOMPARE(item.parentCollection().isValid(), parentCollectionId > 0);
        if (parentCollectionId > 0) {
            QCOMPARE(item.parentCollection().id(), parentCollectionId);
        }

        auto todo = item.payload<KCalCore::Todo::Ptr>();
        QCOMPARE(todo->summary(), summary);
        QCOMPARE(todo->uid(), todoUid);
        QVERIFY(!todo->customProperty("Zanshin", "Project").isEmpty());
    }

    void shouldVerifyIfAnItemIsAProjectChild_data()
    {
        QTest::addColumn<Domain::Project::Ptr>("project");
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<bool>("isParent");

        // Create project
        auto project = Domain::Project::Ptr::create();
        project->setName(QStringLiteral("project"));
        project->setProperty("todoUid", "1");

        // Create unrelated todo
        auto unrelatedTodo = KCalCore::Todo::Ptr::create();
        unrelatedTodo->setSummary(QStringLiteral("summary"));
        Akonadi::Item unrelatedTodoItem;
        unrelatedTodoItem.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        unrelatedTodoItem.setPayload<KCalCore::Todo::Ptr>(unrelatedTodo);

        QTest::newRow("unrelated todo") << project << unrelatedTodoItem << false;

        // Create child todo
        auto childTodo = KCalCore::Todo::Ptr::create();
        childTodo->setSummary(QStringLiteral("summary"));
        childTodo->setRelatedTo(QStringLiteral("1"));
        Akonadi::Item childTodoItem;
        childTodoItem.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        childTodoItem.setPayload<KCalCore::Todo::Ptr>(childTodo);

        QTest::newRow("child todo") << project << childTodoItem << true;

        auto invalidProject = Domain::Project::Ptr::create();
        QTest::newRow("invalid project") << invalidProject << unrelatedTodoItem << false;

        Akonadi::Item invalidItem;
        QTest::newRow("invalid item") << project << invalidItem << false;

    }

    void shouldVerifyIfAnItemIsAProjectChild()
    {
        // GIVEN
        QFETCH(Domain::Project::Ptr, project);
        QFETCH(Akonadi::Item, item);
        QFETCH(bool, isParent);

        // WHEN
        Akonadi::Serializer serializer;
        bool value = serializer.isProjectChild(project, item);

        // THEN
        QCOMPARE(value, isParent);
    }

    void shouldUpdateItemParent_data()
    {
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<Domain::Task::Ptr>("parent");
        QTest::addColumn<QString>("expectedRelatedToUid");

        Akonadi::Item item1;
        KCalCore::Todo::Ptr todo1(new KCalCore::Todo);
        item1.setPayload<KCalCore::Todo::Ptr>(todo1);

        Domain::Task::Ptr parent(new Domain::Task);
        parent->setProperty("todoUid", "1");

        QTest::newRow("nominal case") << item1 << parent << "1";

        Akonadi::Item item2;
        QTest::newRow("update item without payload") << item2 << parent << QString();

        Domain::Task::Ptr parent2(new Domain::Task);
        QTest::newRow("update item with a empty parent uid") << item1 << parent2 << QString();
    }

    void shouldUpdateItemParent()
    {
        // GIVEN
        QFETCH(Akonadi::Item, item);
        QFETCH(Domain::Task::Ptr, parent);
        QFETCH(QString, expectedRelatedToUid);

        // WHEN
        Akonadi::Serializer serializer;
        serializer.updateItemParent(item, parent);

        // THEN
        if (item.hasPayload<KCalCore::Todo::Ptr>()) {
            auto todo = item.payload<KCalCore::Todo::Ptr>();
            QString relatedUid = todo->relatedTo();
            QCOMPARE(relatedUid, expectedRelatedToUid);
        }
    }

    void shouldUpdateItemProject_data()
    {
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<Domain::Project::Ptr>("parent");
        QTest::addColumn<QString>("expectedRelatedToUid");

        Akonadi::Item todoItem;
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todoItem.setPayload<KCalCore::Todo::Ptr>(todo);

        auto parent = Domain::Project::Ptr::create();
        parent->setProperty("todoUid", "1");

        QTest::newRow("nominal todo case") << todoItem << parent << "1";

        auto invalidParent = Domain::Project::Ptr::create();
        QTest::newRow("update todo item with a empty parent uid") << todoItem << invalidParent << QString();

        Akonadi::Item invalidItem;
        QTest::newRow("update item without payload") << invalidItem << parent << QString();
    }

    void shouldUpdateItemProject()
    {
        // GIVEN
        QFETCH(Akonadi::Item, item);
        QFETCH(Domain::Project::Ptr, parent);
        QFETCH(QString, expectedRelatedToUid);

        // WHEN
        Akonadi::Serializer serializer;
        serializer.updateItemProject(item, parent);

        // THEN
        if (item.hasPayload<KCalCore::Todo::Ptr>()) {
            auto todo = item.payload<KCalCore::Todo::Ptr>();
            const QString relatedUid = todo->relatedTo();
            QCOMPARE(relatedUid, expectedRelatedToUid);
        }
    }

    void shouldFilterChildrenItem_data()
    {
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<Akonadi::Item::List>("items");
        QTest::addColumn<int>("size");

        Akonadi::Item item(12);
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setUid(QStringLiteral("1"));
        item.setPayload<KCalCore::Todo::Ptr>(todo);
        Akonadi::Item::List items;

        QTest::newRow("empty list") << item << items << 0;

        Akonadi::Item item2(13);
        KCalCore::Todo::Ptr todo2(new KCalCore::Todo);
        item2.setPayload<KCalCore::Todo::Ptr>(todo2);
        Akonadi::Item::List items2;
        items2 << item2;

        QTest::newRow("list without child") << item << items2 << 0;

        Akonadi::Item item3(14);
        KCalCore::Todo::Ptr todo3(new KCalCore::Todo);
        todo3->setUid(QStringLiteral("3"));
        todo3->setRelatedTo(QStringLiteral("1"));
        item3.setPayload<KCalCore::Todo::Ptr>(todo3);
        Akonadi::Item::List items3;
        items3 << item2 << item3;

        QTest::newRow("list with child") << item << items3 << 1;

        Akonadi::Item item4(15);
        KCalCore::Todo::Ptr todo4(new KCalCore::Todo);
        todo4->setRelatedTo(QStringLiteral("3"));
        item4.setPayload<KCalCore::Todo::Ptr>(todo4);
        Akonadi::Item::List items4;
        items4 << item2 << item3 << item4;

        QTest::newRow("list with child with a child") << item << items4 << 2;

        Akonadi::Item::List items5;
        items5 << item << item2 << item3 << item4;
        QTest::newRow("list with filter in list") << item << items5 << 2;
    }

    void shouldFilterChildrenItem()
    {
        // GIVEN
        QFETCH(Akonadi::Item, item);
        QFETCH(Akonadi::Item::List, items);
        QFETCH(int, size);

        // WHEN
        Akonadi::Serializer serializer;
        Akonadi::Item::List list = serializer.filterDescendantItems(items, item);

        // THEN
        QCOMPARE(list.size(), size);
    }

    void shouldRemoveItemParent_data()
    {
        QTest::addColumn<Akonadi::Item>("item");

        Akonadi::Item item(15);
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setRelatedTo(QStringLiteral("3"));
        item.setPayload<KCalCore::Todo::Ptr>(todo);

        QTest::newRow("nominal case") << item;

        Akonadi::Item item2(16);
        QTest::newRow("parent invalid") << item2;
    }

    void shouldRemoveItemParent()
    {
        // GIVEN
        QFETCH(Akonadi::Item, item);

        // WHEN
        Akonadi::Serializer serializer;
        serializer.removeItemParent(item);

        // THEN
        if (item.hasPayload<KCalCore::Todo::Ptr>())
            QCOMPARE(item.payload<KCalCore::Todo::Ptr>()->relatedTo(), QString());
    }

    void shouldPromoteItemToProject_data()
    {
        QTest::addColumn<Akonadi::Item>("item");

        auto item = Akonadi::Item(15);
        auto todo = KCalCore::Todo::Ptr::create();
        todo->setRelatedTo(QStringLiteral("3"));
        item.setPayload(todo);

        QTest::newRow("nominal case") << item;
        QTest::newRow("invalid item") << Akonadi::Item(16);
    }

    void shouldPromoteItemToProject()
    {
        // GIVEN
        QFETCH(Akonadi::Item, item);

        // WHEN
        Akonadi::Serializer serializer;
        serializer.promoteItemToProject(item);

        // THEN
        if (item.hasPayload<KCalCore::Todo::Ptr>()) {
            auto todo = item.payload<KCalCore::Todo::Ptr>();
            QCOMPARE(todo->relatedTo(), QString());
            QVERIFY(!todo->customProperty("Zanshin", "Project").isEmpty());
        }
    }

    void shouldClearItem_data()
    {
        QTest::addColumn<Akonadi::Item*>("item");

        Akonadi::Item *itemWithContexts = new Akonadi::Item(15);
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        // we can cheat and not really create contexts...
        todo->setCustomProperty(s_appName, s_contextListProperty, "one,two");
        itemWithContexts->setPayload<KCalCore::Todo::Ptr>(todo);
        QTest::newRow("with_contexts") << itemWithContexts;

        Akonadi::Item *itemWithNoContext = new Akonadi::Item(15);
        itemWithNoContext->setPayload<KCalCore::Todo::Ptr>(todo);
        QTest::newRow("no_context") << itemWithNoContext;
    }

    void shouldClearItem()
    {
        // GIVEN
        QFETCH(Akonadi::Item*, item);

        // WHEN
        Akonadi::Serializer serializer;
        serializer.clearItem(item);

        // THEN
        auto todo = item->payload<KCalCore::Todo::Ptr>();
        QVERIFY(todo->customProperty(s_appName, s_contextListProperty).isEmpty());
        delete item;
    }

    void shouldCreateContextFromItem_data()
    {
        QTest::addColumn<QString>("name");

        QTest::newRow("nominal case") << "Context42";
        QTest::newRow("empty name case") << "";
    }

    void shouldCreateContextFromItem()
    {
        // GIVEN

        // Data...
        QFETCH(QString, name);

        // ... stored in a todo...
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setSummary(name);
        todo->setCustomProperty(s_appName, "Context", QStringLiteral("1"));
        QVERIFY(!todo->uid().isEmpty());

        // ... as payload of an item
        Akonadi::Item item(42);
        item.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        item.setPayload<KCalCore::Todo::Ptr>(todo);

        // which has a parent collection
        Akonadi::Collection collection(43);
        item.setParentCollection(collection);

        // WHEN
        Akonadi::Serializer serializer;
        Domain::Context::Ptr context = serializer.createContextFromItem(item);

        // THEN
        QCOMPARE(context->name(), name);
        QCOMPARE(context->property("todoUid").toString(), todo->uid());
        QCOMPARE(context->property("itemId").toLongLong(), item.id());
        QCOMPARE(context->property("parentCollectionId").toLongLong(), collection.id());
    }

    void shouldNotCreateContextFromWrongItemType_data()
    {
        QTest::addColumn<bool>("isProject");

        QTest::newRow("project") << true;
        QTest::newRow("task") << false;
    }

    void shouldNotCreateContextFromWrongItemType()
    {
        // GIVEN
        QFETCH(bool, isProject);

        // A project todo
        KCalCore::Todo::Ptr originalTodo(new KCalCore::Todo);
        originalTodo->setSummary("summary");
        if (isProject)
            originalTodo->setCustomProperty(s_appName, "Project", QStringLiteral("1"));

        // ... as payload of an item...
        Akonadi::Item originalItem;
        originalItem.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        originalItem.setPayload<KCalCore::Todo::Ptr>(originalTodo);

        // WHEN
        Akonadi::Serializer serializer;
        Domain::Context::Ptr context = serializer.createContextFromItem(originalItem);

        // THEN
        QVERIFY(!context);
    }

    void shouldUpdateContextFromItem_data()
    {
        shouldCreateContextFromItem_data();
    }

    void shouldUpdateContextFromItem()
    {
        // GIVEN

        // Data...
        QFETCH(QString, name);

        // ... stored in a todo...
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setSummary(name);
        todo->setCustomProperty(s_appName, "Context", QStringLiteral("1"));
        QVERIFY(!todo->uid().isEmpty());

        // ... as payload of an item
        Akonadi::Item item(42);
        item.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        item.setPayload<KCalCore::Todo::Ptr>(todo);

        // WHEN
        Akonadi::Serializer serializer;
        Domain::Context::Ptr context(new Domain::Context);

        serializer.updateContextFromItem(context, item);

        // THEN
        QCOMPARE(context->name(), name);
        QCOMPARE(context->property("todoUid").toString(), todo->uid());
        QCOMPARE(context->property("itemId").toLongLong(), item.id());
    }

    void shouldNotUpdateContextFromWrongItemType_data()
    {
        QTest::addColumn<bool>("isProject");

        QTest::newRow("project") << true;
        QTest::newRow("task") << false;
    }

    void shouldNotUpdateContextFromWrongItemType()
    {
        // GIVEN
        QFETCH(bool, isProject);

        // A context
        auto context = Domain::Context::Ptr::create();
        context->setName("summary");
        context->setProperty("todoUid", qint64(43));

        KCalCore::Todo::Ptr wrongTodo(new KCalCore::Todo);
        wrongTodo->setSummary("wrongSummary");
        if (isProject)
            wrongTodo->setCustomProperty(s_appName, "Project", QStringLiteral("1"));

        Akonadi::Item wrongItem;
        wrongItem.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
        wrongItem.setPayload<KCalCore::Todo::Ptr>(wrongTodo);

        // WHEN
        Akonadi::Serializer serializer;
        serializer.updateContextFromItem(context, wrongItem);

        // THEN
        QCOMPARE(context->name(), "summary");
        QCOMPARE(context->property("todoUid").toLongLong(), qint64(43));
    }

    void shouldVerifyIfAnItemIsAContextChild_data()
    {
        QTest::addColumn<Domain::Context::Ptr>("context");
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<bool>("isChild");

        // Create a context
        auto context = Domain::Context::Ptr::create();
        const QString contextUid = QStringLiteral("abc-123");
        context->setProperty("todoUid", contextUid);

        Akonadi::Item unrelatedItem;
        QTest::newRow("Unrelated item") << context << unrelatedItem << false;

        Akonadi::Item relatedItem;
        auto todo = KCalCore::Todo::Ptr::create();
        todo->setSummary("summary");
        todo->setCustomProperty(s_appName, s_contextListProperty, contextUid);
        relatedItem.setPayload<KCalCore::Todo::Ptr>(todo);
        QTest::newRow("Related item") << context << relatedItem << true;

        auto invalidContext = Domain::Context::Ptr::create();
        QTest::newRow("Invalid context") << invalidContext << relatedItem << false;

        Akonadi::Item invalidItem;
        QTest::newRow("Invalid Item") << context << invalidItem << false;
    }

    void shouldVerifyIfAnItemIsAContextChild()
    {
        // GIVEN
        QFETCH(Domain::Context::Ptr, context);
        QFETCH(Akonadi::Item, item);
        QFETCH(bool, isChild);

        // WHEN
        Akonadi::Serializer serializer;
        bool value = serializer.isContextChild(context, item);

        // THEN
        QCOMPARE(value, isChild);
    }

    void shouldCreateItemFromContext_data()
    {
        QTest::addColumn<QString>("name");
        QTest::addColumn<qint64>("itemId");
        QTest::addColumn<QString>("todoUid");

        const QString nameInternet = QStringLiteral("Internet");
        const QString uid = QStringLiteral("uid-123");

        QTest::newRow("nominal_case") << nameInternet << qint64(42) << uid;
        QTest::newRow("no_item_id") << nameInternet<< qint64(-1) << uid;
        QTest::newRow("null_uid") << nameInternet << qint64(42) << QString();
        QTest::newRow("no_name") << QString() << qint64(42) << uid;
    }

    void shouldCreateItemFromContext()
    {
        // GIVEN
        QFETCH(QString, name);
        QFETCH(qint64, itemId);
        QFETCH(QString, todoUid);

        // WHEN
        auto context = Domain::Context::Ptr::create();
        context->setProperty("todoUid", todoUid);
        if (itemId > 0)
            context->setProperty("itemId", itemId);

        context->setName(name);

        Akonadi::Serializer serializer;
        Akonadi::Item item = serializer.createItemFromContext(context);
        auto todo = item.payload<KCalCore::Todo::Ptr>();

        // THEN
        QCOMPARE(todo->summary(), name);

        if (!todoUid.isEmpty()) {
            QCOMPARE(todo->uid(), todoUid);
        }
        QCOMPARE(item.id(), itemId);
    }

    void shouldTestIfItemRepresentsContext_data()
    {
        QTest::addColumn<QString>("contextUid");
        QTest::addColumn<QString>("todoUid");
        QTest::addColumn<bool>("expectedResult");

        QTest::newRow("yes") << "context-123" << "context-123" << true;
        QTest::newRow("no") << "context-123" << "another-context" << false;
    }

    void shouldTestIfItemRepresentsContext()
    {
        // GIVEN
        QFETCH(QString, contextUid);
        QFETCH(QString, todoUid);
        QFETCH(bool, expectedResult);

        auto context = Domain::Context::Ptr::create();
        context->setProperty("todoUid", contextUid);

        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setCustomProperty(s_appName, "Context", QStringLiteral("1"));
        todo->setUid(todoUid);
        Akonadi::Item item;
        item.setPayload<KCalCore::Todo::Ptr>(todo);

        // WHEN
        Akonadi::Serializer serializer;
        const bool result = serializer.itemRepresentsContext(context, item);

        // THEN
        QCOMPARE(result, expectedResult);
        QCOMPARE(serializer.contextUid(item), todoUid);
    }

    void shouldAddContextToTask_data()
    {
        QTest::addColumn<Domain::Context::Ptr>("context");
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<QString>("expectedContextList");

        // Create a context
        auto context = Domain::Context::Ptr::create();
        const QString contextUid = QStringLiteral("abc-123");
        context->setProperty("todoUid", contextUid);

        Akonadi::Item item;
        auto todo = KCalCore::Todo::Ptr::create();
        todo->setSummary("summary");
        item.setPayload<KCalCore::Todo::Ptr>(todo);
        QTest::newRow("item_with_no_context") << context << item << contextUid;

        Akonadi::Item item2;
        auto todo2 = KCalCore::Todo::Ptr::create();
        todo2->setCustomProperty(s_appName, s_contextListProperty, "another");
        item2.setPayload<KCalCore::Todo::Ptr>(todo2);
        const QString bothContexts = QStringLiteral("another,") + contextUid;
        QTest::newRow("item_with_another_context") << context << item2 << bothContexts;

        Akonadi::Item item3;
        auto todo3 = KCalCore::Todo::Ptr::create();
        todo3->setCustomProperty(s_appName, s_contextListProperty, bothContexts);
        item3.setPayload<KCalCore::Todo::Ptr>(todo3);
        QTest::newRow("item_with_this_context_already") << context << item3 << bothContexts;
    }

    void shouldAddContextToTask()
    {
        // GIVEN
        QFETCH(Domain::Context::Ptr, context);
        QFETCH(Akonadi::Item, item);
        QFETCH(QString, expectedContextList);

        // WHEN
        Akonadi::Serializer serializer;
        serializer.addContextToTask(context, item);

        // THEN
        KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();
        QCOMPARE(todo->customProperty(s_appName, s_contextListProperty), expectedContextList);
    }

    void shouldRemoveContextFromTask_data()
    {
        QTest::addColumn<Domain::Context::Ptr>("context");
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<QString>("expectedContextList");

        auto context = Domain::Context::Ptr::create();
        const QString contextUid = QStringLiteral("abc-123");
        context->setProperty("todoUid", contextUid);

        Akonadi::Item item;
        auto todo = KCalCore::Todo::Ptr::create();
        todo->setSummary("summary");
        item.setPayload<KCalCore::Todo::Ptr>(todo);
        QTest::newRow("item_with_no_context") << context << item << QString();

        Akonadi::Item item2;
        auto todo2 = KCalCore::Todo::Ptr::create();
        todo2->setCustomProperty(s_appName, s_contextListProperty, "another");
        item2.setPayload<KCalCore::Todo::Ptr>(todo2);
        QTest::newRow("item_with_another_context") << context << item2 << QString("another");

        Akonadi::Item item3;
        auto todo3 = KCalCore::Todo::Ptr::create();
        todo3->setCustomProperty(s_appName, s_contextListProperty, contextUid);
        item3.setPayload<KCalCore::Todo::Ptr>(todo3);
        QTest::newRow("item_with_this_context_already") << context << item3 << QString();

        Akonadi::Item item4;
        auto todo4 = KCalCore::Todo::Ptr::create();
        const QString bothContexts = QStringLiteral("another,") + contextUid;
        todo4->setCustomProperty(s_appName, s_contextListProperty, bothContexts);
        item4.setPayload<KCalCore::Todo::Ptr>(todo4);
        QTest::newRow("item_with_two_contexts") << context << item4 << QString("another");
    }

    void shouldRemoveContextFromTask()
    {
        // GIVEN
        QFETCH(Domain::Context::Ptr, context);
        QFETCH(Akonadi::Item, item);
        QFETCH(QString, expectedContextList);

        // WHEN
        Akonadi::Serializer serializer;
        serializer.removeContextFromTask(context, item);

        // THEN
        KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();
        QCOMPARE(todo->customProperty(s_appName, s_contextListProperty), expectedContextList);
    }

    // Investigation into how to differentiate all-day events from events with time,
    // using QDateTime only. Doesn't seem to be possible.
    void noWayToHaveQDateTimeWithoutTime()
    {
        // GIVEN a QDateTime without time information
        QDateTime dateOnly(QDate(2016, 6, 12), QTime(-1, -1, -1));
        // THEN we can't detect that there was no time information, i.e. all day event
        QVERIFY(dateOnly.time().isValid()); // I wish this was "!"
        QVERIFY(!dateOnly.time().isNull()); // got converted to midnight localtime by QDateTime
        // This doesn't help, QDateTime converts "null time" to midnight.
        dateOnly.setTime(QTime());
        QVERIFY(dateOnly.time().isValid()); // same as above
        QVERIFY(!dateOnly.time().isNull()); // same as above

        // GIVEN a QDateTime at midnight
        QDateTime atMidnight(QDate(2016, 6, 12), QTime(0, 0, 0));
        // THEN we can detect that a time information was present
        QVERIFY(atMidnight.time().isValid());
        QVERIFY(!atMidnight.time().isNull());

#if 0
        // GIVEN a KDateTime without time information
        KDateTime kdOnly(QDate(2016, 6, 12));
        // THEN we can detect that there was no time information, i.e. all day event
        QVERIFY(kdOnly.isDateOnly());
#endif
    }
};

ZANSHIN_TEST_MAIN(AkonadiSerializerTest)

#include "akonadiserializertest.moc"
