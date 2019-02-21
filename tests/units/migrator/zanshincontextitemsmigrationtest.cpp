/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014-2019 David Faure <faure@kde.org>

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

#include "zanshincontextitemsmigrator.h"

#include <testlib/qtest_zanshin.h>
#include <QSignalSpy>
#include <testlib/testsafety.h>
#include <testlib/akonadidebug.h>

#include <KCalCore/Todo>
#include <KCalCore/ICalFormat>

#include <AkonadiCore/Collection>
#include <AkonadiCore/TransactionSequence>
#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonadistorage.h"
#include "akonadi/akonadiserializer.h"

#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>

#include <QProcess>

class ZanshinContextItemsMigrationTest : public QObject
{
    Q_OBJECT
public:
    explicit ZanshinContextItemsMigrationTest(QObject *parent = nullptr)
        : QObject(parent)
    {
        qRegisterMetaType<Akonadi::Collection>();
        qRegisterMetaType<Akonadi::Item>();
    }

private slots:
    void initTestCase()
    {
        QVERIFY(TestLib::TestSafety::checkTestIsIsolated());
        auto storage = Akonadi::StorageInterface::Ptr(new Akonadi::Storage);
        TestLib::AkonadiDebug::dumpTree(storage);
        //QProcess proc;
        //proc.execute("akonadiconsole");
        //qApp->exec();
    }

    void cleanupTestCase()
    {
        // Give a chance for jobs still waiting for an event loop
        // run to be deleted through deleteLater()
        QTest::qWait(10);
    }

    void shouldFetchAllItems()
    {
        // GIVEN
        ZanshinContextItemsMigrator migrator;

        // WHEN
        auto result = migrator.fetchAllItems(WhichItems::TasksToConvert);

        // THEN
        // the migrator gathered all items and the related tags
        m_expectedUids.insert(QStringLiteral("child-of-project"), {"errands-context", "online-context"});
        m_expectedUids.insert(QStringLiteral("new-project-with-property"), {});
        m_expectedUids.insert(QStringLiteral("old-project-with-comment"), {"errands-context"});
        m_expectedUids.insert(QStringLiteral("project-with-comment-and-property"), {});
        m_expectedUids.insert(QStringLiteral("project-with-children"), {});
        m_expectedUids.insert(QStringLiteral("standalone-task"), {"errands-context"});

        checkExpectedTags(result.items, m_expectedUids);

        // and picked a collection
        QCOMPARE(result.pickedCollection.name(), "Calendar2");
    }

    void shouldFetchAllTags()
    {
        // GIVEN
        ZanshinContextItemsMigrator migrator;

        // WHEN
        Akonadi::Tag::List tags = migrator.fetchAllTags();

        // THEN
        QCOMPARE(tagNames(tags), QStringList() << "errands-context" << "online-context" << "unused-context");
    }

    void shouldCreateContexts()
    {
        // GIVEN
        ZanshinContextItemsMigrator migrator;
        auto result = migrator.fetchAllItems(WhichItems::TasksToConvert); // in fact we just need the collection...
        Akonadi::Tag::List tags = migrator.fetchAllTags();

        // WHEN
        migrator.createContexts(tags, result.pickedCollection);

        // THEN
        auto newItemList = migrator.fetchAllItems(WhichItems::OnlyContexts).items;
        Akonadi::Item::List contextItems;
        QVector<KCalCore::Todo::Ptr> contextTodos;
        for (auto it = newItemList.constBegin(); it != newItemList.constEnd(); ++it) {
            const auto item = *it;
            if (m_serializer.isContext(item)) {
                contextItems.append(item);
                contextTodos.append(item.payload<KCalCore::Todo::Ptr>());
            }
        }
        QCOMPARE(contextItems.size(), 3);
        QCOMPARE(contextTodos.size(), 3);
        QCOMPARE(contextTodos.at(0)->uid(), "errands-context");
        QCOMPARE(contextTodos.at(1)->uid(), "online-context");
        QCOMPARE(contextTodos.at(2)->uid(), "unused-context");
    }

