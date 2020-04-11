/* This file is part of Zanshin

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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

#include "akonadi/akonadilivequeryhelpers.h"

#include <functional>

#include <KCalCore/Todo>

#include "akonadi/akonadiserializer.h"

#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/gentodo.h"
#include "testlib/testhelpers.h"

using namespace Testlib;

using namespace std::placeholders;

static QString titleFromItem(const Akonadi::Item &item)
{
    if (item.hasPayload<KCalCore::Todo::Ptr>()) {
        const auto todo = item.payload<KCalCore::Todo::Ptr>();
        return todo->summary();
    } else {
        return QString();
    }
}

class AkonadiLiveQueryHelpersTest : public QObject
{
    Q_OBJECT

private:
    Akonadi::LiveQueryHelpers::Ptr createHelpers(AkonadiFakeData &data)
    {
        return Akonadi::LiveQueryHelpers::Ptr(new Akonadi::LiveQueryHelpers(createSerializer(), createStorage(data)));
    }

    Akonadi::StorageInterface::Ptr createStorage(AkonadiFakeData &data)
    {
        return Akonadi::StorageInterface::Ptr(data.createStorage());
    }

    Akonadi::SerializerInterface::Ptr createSerializer()
    {
        return Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
    }

private slots:
    void shouldFetchAllCollections()
    {
        // GIVEN
        auto data = AkonadiFakeData();
        auto helpers = createHelpers(data);

        // Three top level collections (any content, tasks and notes)
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));
        data.createCollection(GenCollection().withId(43).withRootAsParent().withName(QStringLiteral("43")).withTaskContent());
        data.createCollection(GenCollection().withId(44).withRootAsParent().withName(QStringLiteral("44")).withNoteContent());

        // Three children under each of the top level for each content type
        data.createCollection(GenCollection().withId(45).withParent(42).withName(QStringLiteral("45")));
        data.createCollection(GenCollection().withId(46).withParent(42).withName(QStringLiteral("46")).withTaskContent());
        data.createCollection(GenCollection().withId(47).withParent(42).withName(QStringLiteral("47")).withNoteContent());
        data.createCollection(GenCollection().withId(48).withParent(43).withName(QStringLiteral("48")));
        data.createCollection(GenCollection().withId(49).withParent(43).withName(QStringLiteral("49")).withTaskContent());
        data.createCollection(GenCollection().withId(50).withParent(43).withName(QStringLiteral("50")).withNoteContent());
        data.createCollection(GenCollection().withId(51).withParent(44).withName(QStringLiteral("51")));
        data.createCollection(GenCollection().withId(52).withParent(44).withName(QStringLiteral("52")).withTaskContent());
        data.createCollection(GenCollection().withId(53).withParent(44).withName(QStringLiteral("53")).withNoteContent());

        // The list which will be filled by the fetch function
        auto collections = Akonadi::Collection::List();
        auto add = [&collections] (const Akonadi::Collection &collection) {
            collections.append(collection);
        };

        // WHEN
        auto fetch = helpers->fetchAllCollections();
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        auto result = QStringList();
        std::transform(collections.constBegin(), collections.constEnd(),
                       std::back_inserter(result),
                       std::bind(&Akonadi::Collection::displayName, _1));
        result.sort();

        // THEN
        auto expected = QStringList();
        expected << QStringLiteral("43") << QStringLiteral("46") << QStringLiteral("49") << QStringLiteral("52");

        expected.sort();
        QCOMPARE(result, expected);

        // WHEN (should not crash when the helpers object is deleted)
        helpers.clear();
        collections.clear();
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        result.clear();
        std::transform(collections.constBegin(), collections.constEnd(),
                       std::back_inserter(result),
                       std::bind(&Akonadi::Collection::displayName, _1));
        result.sort();
        QCOMPARE(result, expected);
    }

    void shouldFetchCollectionsForRoot_data()
    {
        QTest::addColumn<Akonadi::Collection>("root");

        QTest::newRow("all collections from root") << Akonadi::Collection::root();
        QTest::newRow("all collections from 'all branch'") << Akonadi::Collection(42);
        QTest::newRow("all collections from 'task branch'") << Akonadi::Collection(43);
        QTest::newRow("all collections from 'note branch'") << Akonadi::Collection(44);
    }

    void shouldFetchCollectionsForRoot()
    {
        // GIVEN
        auto data = AkonadiFakeData();
        auto helpers = createHelpers(data);

        // Three top level collections (any content, tasks and notes)
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));
        data.createCollection(GenCollection().withId(43).withRootAsParent().withName(QStringLiteral("43")).withTaskContent());
        data.createCollection(GenCollection().withId(44).withRootAsParent().withName(QStringLiteral("44")).withNoteContent());

        // Three children under each of the top level for each content type
        data.createCollection(GenCollection().withId(45).withParent(42).withName(QStringLiteral("45")));
        data.createCollection(GenCollection().withId(46).withParent(42).withName(QStringLiteral("46")).withTaskContent());
        data.createCollection(GenCollection().withId(47).withParent(42).withName(QStringLiteral("47")).withNoteContent());
        data.createCollection(GenCollection().withId(48).withParent(43).withName(QStringLiteral("48")));
        data.createCollection(GenCollection().withId(49).withParent(43).withName(QStringLiteral("49")).withTaskContent());
        data.createCollection(GenCollection().withId(50).withParent(43).withName(QStringLiteral("50")).withNoteContent());
        data.createCollection(GenCollection().withId(51).withParent(44).withName(QStringLiteral("51")));
        data.createCollection(GenCollection().withId(52).withParent(44).withName(QStringLiteral("52")).withTaskContent());
        data.createCollection(GenCollection().withId(53).withParent(44).withName(QStringLiteral("53")).withNoteContent());

        // The list which will be filled by the fetch function
        auto collections = Akonadi::Collection::List();
        auto add = [&collections] (const Akonadi::Collection &collection) {
            collections.append(collection);
        };

        // WHEN
        QFETCH(Akonadi::Collection, root);
        auto fetch = helpers->fetchCollections(root);
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        auto result = QStringList();
        std::transform(collections.constBegin(), collections.constEnd(),
                       std::back_inserter(result),
                       std::bind(&Akonadi::Collection::displayName, _1));
        result.sort();

        // THEN
        auto expected = QStringList();

        if (root == Akonadi::Collection::root()) {
            expected << QStringLiteral("42") << QStringLiteral("43") << QStringLiteral("44");
        } else {
            const qint64 baseId = root.id() == 42 ? 45
                                : root.id() == 43 ? 48
                                : root.id() == 44 ? 51
                                : -1;
            QVERIFY(baseId > 0);

            expected << QString::number(baseId + 1);
        }

        expected.sort();
        QCOMPARE(result, expected);

        // WHEN (should not crash when the helpers object is deleted)
        helpers.clear();
        collections.clear();
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        result.clear();
        std::transform(collections.constBegin(), collections.constEnd(),
                       std::back_inserter(result),
                       std::bind(&Akonadi::Collection::displayName, _1));
        result.sort();
        QCOMPARE(result, expected);
    }

    void shouldFetchItems()
    {
        // GIVEN
        auto data = AkonadiFakeData();
        auto helpers = createHelpers(data);

        // Two top level collections, one with no particular content, one with tasks
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));
        data.createCollection(GenCollection().withId(43).withRootAsParent().withName(QStringLiteral("43")).withTaskContent());

        // One note collection as child of the first one
        data.createCollection(GenCollection().withId(44).withParent(42).withName(QStringLiteral("44")).withNoteContent());

        // One task collection as child of the note collection
        data.createCollection(GenCollection().withId(45).withParent(44).withName(QStringLiteral("45")).withTaskContent());

        // One task in the first collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")));

        // Two tasks in all the other collections
        data.createItem(GenTodo().withId(43).withParent(43).withTitle(QStringLiteral("43")));
        data.createItem(GenTodo().withId(44).withParent(43).withTitle(QStringLiteral("44")));
        data.createItem(GenTodo().withId(45).withParent(44).withTitle(QStringLiteral("45")));
        data.createItem(GenTodo().withId(46).withParent(44).withTitle(QStringLiteral("46")));
        data.createItem(GenTodo().withId(47).withParent(45).withTitle(QStringLiteral("47")));
        data.createItem(GenTodo().withId(48).withParent(45).withTitle(QStringLiteral("48")));

        // The list which will be filled by the fetch function
        auto items = Akonadi::Item::List();
        auto add = [&items] (const Akonadi::Item &item) {
            items.append(item);
        };

        // WHEN
        auto fetch = helpers->fetchItems(nullptr);
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        auto result = QStringList();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();

        // THEN
        auto expected = QStringList();
        expected << QStringLiteral("43") << QStringLiteral("44") << QStringLiteral("47") << QStringLiteral("48");
        expected.sort();
        QCOMPARE(result, expected);

        // WHEN (should not crash when the helpers object is deleted)
        helpers.clear();
        items.clear();
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        result.clear();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();
        QCOMPARE(result, expected);
    }

    void shouldFetchItemsByCollection_data()
    {
        QTest::addColumn<Akonadi::Collection>("collection");

        QTest::newRow("first collection") << Akonadi::Collection(42);
        QTest::newRow("second collection") << Akonadi::Collection(43);
    }

    void shouldFetchItemsByCollection()
    {
        // GIVEN
        auto data = AkonadiFakeData();
        auto helpers = createHelpers(data);

        // Two top level collections with tasks
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")).withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withName(QStringLiteral("43")).withTaskContent());

        // Two items in each collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")));
        data.createItem(GenTodo().withId(43).withParent(42).withTitle(QStringLiteral("43")));
        data.createItem(GenTodo().withId(44).withParent(43).withTitle(QStringLiteral("44")));
        data.createItem(GenTodo().withId(45).withParent(43).withTitle(QStringLiteral("45")));

        // The list which will be filled by the fetch function
        auto items = Akonadi::Item::List();
        auto add = [&items] (const Akonadi::Item &item) {
            items.append(item);
        };

        // WHEN
        QFETCH(Akonadi::Collection, collection);
        auto fetch = helpers->fetchItems(collection, nullptr);
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        auto result = QStringList();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();

        // THEN
        auto expected = QStringList();

        switch (collection.id()) {
        case 42:
            expected << QStringLiteral("42") << QStringLiteral("43");
            break;
        case 43:
            expected << QStringLiteral("44") << QStringLiteral("45");
            break;
        }
        QVERIFY(!expected.isEmpty());

        expected.sort();
        QCOMPARE(result, expected);

        // WHEN (should not crash when the helpers object is deleted)
        helpers.clear();
        items.clear();
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        result.clear();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();
        QCOMPARE(result, expected);
    }

    void shouldFetchItemsForContext_data()
    {
        QTest::addColumn<Domain::Context::Ptr>("context");

        auto context1 = Domain::Context::Ptr::create();
        context1->setProperty("todoUid", "ctx-42");
        auto context2 = Domain::Context::Ptr::create();
        context2->setProperty("todoUid", "ctx-43");

        QTest::newRow("first") << context1;
        QTest::newRow("second") << context2;
    }

    void shouldFetchItemsForContext()
    {
        // GIVEN
        auto data = AkonadiFakeData();
        auto helpers = createHelpers(data);

        // Two top level collections with tasks
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")).withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withName(QStringLiteral("43")).withTaskContent());

        // Two contexts
        data.createItem(GenTodo().withId(142).withUid("ctx-42").asContext());
        data.createItem(GenTodo().withId(143).withUid("ctx-43").asContext());

        // Four items in each collection, one with no context, one with the first context,
        // one with the second context, last one with both contexts
        data.createItem(GenTodo().withId(42).withParent(42).withContexts({}).withTitle(QStringLiteral("42")));
        data.createItem(GenTodo().withId(43).withParent(42).withContexts({"ctx-42"}).withTitle(QStringLiteral("43")));
        data.createItem(GenTodo().withId(44).withParent(42).withContexts({"ctx-43"}).withTitle(QStringLiteral("44")));
        data.createItem(GenTodo().withId(45).withParent(42).withContexts({"ctx-42", "ctx-43"}).withTitle(QStringLiteral("45")));
        data.createItem(GenTodo().withId(46).withParent(43).withContexts({}).withTitle(QStringLiteral("46")));
        data.createItem(GenTodo().withId(47).withParent(43).withContexts({"ctx-42"}).withTitle(QStringLiteral("47")));
        data.createItem(GenTodo().withId(48).withParent(43).withContexts({"ctx-43"}).withTitle(QStringLiteral("48")));
        data.createItem(GenTodo().withId(49).withParent(43).withContexts({"ctx-42", "ctx-43"}).withTitle(QStringLiteral("49")));

        // The list which will be filled by the fetch function
        auto items = Akonadi::Item::List();
        auto add = [&items] (const Akonadi::Item &item) {
            items.append(item);
        };

        // WHEN
        QFETCH(Domain::Context::Ptr, context);
        auto fetch = helpers->fetchItemsForContext(context, nullptr);
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        auto result = QStringList();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();

        // THEN
        auto expected = QStringList();

        if (context->property("todoUid") == "ctx-42")
            expected << QStringLiteral("43") << QStringLiteral("45") << QStringLiteral("47") << QStringLiteral("49");
        else if (context->property("todoUid") == "ctx-43")
            expected << QStringLiteral("44") << QStringLiteral("45") << QStringLiteral("48") << QStringLiteral("49");
        QVERIFY(!expected.isEmpty());

        expected.sort();
        QCOMPARE(result, expected);

        // WHEN (should not crash when the helpers object is deleted)
        helpers.clear();
        items.clear();
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        result.clear();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();
        QCOMPARE(result, expected);
    }

    void shouldFetchSiblings_data()
    {
        QTest::addColumn<Akonadi::Item>("item");

        QTest::newRow("item in first collection") << Akonadi::Item(43);
        QTest::newRow("item in second collection") << Akonadi::Item(48);
    }

    void shouldFetchSiblings()
    {
        // GIVEN
        auto data = AkonadiFakeData();
        auto helpers = createHelpers(data);

        // Two top level collections (one with notes, one with tasks)
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")).withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withName(QStringLiteral("43")).withNoteContent());

        // Four items in each collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")));
        data.createItem(GenTodo().withId(43).withParent(42).withTitle(QStringLiteral("43")));
        data.createItem(GenTodo().withId(44).withParent(42).withTitle(QStringLiteral("44")));
        data.createItem(GenTodo().withId(45).withParent(42).withTitle(QStringLiteral("45")));
        data.createItem(GenTodo().withId(46).withParent(43).withTitle(QStringLiteral("46")));
        data.createItem(GenTodo().withId(47).withParent(43).withTitle(QStringLiteral("47")));
        data.createItem(GenTodo().withId(48).withParent(43).withTitle(QStringLiteral("48")));
        data.createItem(GenTodo().withId(49).withParent(43).withTitle(QStringLiteral("49")));

        // The list which will be filled by the fetch function
        auto items = Akonadi::Item::List();
        auto add = [&items] (const Akonadi::Item &item) {
            items.append(item);
        };

        // WHEN
        QFETCH(Akonadi::Item, item);
        auto fetch = helpers->fetchSiblings(item, nullptr);
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        auto result = QStringList();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();

        // THEN
        auto expected = QStringList();

        switch (item.id()) {
        case 43:
            expected << QStringLiteral("42") << QStringLiteral("43") << QStringLiteral("44") << QStringLiteral("45");
            break;
        case 48:
            expected << QStringLiteral("46") << QStringLiteral("47") << QStringLiteral("48") << QStringLiteral("49");
            break;
        }
        QVERIFY(!expected.isEmpty());

        expected.sort();
        QCOMPARE(result, expected);

        // WHEN (should not crash when the helpers object is deleted)
        helpers.clear();
        items.clear();
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        result.clear();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();
        QCOMPARE(result, expected);
    }
};

ZANSHIN_TEST_MAIN(AkonadiLiveQueryHelpersTest)

#include "akonadilivequeryhelperstest.moc"
