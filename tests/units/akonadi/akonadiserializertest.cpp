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

#include <QtTest>

#include "akonadi/akonadiserializer.h"

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
    void shouldCreateDataSourceFromCollection_data()
    {
        QTest::addColumn<QString>("name");
        QTest::addColumn<QString>("iconName");

        QTest::newRow("nominal case") << "name" << "icon";
        QTest::newRow("empty case") << QString() << QString();
    }

    void shouldCreateDataSourceFromCollection()
    {
        // GIVEN

        // Data...
        QFETCH(QString, name);
        QFETCH(QString, iconName);

        // ... stored in a collection
        Akonadi::Collection collection(42);
        collection.setName(name);
        auto attribute = new Akonadi::EntityDisplayAttribute;
        attribute->setIconName(iconName);
        collection.addAttribute(attribute);

        // WHEN
        Akonadi::Serializer serializer;
        auto dataSource = serializer.createDataSourceFromCollection(collection);

        // THEN
        QCOMPARE(dataSource->name(), name);
        QCOMPARE(dataSource->iconName(), iconName);
    }

    void shouldCreateNullDataSourceFromInvalidCollection()
    {
        // GIVEN
        Akonadi::Collection collection;

        // WHEN
        Akonadi::Serializer serializer;
        auto dataSource = serializer.createDataSourceFromCollection(collection);

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
        auto dataSource = serializer.createDataSourceFromCollection(originalCollection);

        // WHEN

        // Data...
        QFETCH(QString, updatedName);

        // ... in a new collection
        Akonadi::Collection updatedCollection(42);
        updatedCollection.setName(updatedName);

        serializer.updateDataSourceFromCollection(dataSource, updatedCollection);

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
        auto dataSource = serializer.createDataSourceFromCollection(originalCollection);

        // WHEN
        Akonadi::Collection invalidCollection;
        invalidCollection.setName("foo");
        serializer.updateDataSourceFromCollection(dataSource, invalidCollection);

        // THEN
        QCOMPARE(dataSource->name(), name);
    }

    void shouldNameDataSourceFromCollectionPath()
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
        auto dataSource1 = serializer.createDataSourceFromCollection(collection);

        // Give it another try with the root
        parentCollection.setParentCollection(Akonadi::Collection::root());
        collection.setParentCollection(parentCollection);
        auto dataSource2 = serializer.createDataSourceFromCollection(collection);

        // THEN
        QCOMPARE(dataSource1->name(), QString(parentName + "/" + name));
        QCOMPARE(dataSource2->name(), QString(parentName + "/" + name));
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

    void shouldUpdateTaskFromItem_data()
    {
        QTest::addColumn<QString>("updatedSummary");
        QTest::addColumn<QString>("updatedContent");
        QTest::addColumn<bool>("updatedDone");
        QTest::addColumn<QDateTime>("updatedStartDate");
        QTest::addColumn<QDateTime>("updatedDueDate");

        QTest::newRow("no change") << "summary" << "content" << false << QDateTime(QDate(2013, 11, 24)) << QDateTime(QDate(2014, 03, 01));
        QTest::newRow("changed") << "new summary" << "new content" << true << QDateTime(QDate(2013, 11, 25)) << QDateTime(QDate(2014, 03, 02));
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

        // ... in a new todo...
        KCalCore::Todo::Ptr updatedTodo(new KCalCore::Todo);
        updatedTodo->setSummary(updatedSummary);
        updatedTodo->setDescription(updatedContent);
        updatedTodo->setCompleted(updatedDone);
        updatedTodo->setDtStart(KDateTime(updatedStartDate));
        updatedTodo->setDtDue(KDateTime(updatedDueDate));

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
    }

    void shouldCreateItemFromTask_data()
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

    void shouldCreateItemFromTask()
    {
        // GIVEN

        // Data...
        QFETCH(QString, summary);
        QFETCH(QString, content);
        QFETCH(bool, isDone);
        QFETCH(QDateTime, startDate);
        QFETCH(QDateTime, dueDate);

        // ... stored in a task
        auto task = Domain::Task::Ptr::create();
        task->setTitle(summary);
        task->setText(content);
        task->setDone(isDone);
        task->setStartDate(startDate);
        task->setDueDate(dueDate);

        // WHEN
        Akonadi::Serializer serializer;
        auto item = serializer.createItemFromTask(task);

        // THEN
        QCOMPARE(item.mimeType(), KCalCore::Todo::todoMimeType());

        auto todo = item.payload<KCalCore::Todo::Ptr>();
        QCOMPARE(todo->summary(), summary);
        QCOMPARE(todo->description(), content);
        QCOMPARE(todo->isCompleted(), isDone);
        QCOMPARE(todo->dtStart().dateTime(), startDate);
        QCOMPARE(todo->dtDue().dateTime(), dueDate);
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

        QTest::newRow("without related") << item1 << QString();
        QTest::newRow("with related") << item2 << "1";
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

        QTest::newRow("nominal case") << "A note title" << "A note content.\nWith two lines.";
        QTest::newRow("empty case") << QString() << QString();
    }

    void shouldCreateNoteFromItem()
    {
        // GIVEN

        // Data...
        QFETCH(QString, title);
        QFETCH(QString, text);

        // ... stored in a message...
        KMime::Message::Ptr message(new KMime::Message);
        message->subject(true)->fromUnicodeString(title, "utf-8");
        message->mainBodyPart()->fromUnicodeString(text);

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

        QTest::newRow("no change") << "title" << "content";
        QTest::newRow("data changed") << "A new title" << "A new content";
    }

    void shouldUpdateNoteFromItem()
    {
        // GIVEN

        // A message...
        KMime::Message::Ptr message(new KMime::Message);
        message->subject(true)->fromUnicodeString("title", "utf-8");
        message->mainBodyPart()->fromUnicodeString("text");

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

        //... stored in a new message...
        KMime::Message::Ptr updatedMessage(new KMime::Message);
        updatedMessage->subject(true)->fromUnicodeString(updatedTitle, "utf-8");
        updatedMessage->mainBodyPart()->fromUnicodeString(updatedText);

        //... as the payload of a new item...
        Akonadi::Item updatedItem;
        updatedItem.setMimeType(Akonadi::NoteUtils::noteMimeType());
        updatedItem.setPayload<KMime::Message::Ptr>(updatedMessage);

        serializer.updateNoteFromItem(note, updatedItem);

        // THEN
        QCOMPARE(note->title(), updatedTitle);
        QCOMPARE(note->text(), updatedText);
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
    }
};

QTEST_MAIN(AkonadiSerializerTest)

#include "akonadiserializertest.moc"