    void shouldAssociateContexts()
    {
        // GIVEN
        ZanshinContextItemsMigrator migrator;
        auto result = migrator.fetchAllItems(WhichItems::TasksToConvert);

        // WHEN
        migrator.associateContexts(result.items);

        // THEN
        auto newResult = migrator.fetchAllItems(WhichItems::AllTasks);
        checkExpectedContexts(newResult.items, m_expectedUids);
   }

private:

    void checkExpectedTags(const Akonadi::Item::List &items, const QMap<QString /*uid*/, QStringList /*tags*/> &expectedItems)
    {
        const QMap<QString, Akonadi::Item> itemHash = fillHash(items);
        const QStringList uids = itemHash.keys();
        if (items.count() != expectedItems.count()) // QCOMPARE for QStringList isn't verbose enough
            qWarning() << "Got" << uids << "expected" << expectedItems.keys();
        QCOMPARE(uids, QStringList(expectedItems.keys()));

        for (auto it = expectedItems.constBegin(); it != expectedItems.constEnd(); ++it) {
            //qDebug() << it.key();
            const Akonadi::Item item = itemHash.value(it.key());
            const auto allTags = item.tags();
            Akonadi::Tag::List tags;
            std::copy_if(allTags.constBegin(), allTags.constEnd(), std::back_inserter(tags),
                         [](const Akonadi::Tag &tag) { return tag.type() == "Zanshin-Context"; });
            const auto itemTags = tagNames(tags);
            if (itemTags != it.value()) // QCOMPARE for QStringList isn't verbose enough
                qWarning() << it.key() << "got" << itemTags << "expected" << it.value();
            QCOMPARE(itemTags, it.value());

            QVERIFY(!m_serializer.isContext(item));
        }
    }

    static QMap<QString, Akonadi::Item> fillHash(const Akonadi::Item::List &items)
    {
        QMap<QString, Akonadi::Item> itemHash;
        for (const Akonadi::Item &item : items) {
            auto todo = item.payload<KCalCore::Todo::Ptr>();
            itemHash.insert(todo->uid(), item);
        }
        return itemHash;
    }

    void checkExpectedContexts(const Akonadi::Item::List &items, const QMap<QString /*uid*/, QStringList /*tags*/> &expectedItems)
    {
        const QMap<QString, Akonadi::Item> itemHash = fillHash(items);
        const QStringList uids = itemHash.keys();
        if (uids.count() != expectedItems.count()) // QCOMPARE for QStringList isn't verbose enough
            qWarning() << "Got" << uids << "expected" << expectedItems.keys();
        QCOMPARE(uids, QStringList(expectedItems.keys()));

        for (auto it = expectedItems.constBegin(); it != expectedItems.constEnd(); ++it) {
            const Akonadi::Item item = itemHash.value(it.key());
            //qDebug() << item.id() << it.key();
            auto todo = item.payload<KCalCore::Todo::Ptr>();
            QVERIFY(todo);
            const auto contexts = todo->customProperty("Zanshin", "ContextList").split(',', QString::SkipEmptyParts);
            if (contexts != it.value()) // QCOMPARE for QStringList isn't verbose enough
                qWarning() << it.key() << "got" << contexts << "expected" << it.value();
            QCOMPARE(contexts, it.value());

            QVERIFY(!m_serializer.isContext(item));
        }
    }

    static QStringList tagNames(const Akonadi::Tag::List &tags)
    {
        QStringList itemTags;
        std::transform(tags.constBegin(), tags.constEnd(), std::back_inserter(itemTags), [](const Akonadi::Tag &tag) { return tag.gid(); });
        itemTags.sort();
        return itemTags;
    }

    QMap<QString /*uid*/, QStringList /*tags --> contexts*/> m_expectedUids;
    Akonadi::Serializer m_serializer;
};


ZANSHIN_TEST_MAIN(ZanshinContextItemsMigrationTest)

#include "zanshincontextitemsmigrationtest.moc"



