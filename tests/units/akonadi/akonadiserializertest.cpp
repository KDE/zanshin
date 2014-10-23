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

#include <QtTest>

#include "akonadi/akonadiserializer.h"
#include "akonadi/akonadiapplicationselectedattribute.h"
#include "akonadi/akonaditimestampattribute.h"

#include <Akonadi/Collection>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/Item>
#include <Akonadi/Notes/NoteUtils>
#include <KCalCore/Todo>
#include <KMime/Message>

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

    void shouldKnowObjectUid()
    {
        // GIVEN
        Akonadi::Serializer serializer;
        auto object = Akonadi::Serializer::QObjectPtr::create();

        // WHEN
        object->setProperty("todoUid", "my-uid");

        // THEN
        QCOMPARE(serializer.objectUid(object), QString("my-uid"));
    }

    void shouldCreateDataSourceFromCollection_data()
    {
        QTest::addColumn<QString>("name");
        QTest::addColumn<QString>("iconName");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<bool>("hasSelectedAttribute");
        QTest::addColumn<bool>("isSelected");
        QTest::addColumn<bool>("isReferenced");
        QTest::addColumn<bool>("isEnabled");

        const auto noteMimeTypes = QStringList() << "text/x-vnd.akonadi.note";
        const auto taskMimeTypes = QStringList() << "application/x-vnd.akonadi.calendar.todo";
        const auto bogusMimeTypes = QStringList() << "foo/bar";
        const auto allMimeTypes = noteMimeTypes + taskMimeTypes + bogusMimeTypes;

        QTest::newRow("nominal case") << "name" << "icon" << allMimeTypes << true << false << false << true;

        QTest::newRow("only notes") << "name" << "icon" << noteMimeTypes << true << false << false << true;
        QTest::newRow("only tasks") << "name" << "icon" << taskMimeTypes << true << false << false << true;
        QTest::newRow("only bogus") << "name" << "icon" << bogusMimeTypes << true << false << false << true;

        QTest::newRow("no selected attribute") << "name" << "icon" << allMimeTypes << false << false << false << true;
        QTest::newRow("selected attribute (false)") << "name" << "icon" << allMimeTypes << true << false << false << true;
        QTest::newRow("selected attribute (true)") << "name" << "icon" << allMimeTypes << true << true << false << true;

        QTest::newRow("enabled and referenced") << "name" << "icon" << allMimeTypes << true << false << true << true;
        QTest::newRow("enabled and !referenced") << "name" << "icon" << allMimeTypes << true << false << true << false;
        QTest::newRow("!enabled and referenced") << "name" << "icon" << allMimeTypes << true << false << false << true;
        QTest::newRow("!enabled and !referenced") << "name" << "icon" << allMimeTypes << true << false << false << false;

        QTest::newRow("empty case") << QString() << QString() << QStringList() << false << false << false << true;
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
        QFETCH(bool, isReferenced);
        QFETCH(bool, isEnabled);

        Domain::DataSource::ContentTypes expectedContentTypes;
        if (mimeTypes.contains("text/x-vnd.akonadi.note")) {
            expectedContentTypes |= Domain::DataSource::Notes;
        }
        if (mimeTypes.contains("application/x-vnd.akonadi.calendar.todo")) {
            expectedContentTypes |= Domain::DataSource::Tasks;
        }

        // ... stored in a collection
        Akonadi::Collection collection(42);
        collection.setContentMimeTypes(mimeTypes);
        collection.setName(name);
        collection.setReferenced(isReferenced);
        collection.setEnabled(isEnabled);
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
        QCOMPARE((dataSource->listStatus() & Domain::DataSource::Listed) != 0, isReferenced || isEnabled);
        QCOMPARE((dataSource->listStatus() == Domain::DataSource::Bookmarked), isEnabled);
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
        originalCollection.setName("name");

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
        const QString name = "name";

        // ... stored in a collection...
        Akonadi::Collection originalCollection(42);
        originalCollection.setName(name);

        // ... deserialized as a data source
        Akonadi::Serializer serializer;
        auto dataSource = serializer.createDataSourceFromCollection(originalCollection, Akonadi::SerializerInterface::BaseName);

        // WHEN
        Akonadi::Collection invalidCollection;
        invalidCollection.setName("foo");
        serializer.updateDataSourceFromCollection(dataSource, invalidCollection, Akonadi::SerializerInterface::BaseName);

        // THEN
        QCOMPARE(dataSource->name(), name);
    }

    void shouldNameDataSourceFromCollectionPathIfRequested()
    {
        // GIVEN

        // Data...
        const QString name = "name";
        const QString parentName = "parent";

        // ... stored in a collection with a parent
        Akonadi::Collection collection(42);
        collection.setName(name);
        Akonadi::Collection parentCollection(41);
        parentCollection.setName("Foo");
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
        QCOMPARE(dataSource1->name(), QString(parentName + "/" + name));
        QCOMPARE(dataSource2->name(), name);
        QCOMPARE(dataSource3->name(), QString(parentName + "/" + name));
        QCOMPARE(dataSource4->name(), name);
    }

    void shouldCreateCollectionFromDataSource_data()
    {
        QTest::addColumn<QString>("name");
        QTest::addColumn<QString>("iconName");
        QTest::addColumn<Domain::DataSource::ContentTypes>("contentTypes");
        QTest::addColumn<bool>("isSelected");
        QTest::addColumn<Domain::DataSource::ListStatus>("listStatus");

        const auto noType = Domain::DataSource::ContentTypes(Domain::DataSource::NoContent);
        const auto taskType = Domain::DataSource::ContentTypes(Domain::DataSource::Tasks);
        const auto noteType = Domain::DataSource::ContentTypes(Domain::DataSource::Notes);
        const auto allTypes = taskType | noteType;

        const auto unlisted = Domain::DataSource::Unlisted;
        const auto listed = Domain::DataSource::Listed;
        const auto bookmarked = Domain::DataSource::Bookmarked;

        QTest::newRow("nominal case") << "name" << "icon-name" << allTypes << true << unlisted;

        QTest::newRow("only notes") << "name" << "icon-name" << noteType << true << unlisted;
        QTest::newRow("only tasks") << "name" << "icon-name" << taskType << true << unlisted;
        QTest::newRow("only nothing ;)") << "name" << "icon-name" << noType << true << unlisted;

        QTest::newRow("not selected") << "name" << "icon-name" << allTypes << false << unlisted;
        QTest::newRow("selected") << "name" << "icon-name" << allTypes << true << unlisted;

        QTest::newRow("unlisted") << "name" << "icon-name" << allTypes << true << unlisted;
        QTest::newRow("listed") << "name" << "icon-name" << allTypes << true << listed;
        QTest::newRow("bookmarked") << "name" << "icon-name" << allTypes << true << bookmarked;

        QTest::newRow("empty case") << QString() << QString() << noType << true << unlisted;
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
        QFETCH(Domain::DataSource::ListStatus, listStatus);

        QStringList mimeTypes;
        if (contentTypes & Domain::DataSource::Tasks)
            mimeTypes << "application/x-vnd.akonadi.calendar.todo";
        if (contentTypes & Domain::DataSource::Notes)
            mimeTypes << "text/x-vnd.akonadi.note";


        // ... stored in a data source
        auto source = Domain::DataSource::Ptr::create();
        source->setName(name);
        source->setIconName(iconName);
        source->setContentTypes(contentTypes);
        source->setListStatus(listStatus);
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

        switch (listStatus) {
        case Domain::DataSource::Unlisted:
            QVERIFY(!collection.referenced());
            QVERIFY(!collection.enabled());
            break;
        case Domain::DataSource::Listed:
            QVERIFY(collection.referenced());
            QVERIFY(!collection.enabled());
            break;
        case Domain::DataSource::Bookmarked:
            QVERIFY(collection.enabled());
            break;
        default:
            qFatal("Shouldn't happen");
            break;
        }
    }

    void shouldVerifyIfCollectionIsSelected_data()
    {
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<bool>("hasSelectedAttribute");
        QTest::addColumn<bool>("isSelected");
        QTest::addColumn<bool>("isReferenced");
        QTest::addColumn<bool>("isEnabled");
        QTest::addColumn<bool>("expectedSelected");

        const auto noteMimeTypes = QStringList() << "text/x-vnd.akonadi.note";
        const auto taskMimeTypes = QStringList() << "application/x-vnd.akonadi.calendar.todo";
        const auto bogusMimeTypes = QStringList() << "foo/bar";
        const auto allMimeTypes = noteMimeTypes + taskMimeTypes + bogusMimeTypes;

        QTest::newRow("nominal case") << allMimeTypes << true << false << false << true << false;

        QTest::newRow("only notes") << noteMimeTypes << true << false << false << true << false;
        QTest::newRow("only tasks") << taskMimeTypes << true << false << false << true << false;
        QTest::newRow("only bogus") << bogusMimeTypes << true << false << false << true << false;

        QTest::newRow("selected, only notes") << noteMimeTypes << true << true << false << true << true;
        QTest::newRow("selected, only tasks") << taskMimeTypes << true << true << false << true << true;
        QTest::newRow("selected, only bogus") << bogusMimeTypes << true << true << false << true << false;

        QTest::newRow("no selected attribute") << allMimeTypes << false << false << false << true << true;
        QTest::newRow("selected attribute (false)") << allMimeTypes << true << false << false << true << false;
        QTest::newRow("selected attribute (true)") << allMimeTypes << true << true << false << true << true;

        QTest::newRow("enabled and referenced") << allMimeTypes << true << false << true << true << false;
        QTest::newRow("enabled and !referenced") << allMimeTypes << true << false << true << false << false;
        QTest::newRow("!enabled and referenced") << allMimeTypes << true << false << false << true << false;
        QTest::newRow("!enabled and !referenced") << allMimeTypes << true << false << false << false << false;

        QTest::newRow("selected, enabled and referenced") << allMimeTypes << true << true << true << true << true;
        QTest::newRow("selected, enabled and !referenced") << allMimeTypes << true << true << true << false << true;
        QTest::newRow("selected, !enabled and referenced") << allMimeTypes << true << true << false << true << true;
        QTest::newRow("selected, !enabled and !referenced") << allMimeTypes << true << true << false << false << false;

        QTest::newRow("empty case") << QStringList() << false << false << false << true << false;
    }

    void shouldVerifyIfCollectionIsSelected()
    {
        // GIVEN
        QFETCH(QStringList, mimeTypes);
        QFETCH(bool, hasSelectedAttribute);
        QFETCH(bool, isSelected);
        QFETCH(bool, isReferenced);
        QFETCH(bool, isEnabled);

        Domain::DataSource::ContentTypes expectedContentTypes;
        if (mimeTypes.contains("text/x-vnd.akonadi.note")) {
            expectedContentTypes |= Domain::DataSource::Notes;
        }
        if (mimeTypes.contains("application/x-vnd.akonadi.calendar.todo")) {
            expectedContentTypes |= Domain::DataSource::Tasks;
        }

        // ... stored in a collection
        Akonadi::Collection collection(42);
        collection.setContentMimeTypes(mimeTypes);
        collection.setReferenced(isReferenced);
        collection.setEnabled(isEnabled);
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
        QTest::addColumn<bool>("expectedNotes");
        QTest::addColumn<bool>("expectedTasks");

        QTest::newRow("task collection") << "application/x-vnd.akonadi.calendar.todo" << false << true;
        QTest::newRow("note collection") << "text/x-vnd.akonadi.note" << true << false;
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
        QFETCH(bool, expectedNotes);
        QFETCH(bool, expectedTasks);

        // THEN
        QCOMPARE(serializer.isNoteCollection(collection), expectedNotes);
        QCOMPARE(serializer.isTaskCollection(collection), expectedTasks);
    }

    void shouldCreateTaskFromItem_data()
    {
        QTest::addColumn<QString>("summary");
        QTest::addColumn<QString>("content");
        QTest::addColumn<bool>("isDone");
        QTest::addColumn<QDateTime>("startDate");
        QTest::addColumn<QDateTime>("dueDate");

        QTest::newRow("nominal case") << "summary" << "content" << false << QDateTime(QDate(2013, 11, 24)) << QDateTime(QDate(2014, 03, 01));
        QTest::newRow("done case") << "summary" << "content" << true << QDateTime(QDate(2013, 11, 24)) << QDateTime(QDate(2014, 03, 01));
        QTest::newRow("empty case") << QString() << QString() << false << QDateTime() << QDateTime();
    }

    void shouldCreateTaskFromItem()
    {
        // GIVEN

        // Data...
        QFETCH(QString, summary);
        QFETCH(QString, content);
        QFETCH(bool, isDone);
        QFETCH(QDateTime, startDate);
        QFETCH(QDateTime, dueDate);

        // ... stored in a todo...
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setSummary(summary);
        todo->setDescription(content);
        todo->setCompleted(isDone);
        todo->setDtStart(KDateTime(startDate));
        todo->setDtDue(KDateTime(dueDate));
        todo->setRelatedTo("my-uid");

        // ... as payload of an item
        Akonadi::Item item;
        item.setMimeType("application/x-vnd.akonadi.calendar.todo");
        item.setPayload<KCalCore::Todo::Ptr>(todo);

        // WHEN
        Akonadi::Serializer serializer;
        Domain::Task::Ptr task = serializer.createTaskFromItem(item);

        // THEN
        QCOMPARE(task->title(), summary);
        QCOMPARE(task->text(), content);
        QCOMPARE(task->isDone(), isDone);
        QCOMPARE(task->startDate(), startDate);
        QCOMPARE(task->dueDate(), dueDate);
        QCOMPARE(task->property("todoUid").toString(), todo->uid());
        QCOMPARE(task->property("relatedUid").toString(), todo->relatedTo());
        QCOMPARE(task->property("itemId").toLongLong(), item.id());
    }

    void shouldCreateNullTaskFromInvalidItem()
    {
        // GIVEN
        Akonadi::Item item;

        // WHEN
        Akonadi::Serializer serializer;
        Domain::Task::Ptr task = serializer.createTaskFromItem(item);

        // THEN
        QVERIFY(task.isNull());
    }

    void shouldCreateNullTaskFromProjectItem()
    {
        // GIVEN

        // A todo with the project flag
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setSummary("foo");
        todo->setCustomProperty("Zanshin", "Project", "1");

        // ... as payload of an item
        Akonadi::Item item;
        item.setMimeType("application/x-vnd.akonadi.calendar.todo");
        item.setPayload<KCalCore::Todo::Ptr>(todo);

        // WHEN
        Akonadi::Serializer serializer;
        Domain::Task::Ptr task = serializer.createTaskFromItem(item);

        // THEN
        QVERIFY(task.isNull());
    }

    void shouldUpdateTaskFromItem_data()
    {
        QTest::addColumn<QString>("updatedSummary");
        QTest::addColumn<QString>("updatedContent");
        QTest::addColumn<bool>("updatedDone");
        QTest::addColumn<QDateTime>("updatedStartDate");
        QTest::addColumn<QDateTime>("updatedDueDate");
        QTest::addColumn<QString>("updatedRelated");

        QTest::newRow("no change") << "summary" << "content" << false << QDateTime(QDate(2013, 11, 24)) << QDateTime(QDate(2014, 03, 01)) << "my-uid";
        QTest::newRow("changed") << "new summary" << "new content" << true << QDateTime(QDate(2013, 11, 25)) << QDateTime(QDate(2014, 03, 02)) << "my-new-uid";
    }

    void shouldUpdateTaskFromItem()
    {
        // GIVEN

        // A todo...
        KCalCore::Todo::Ptr originalTodo(new KCalCore::Todo);
        originalTodo->setSummary("summary");
        originalTodo->setDescription("content");
        originalTodo->setCompleted(false);
        originalTodo->setDtStart(KDateTime(QDate(2013, 11, 24)));
        originalTodo->setDtDue(KDateTime(QDate(2014, 03, 01)));
        originalTodo->setRelatedTo("my-uid");

        // ... as payload of an item...
        Akonadi::Item originalItem;
        originalItem.setMimeType("application/x-vnd.akonadi.calendar.todo");
        originalItem.setPayload<KCalCore::Todo::Ptr>(originalTodo);

        // ... deserialized as a task
        Akonadi::Serializer serializer;
        auto task = serializer.createTaskFromItem(originalItem);

        // WHEN

        // Data...
        QFETCH(QString, updatedSummary);
        QFETCH(QString, updatedContent);
        QFETCH(bool, updatedDone);
        QFETCH(QDateTime, updatedStartDate);
        QFETCH(QDateTime, updatedDueDate);
        QFETCH(QString, updatedRelated);

        // ... in a new todo...
        KCalCore::Todo::Ptr updatedTodo(new KCalCore::Todo);
        updatedTodo->setSummary(updatedSummary);
        updatedTodo->setDescription(updatedContent);
        updatedTodo->setCompleted(updatedDone);
        updatedTodo->setDtStart(KDateTime(updatedStartDate));
        updatedTodo->setDtDue(KDateTime(updatedDueDate));
        updatedTodo->setRelatedTo(updatedRelated);

        // ... as payload of a new item
        Akonadi::Item updatedItem;
        updatedItem.setMimeType("application/x-vnd.akonadi.calendar.todo");
        updatedItem.setPayload<KCalCore::Todo::Ptr>(updatedTodo);

        serializer.updateTaskFromItem(task, updatedItem);

        // THEN
        QCOMPARE(task->title(), updatedSummary);
        QCOMPARE(task->text(), updatedContent);
        QCOMPARE(task->isDone(), updatedDone);
        QCOMPARE(task->startDate(), updatedStartDate);
        QCOMPARE(task->dueDate(), updatedDueDate);
        QCOMPARE(task->property("todoUid").toString(), updatedTodo->uid());
        QCOMPARE(task->property("relatedUid").toString(), updatedTodo->relatedTo());
        QCOMPARE(task->property("itemId").toLongLong(), updatedItem.id());
    }

    void shouldNotUpdateTaskFromInvalidItem()
    {
        // GIVEN

        // Data...
        const QString summary = "summary";
        const QString content = "content";
        const bool isDone = true;
        const QDateTime startDate(QDate(2013, 11, 24));
        const QDateTime dueDate(QDate(2014, 03, 01));

        // ... stored in a todo...
        KCalCore::Todo::Ptr originalTodo(new KCalCore::Todo);
        originalTodo->setSummary(summary);
        originalTodo->setDescription(content);
        originalTodo->setCompleted(isDone);
        originalTodo->setDtStart(KDateTime(startDate));
        originalTodo->setDtDue(KDateTime(dueDate));

        // ... as payload of an item...
        Akonadi::Item originalItem;
        originalItem.setMimeType("application/x-vnd.akonadi.calendar.todo");
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
        QCOMPARE(task->startDate(), startDate);
        QCOMPARE(task->dueDate(), dueDate);
        QCOMPARE(task->property("itemId").toLongLong(), originalItem.id());
    }

    void shouldNotUpdateTaskFromProjectItem()
    {
        // GIVEN

        // Data...
        const QString summary = "summary";
        const QString content = "content";
        const bool isDone = true;
        const QDateTime startDate(QDate(2013, 11, 24));
        const QDateTime dueDate(QDate(2014, 03, 01));

        // ... stored in a todo...
        KCalCore::Todo::Ptr originalTodo(new KCalCore::Todo);
        originalTodo->setSummary(summary);
        originalTodo->setDescription(content);
        originalTodo->setCompleted(isDone);
        originalTodo->setDtStart(KDateTime(startDate));
        originalTodo->setDtDue(KDateTime(dueDate));

        // ... as payload of an item...
        Akonadi::Item originalItem;
        originalItem.setMimeType("application/x-vnd.akonadi.calendar.todo");
        originalItem.setPayload<KCalCore::Todo::Ptr>(originalTodo);

        // ... deserialized as a task
        Akonadi::Serializer serializer;
        auto task = serializer.createTaskFromItem(originalItem);

        // WHEN
        // A todo with the project flag
        KCalCore::Todo::Ptr projectTodo(new KCalCore::Todo);
        projectTodo->setSummary("foo");
        projectTodo->setCustomProperty("Zanshin", "Project", "1");

        // ... as payload of an item
        Akonadi::Item projectItem;
        projectItem.setMimeType("application/x-vnd.akonadi.calendar.todo");
        projectItem.setPayload<KCalCore::Todo::Ptr>(projectTodo);
        serializer.updateTaskFromItem(task, projectItem);

        // THEN
        QCOMPARE(task->title(), summary);
        QCOMPARE(task->text(), content);
        QCOMPARE(task->isDone(), isDone);
        QCOMPARE(task->startDate(), startDate);
        QCOMPARE(task->dueDate(), dueDate);
        QCOMPARE(task->property("itemId").toLongLong(), originalItem.id());
    }

    void shouldCreateItemFromTask_data()
    {
        QTest::addColumn<QString>("summary");
        QTest::addColumn<QString>("content");
        QTest::addColumn<bool>("isDone");
        QTest::addColumn<QDateTime>("startDate");
        QTest::addColumn<QDateTime>("dueDate");
        QTest::addColumn<qint64>("itemId");
        QTest::addColumn<QString>("todoUid");

        QTest::newRow("nominal case (no id)") << "summary" << "content" << false
                                              << QDateTime(QDate(2013, 11, 24)) << QDateTime(QDate(2014, 03, 01))
                                              << qint64(-1) << QString();
        QTest::newRow("done case (no id)") << "summary" << "content" << true
                                           << QDateTime(QDate(2013, 11, 24)) << QDateTime(QDate(2014, 03, 01))
                                           << qint64(-1) << QString();
        QTest::newRow("empty case (no id)") << QString() << QString() << false
                                            << QDateTime() << QDateTime()
                                            << qint64(-1) << QString();

        QTest::newRow("nominal case (with id)") << "summary" << "content" << false
                                                << QDateTime(QDate(2013, 11, 24)) << QDateTime(QDate(2014, 03, 01))
                                                << qint64(42) << "my-uid";
        QTest::newRow("done case (with id)") << "summary" << "content" << true
                                             << QDateTime(QDate(2013, 11, 24)) << QDateTime(QDate(2014, 03, 01))
                                             << qint64(42) << "my-uid";
        QTest::newRow("empty case (with id)") << QString() << QString() << false
                                              << QDateTime() << QDateTime()
                                              << qint64(42) << "my-uid";
    }

    void shouldCreateItemFromTask()
    {
        // GIVEN

        // Data...
        QFETCH(QString, summary);
        QFETCH(QString, content);
        QFETCH(bool, isDone);
        QFETCH(QDateTime, startDate);
        QFETCH(QDateTime, dueDate);
        QFETCH(qint64, itemId);
        QFETCH(QString, todoUid);

        // ... stored in a task
        auto task = Domain::Task::Ptr::create();
        task->setTitle(summary);
        task->setText(content);
        task->setDone(isDone);
        task->setStartDate(startDate);
        task->setDueDate(dueDate);

        if (itemId > 0)
            task->setProperty("itemId", itemId);

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

        auto todo = item.payload<KCalCore::Todo::Ptr>();
        QCOMPARE(todo->summary(), summary);
        QCOMPARE(todo->description(), content);
        QCOMPARE(todo->isCompleted(), isDone);
        QCOMPARE(todo->dtStart().dateTime(), startDate);
        QCOMPARE(todo->dtDue().dateTime(), dueDate);

        if (!todoUid.isEmpty()) {
            QCOMPARE(todo->uid(), todoUid);
        }

        QCOMPARE(todo->relatedTo(), QString("parent-uid"));
    }

    void shouldVerifyIfAnItemIsATaskChild_data()
    {
        QTest::addColumn<Domain::Task::Ptr>("task");
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<bool>("isParent");

        // Create task
        const QString summary = "summary";
        const QString content = "content";
        const bool isDone = true;
        const QDateTime startDate(QDate(2013, 11, 24));
        const QDateTime dueDate(QDate(2014, 03, 01));

        // ... create a task
        Domain::Task::Ptr task(new Domain::Task);
        task->setTitle(summary);
        task->setText(content);
        task->setDone(isDone);
        task->setStartDate(startDate);
        task->setDueDate(dueDate);
        task->setProperty("todoUid", "1");

        // Create Child item
        KCalCore::Todo::Ptr childTodo(new KCalCore::Todo);
        childTodo->setSummary(summary);
        childTodo->setDescription(content);
        childTodo->setCompleted(isDone);
        childTodo->setDtStart(KDateTime(startDate));
        childTodo->setDtDue(KDateTime(dueDate));

        Akonadi::Item childItem;
        childItem.setMimeType("application/x-vnd.akonadi.calendar.todo");
        childItem.setPayload<KCalCore::Todo::Ptr>(childTodo);

        QTest::newRow("without parent") << task << childItem << false;

        // Create Child Item with parent
        KCalCore::Todo::Ptr childTodo2(new KCalCore::Todo);
        childTodo2->setSummary(summary);
        childTodo2->setDescription(content);
        childTodo2->setCompleted(isDone);
        childTodo2->setDtStart(KDateTime(startDate));
        childTodo2->setDtDue(KDateTime(dueDate));
        childTodo2->setRelatedTo("1");

        Akonadi::Item childItem2;
        childItem2.setMimeType("application/x-vnd.akonadi.calendar.todo");
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
        todo2->setRelatedTo("1");
        item2.setPayload<KCalCore::Todo::Ptr>(todo2);

        Akonadi::Item item3;
        KMime::Message::Ptr message1(new KMime::Message);
        message1->subject(true)->fromUnicodeString("foo", "utf-8");
        message1->mainBodyPart()->fromUnicodeString("bar");
        item3.setMimeType(Akonadi::NoteUtils::noteMimeType());
        item3.setPayload<KMime::Message::Ptr>(message1);

        Akonadi::Item item4;
        KMime::Message::Ptr message2(new KMime::Message);
        message2->subject(true)->fromUnicodeString("foo", "utf-8");
        message2->mainBodyPart()->fromUnicodeString("bar");
        auto relatedHeader1 = new KMime::Headers::Generic("X-Zanshin-RelatedProjectUid");
        relatedHeader1->from7BitString("1");
        message2->appendHeader(relatedHeader1);
        item4.setMimeType(Akonadi::NoteUtils::noteMimeType());
        item4.setPayload<KMime::Message::Ptr>(message2);

        Akonadi::Item item5;
        KMime::Message::Ptr message3(new KMime::Message);
        message3->subject(true)->fromUnicodeString("foo", "utf-8");
        message3->mainBodyPart()->fromUnicodeString("bar");
        auto relatedHeader2 = new KMime::Headers::Generic("X-Zanshin-RelatedProjectUid");
        message3->appendHeader(relatedHeader2);
        item5.setMimeType(Akonadi::NoteUtils::noteMimeType());
        item5.setPayload<KMime::Message::Ptr>(message3);

        QTest::newRow("task without related") << item1 << QString();
        QTest::newRow("task with related") << item2 << "1";
        QTest::newRow("note without related") << item3 << QString();
        QTest::newRow("note with related") << item4 << "1";
        QTest::newRow("note with empty related") << item5 << QString();
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
        QTest::newRow("empty case") << QString() << QString() << QString();
    }

    void shouldCreateNoteFromItem()
    {
        // GIVEN

        // Data...
        QFETCH(QString, title);
        QFETCH(QString, text);
        QFETCH(QString, relatedUid);

        // ... stored in a message...
        KMime::Message::Ptr message(new KMime::Message);
        message->subject(true)->fromUnicodeString(title, "utf-8");
        message->mainBodyPart()->fromUnicodeString(text);

        if (!relatedUid.isEmpty()) {
            auto relatedHeader = new KMime::Headers::Generic("X-Zanshin-RelatedProjectUid");
            relatedHeader->from7BitString(relatedUid.toUtf8());
            message->appendHeader(relatedHeader);
        }

        // ... as payload of an item.
        Akonadi::Item item;
        item.setMimeType(Akonadi::NoteUtils::noteMimeType());
        item.setPayload<KMime::Message::Ptr>(message);

        // WHEN
        Akonadi::Serializer serializer;
        Domain::Note::Ptr note = serializer.createNoteFromItem(item);

        // THEN
        QCOMPARE(note->title(), title);
        QCOMPARE(note->text(), text);
        QCOMPARE(note->property("itemId").toLongLong(), item.id());
        QCOMPARE(note->property("relatedUid").toString(), relatedUid);
    }

    void shouldCreateNullNoteFromInvalidItem()
    {
        // GIVEN
        Akonadi::Item item;

        // WHEN
        Akonadi::Serializer serializer;
        Domain::Note::Ptr note = serializer.createNoteFromItem(item);

        // THEN
        QVERIFY(note.isNull());
    }

    void shouldUpdateNoteFromItem_data()
    {
        QTest::addColumn<QString>("updatedTitle");
        QTest::addColumn<QString>("updatedText");
        QTest::addColumn<QString>("updatedRelatedUid");

        QTest::newRow("no change") << "title" << "content" << "parent-uid";
        QTest::newRow("data changed (with related)") << "A new title" << "A new content" << "new-parent-uid";
        QTest::newRow("data changed (with no related)") << "A new title" << "A new content" << QString();
    }

    void shouldUpdateNoteFromItem()
    {
        // GIVEN

        // A message...
        KMime::Message::Ptr message(new KMime::Message);
        message->subject(true)->fromUnicodeString("title", "utf-8");
        message->mainBodyPart()->fromUnicodeString("text");
        auto relatedHeader = new KMime::Headers::Generic("X-Zanshin-RelatedProjectUid");
        relatedHeader->from7BitString("parent-uid");
        message->appendHeader(relatedHeader);

        //... as the payload of an item...
        Akonadi::Item item;
        item.setMimeType(Akonadi::NoteUtils::noteMimeType());
        item.setPayload<KMime::Message::Ptr>(message);

        //... deserialized as a note
        Akonadi::Serializer serializer;
        auto note = serializer.createNoteFromItem(item);

        // WHEN

        // Data...
        QFETCH(QString, updatedTitle);
        QFETCH(QString, updatedText);
        QFETCH(QString, updatedRelatedUid);

        //... stored in a new message...
        KMime::Message::Ptr updatedMessage(new KMime::Message);
        updatedMessage->subject(true)->fromUnicodeString(updatedTitle, "utf-8");
        updatedMessage->mainBodyPart()->fromUnicodeString(updatedText);

        if (!updatedRelatedUid.isEmpty()) {
            relatedHeader = new KMime::Headers::Generic("X-Zanshin-RelatedProjectUid");
            relatedHeader->from7BitString(updatedRelatedUid.toUtf8());
            updatedMessage->appendHeader(relatedHeader);
        }

        //... as the payload of a new item...
        Akonadi::Item updatedItem;
        updatedItem.setMimeType(Akonadi::NoteUtils::noteMimeType());
        updatedItem.setPayload<KMime::Message::Ptr>(updatedMessage);

        serializer.updateNoteFromItem(note, updatedItem);

        // THEN
        QCOMPARE(note->title(), updatedTitle);
        QCOMPARE(note->text(), updatedText);
        QCOMPARE(note->property("itemId").toLongLong(), updatedItem.id());
        QCOMPARE(note->property("relatedUid").toString(), updatedRelatedUid);
    }

    void shouldNotUpdateNoteFromInvalidItem()
    {
        // GIVEN

        // Data...
        QString title = "A title";
        QString text = "A note content";

        // ... stored in a message...
        KMime::Message::Ptr message(new KMime::Message);
        message->subject(true)->fromUnicodeString(title, "utf-8");
        message->mainBodyPart()->fromUnicodeString(text);

        //... as the payload of an item...
        Akonadi::Item item;
        item.setMimeType(Akonadi::NoteUtils::noteMimeType());
        item.setPayload<KMime::Message::Ptr>(message);

        //... deserialized as a note
        Akonadi::Serializer serializer;
        auto note = serializer.createNoteFromItem(item);

        // WHEN
        Akonadi::Item invalidItem;

        serializer.updateNoteFromItem(note, invalidItem);

        //THEN
        QCOMPARE(note->title(), title);
        QCOMPARE(note->text(), text);
        QCOMPARE(note->property("itemId").toLongLong(), item.id());
    }

    void shouldCreateItemFromNote_data()
    {
        QTest::addColumn<QString>("title");
        QTest::addColumn<QString>("content");
        QTest::addColumn<QString>("expectedTitle");
        QTest::addColumn<QString>("expectedContent");
        QTest::addColumn<qint64>("itemId");
        QTest::addColumn<QString>("relatedUid");

        QTest::newRow("nominal case (no id)") << "title" << "content" << "title" << "content" << qint64(-1) << QString();
        QTest::newRow("empty case (no id)") << QString() << QString() << "New Note" << QString() << qint64(-1) << QString();

        QTest::newRow("nominal case (with id)") << "title" << "content" << "title" << "content" << qint64(42) << "parent-uid";
        QTest::newRow("empty case (with id)") << QString() << QString() << "New Note" << QString() << qint64(42) << "parent-uid";
    }

    void shouldCreateItemFromNote()
    {
        // GIVEN

        // Data...
        QFETCH(QString, title);
        QFETCH(QString, content);
        QFETCH(qint64, itemId);
        QFETCH(QString, relatedUid);

        // ... stored in a note
        auto note = Domain::Note::Ptr::create();
        note->setTitle(title);
        note->setText(content);

        if (itemId > 0)
            note->setProperty("itemId", itemId);

        if (!relatedUid.isEmpty())
            note->setProperty("relatedUid", relatedUid);

        // WHEN
        Akonadi::Serializer serializer;
        auto item = serializer.createItemFromNote(note);

        // THEN
        QCOMPARE(item.mimeType(), Akonadi::NoteUtils::noteMimeType());

        QCOMPARE(item.isValid(), itemId > 0);
        if (itemId > 0) {
            QCOMPARE(item.id(), itemId);
        }

        QFETCH(QString, expectedTitle);
        QFETCH(QString, expectedContent);
        auto message = item.payload<KMime::Message::Ptr>();
        QCOMPARE(message->subject(false)->asUnicodeString(), expectedTitle);
        QCOMPARE(message->mainBodyPart()->decodedText(true), expectedContent);

        if (relatedUid.isEmpty()) {
            QVERIFY(!message->headerByType("X-Zanshin-RelatedProjectUid"));
        } else {
            QVERIFY(message->headerByType("X-Zanshin-RelatedProjectUid"));
            QCOMPARE(message->headerByType("X-Zanshin-RelatedProjectUid")->asUnicodeString(), relatedUid);
        }
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
        todo->setCustomProperty("Zanshin", "Project", "1");
        QVERIFY(!todo->uid().isEmpty());

        // ... as payload of an item
        Akonadi::Item item(42);
        item.setMimeType("application/x-vnd.akonadi.calendar.todo");
        item.setPayload<KCalCore::Todo::Ptr>(todo);

        // which has a prent collection
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
        todo->setSummary("foo");

        // ... as payload of an item
        Akonadi::Item item;
        item.setMimeType("application/x-vnd.akonadi.calendar.todo");
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
        originalTodo->setSummary("summary");
        originalTodo->setCustomProperty("Zanshin", "Project", "1");

        // ... as payload of an item...
        Akonadi::Item originalItem(42);
        originalItem.setMimeType("application/x-vnd.akonadi.calendar.todo");
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
        updatedTodo->setCustomProperty("Zanshin", "Project", "1");
        QVERIFY(!updatedTodo->uid().isEmpty());

        // ... as payload of a new item
        Akonadi::Item updatedItem(44);
        updatedItem.setMimeType("application/x-vnd.akonadi.calendar.todo");
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
        const QString summary = "summary";

        // ... stored in a todo...
        KCalCore::Todo::Ptr originalTodo(new KCalCore::Todo);
        originalTodo->setSummary(summary);
        originalTodo->setCustomProperty("Zanshin", "Project", "1");

        // ... as payload of an item...
        Akonadi::Item originalItem;
        originalItem.setMimeType("application/x-vnd.akonadi.calendar.todo");
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
        const QString summary = "summary";

        // ... stored in a todo...
        KCalCore::Todo::Ptr originalTodo(new KCalCore::Todo);
        originalTodo->setSummary(summary);
        originalTodo->setCustomProperty("Zanshin", "Project", "1");

        // ... as payload of an item...
        Akonadi::Item originalItem;
        originalItem.setMimeType("application/x-vnd.akonadi.calendar.todo");
        originalItem.setPayload<KCalCore::Todo::Ptr>(originalTodo);

        // ... deserialized as a project
        Akonadi::Serializer serializer;
        auto project = serializer.createProjectFromItem(originalItem);

        // WHEN
        // A todo without the project flag
        KCalCore::Todo::Ptr projectTodo(new KCalCore::Todo);
        projectTodo->setSummary("foo");

        // ... as payload of an item
        Akonadi::Item projectItem;
        projectItem.setMimeType("application/x-vnd.akonadi.calendar.todo");
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
        const QString todoUid = "test-uid";

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
        project->setName("project");
        project->setProperty("todoUid", "1");

        // Create unrelated todo
        auto unrelatedTodo = KCalCore::Todo::Ptr::create();
        unrelatedTodo->setSummary("summary");
        Akonadi::Item unrelatedTodoItem;
        unrelatedTodoItem.setMimeType("application/x-vnd.akonadi.calendar.todo");
        unrelatedTodoItem.setPayload<KCalCore::Todo::Ptr>(unrelatedTodo);

        QTest::newRow("unrelated todo") << project << unrelatedTodoItem << false;

        // Create child todo
        auto childTodo = KCalCore::Todo::Ptr::create();
        childTodo->setSummary("summary");
        childTodo->setRelatedTo("1");
        Akonadi::Item childTodoItem;
        childTodoItem.setMimeType("application/x-vnd.akonadi.calendar.todo");
        childTodoItem.setPayload<KCalCore::Todo::Ptr>(childTodo);

        QTest::newRow("child todo") << project << childTodoItem << true;

        // Create unrelated note
        KMime::Message::Ptr unrelatedNote(new KMime::Message);
        unrelatedNote->subject(true)->fromUnicodeString("subject", "utf-8");
        Akonadi::Item unrelatedNoteItem;
        unrelatedNoteItem.setMimeType(Akonadi::NoteUtils::noteMimeType());
        unrelatedNoteItem.setPayload<KMime::Message::Ptr>(unrelatedNote);

        QTest::newRow("unrelated note") << project << unrelatedNoteItem << false;

        // Create child note
        KMime::Message::Ptr childNote(new KMime::Message);
        childNote->subject(true)->fromUnicodeString("subject", "utf-8");
        auto relatedHeader = new KMime::Headers::Generic("X-Zanshin-RelatedProjectUid");
        relatedHeader->from7BitString("1");
        childNote->appendHeader(relatedHeader);
        Akonadi::Item childNoteItem;
        childNoteItem.setMimeType(Akonadi::NoteUtils::noteMimeType());
        childNoteItem.setPayload<KMime::Message::Ptr>(childNote);

        QTest::newRow("child todo") << project << childNoteItem << true;

        auto invalidProject = Domain::Project::Ptr::create();
        QTest::newRow("invalid project") << invalidProject << unrelatedNoteItem << false;

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

        Akonadi::Item noteItem;
        KMime::Message::Ptr note(new KMime::Message);
        noteItem.setPayload<KMime::Message::Ptr>(note);

        QTest::newRow("nominal note case") << noteItem << parent << "1";
        QTest::newRow("update note item with a empty parent uid") << noteItem << invalidParent << QString();

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
        } else if (item.hasPayload<KMime::Message::Ptr>()) {
            auto note = item.payload<KMime::Message::Ptr>();
            const auto relatedHeader = note->headerByType("X-Zanshin-RelatedProjectUid");
            const QString relatedUid = relatedHeader ? relatedHeader->asUnicodeString() : QString();
            QCOMPARE(relatedUid, expectedRelatedToUid);
            if (!expectedRelatedToUid.isEmpty())
                QVERIFY(note->encodedContent().contains(QString("X-Zanshin-RelatedProjectUid: %1").arg(expectedRelatedToUid).toUtf8()));
            else
                QVERIFY(!note->encodedContent().contains("X-Zanshin-RelatedProjectUid:"));
        }
    }

    void shouldFilterChildrenItem_data()
    {
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<Akonadi::Item::List>("items");
        QTest::addColumn<int>("size");

        Akonadi::Item item(12);
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setUid("1");
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
        todo3->setUid("3");
        todo3->setRelatedTo("1");
        item3.setPayload<KCalCore::Todo::Ptr>(todo3);
        Akonadi::Item::List items3;
        items3 << item2 << item3;

        QTest::newRow("list with child") << item << items3 << 1;

        Akonadi::Item item4(15);
        KCalCore::Todo::Ptr todo4(new KCalCore::Todo);
        todo4->setRelatedTo("3");
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
        todo->setRelatedTo("3");
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

    void shouldCreateContextFromTag_data()
    {
        QTest::addColumn<QByteArray>("type");
        QTest::addColumn<QString>("name");
        QTest::addColumn<Akonadi::Tag::Id>("tagId");

        const QByteArray rightTagType = Akonadi::Serializer::contextTagType() ;

        QTest::newRow("nominal case") << rightTagType << "Context42" << Akonadi::Tag::Id(42);
        QTest::newRow("empty name case") << rightTagType << "" << Akonadi::Tag::Id(43);
    }

    void shouldCreateContextFromTag()
    {
        // GIVEN

        // Data...
        QFETCH(QByteArray, type);
        QFETCH(QString, name);
        QFETCH(Akonadi::Tag::Id, tagId);

        // ... stored as an Akonadi Tag
        Akonadi::Tag tag(name);
        tag.setType(type);
        tag.setId(tagId);

        // WHEN
        Akonadi::Serializer serializer;
        Domain::Context::Ptr context = serializer.createContextFromTag(tag);

        // THEN
        QCOMPARE(context->name(), tag.name());
        QCOMPARE(context->property("tagId").toLongLong(), tag.id());
    }

    void shouldNotCreateContextFromWrongTagType()
    {
        // GIVEN

        // Data stored as an Akonadi Tag
        Akonadi::Tag tag("context42");
        tag.setType(QByteArray("wrongTagType"));

        // WHEN
        Akonadi::Serializer serializer;
        Domain::Context::Ptr context = serializer.createContextFromTag(tag);

        // THEN
        QVERIFY(!context);
    }

    void shouldUpdateContextFromTag_data()
    {
        shouldCreateContextFromTag_data();
    }

    void shouldUpdateContextFromTag()
    {
        // GIVEN

        // Data...
        QFETCH(QByteArray, type);
        QFETCH(QString, name);
        QFETCH(Akonadi::Tag::Id, tagId);

        // ... stored as an Akonadi Tag
        Akonadi::Tag tag(name);
        tag.setType(type);
        tag.setId(tagId);

        // WHEN
        Akonadi::Serializer serializer;
        Domain::Context::Ptr context(new Domain::Context);

        serializer.updateContextFromTag(context, tag);

        // THEN
        QCOMPARE(context->name(), tag.name());
        QCOMPARE(context->property("tagId").toLongLong(), tag.id());
    }

    void shouldNotUpdateContextFromWrongTagType()
    {
        // GIVEN

        Akonadi::Tag originalTag("Context42");
        originalTag.setType(Akonadi::Serializer::contextTagType());
        originalTag.setId(42);

        Akonadi::Serializer serializer;
        Domain::Context::Ptr context = serializer.createContextFromTag(originalTag);

        // WHEN
        Akonadi::Tag wrongTag("WrongContext42");
        wrongTag.setType(QByteArray("wrongTypeTag"));
        serializer.updateContextFromTag(context, wrongTag);

        // THEN
        QCOMPARE(context->name(), originalTag.name());
        QCOMPARE(context->property("tagId").toLongLong(), originalTag.id());
    }

    void shouldVerifyIfAnItemIsAContextChild_data()
    {
        QTest::addColumn<Domain::Context::Ptr>("context");
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<bool>("isChild");

        // Create a context
        auto context = Domain::Context::Ptr::create();
        context->setProperty("tagId", qint64(43));
        Akonadi::Tag tag(Akonadi::Tag::Id(43));

        Akonadi::Item unrelatedItem;
        QTest::newRow("Unrelated item") << context << unrelatedItem << false;

        Akonadi::Item relatedItem;
        relatedItem.setTag(tag);
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

    void shouldCheckIfAnItemHasContextsOrTags_data()
    {
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<bool>("contextsExpected");
        QTest::addColumn<bool>("tagsExpected");

        Akonadi::Tag unrelatedTag("Foo");
        unrelatedTag.setType("unrelated");
        Akonadi::Tag contextTag("Bar");
        contextTag.setType(Akonadi::Serializer::contextTagType());
        Akonadi::Tag akonadiTag("Baz");
        akonadiTag.setType(Akonadi::Tag::PLAIN);

        Akonadi::Item item;
        QTest::newRow("no tags") << item << false << false;

        item.setTags({ unrelatedTag });
        QTest::newRow("unrelated tags") << item << false << false;

        item.setTags({ unrelatedTag, contextTag });
        QTest::newRow("has contexts") << item << true << false;

        item.setTags({ unrelatedTag, akonadiTag });
        QTest::newRow("has tags") << item << false << true;

        item.setTags({ unrelatedTag, contextTag, akonadiTag });
        QTest::newRow("has both") << item << true << true;
    }

    void shouldCheckIfAnItemHasContextsOrTags()
    {
        // GIVEN
        QFETCH(Akonadi::Item, item);
        QFETCH(bool, contextsExpected);
        QFETCH(bool, tagsExpected);

        Akonadi::Serializer serializer;

        // WHEN
        const bool hasContexts = serializer.hasContextTags(item);
        const bool hasTags = serializer.hasAkonadiTags(item);

        // THEN
        QCOMPARE(hasContexts, contextsExpected);
        QCOMPARE(hasTags, tagsExpected);
    }

    void shouldCreateTagFromContext_data()
    {

        QTest::addColumn<QString>("name");
        QTest::addColumn<qint64>("tagId");
        QTest::addColumn<QByteArray>("tagGid");

        QString nameInternet = "Internet";

        QTest::newRow("nominal case") << QString(nameInternet) << qint64(42) << nameInternet.toLatin1();
        QTest::newRow("null name case") << QString() << qint64(42) << QByteArray();
        QTest::newRow("null tagId case") << QString(nameInternet)<< qint64(-1) << nameInternet.toLatin1();
        QTest::newRow("totally null context case") << QString() << qint64(-1) << QByteArray();
    }

    void shouldCreateTagFromContext()
    {
        // GIVEN
        QFETCH(QString, name);
        QFETCH(qint64, tagId);
        QFETCH(QByteArray, tagGid);

        // WHEN
        auto context = Domain::Context::Ptr::create();
        context->setProperty("tagId", tagId);
        context->setName(name);

        Akonadi::Serializer serializer;
        Akonadi::Tag tag = serializer.createTagFromContext(context);

        // THEN
        QCOMPARE(tag.name(), name);
        QCOMPARE(tag.isValid(), tagId > 0);

        if (tagId > 0) {
            QCOMPARE(tag.id(), tagId);
            QCOMPARE(tag.gid(), tagGid);
            QCOMPARE(tag.type(), Akonadi::SerializerInterface::contextTagType());
        }
    }
};

QTEST_MAIN(AkonadiSerializerTest)

#include "akonadiserializertest.moc"
