/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2014-2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "zanshincontextitemsmigrator.h"

#include <testlib/qtest_zanshin.h>
#include <QSignalSpy>
#include <testlib/testsafety.h>
#include <testlib/akonadidebug.h>

#include <KCalCore/Todo>
#include <KCalCore/ICalFormat>

#include <AkonadiCore/Akonadi/Collection>
#include <AkonadiCore/Akonadi/TransactionSequence>
#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonadistorage.h"
#include "akonadi/akonadiserializer.h"

#include <AkonadiCore/Akonadi/CollectionFetchJob>
#include <AkonadiCore/Akonadi/ItemFetchJob>
#include <AkonadiCore/Akonadi/ItemFetchScope>

#include <QProcess>

using Akonadi::Serializer;

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
        // WHEN
        auto result = m_migrator.fetchAllItems(WhichItems::TasksToConvert);

        // THEN
        // the migrator gathered all items and the related tags
        m_expectedUids.insert(QStringLiteral("child-of-project"), {"Errands", "Online"});
        m_expectedUids.insert(QStringLiteral("new-project-with-property"), {});
        m_expectedUids.insert(QStringLiteral("old-project-with-comment"), {"Errands"});
        m_expectedUids.insert(QStringLiteral("project-with-comment-and-property"), {});
        m_expectedUids.insert(QStringLiteral("project-with-children"), {});
        m_expectedUids.insert(QStringLiteral("standalone-task"), {"Errands"});

        checkExpectedTags(result.items, m_expectedUids);

        // and picked a collection
        QCOMPARE(result.pickedCollection.name(), "Calendar2");
    }

    void shouldFetchAllTags()
    {
        // WHEN
        Akonadi::Tag::List tags = m_migrator.fetchAllTags();

        // THEN
        QCOMPARE(tagNames(tags), QStringList() << "Errands" << "Online" << "Unused");
    }

    void shouldCreateContexts()
    {
        // GIVEN
        const auto result = m_migrator.fetchAllItems(WhichItems::TasksToConvert); // in fact we just need the collection...
        const Akonadi::Tag::List tags = m_migrator.fetchAllTags();

        // WHEN
        m_migrator.createContexts(tags, result.pickedCollection);

        // THEN
        auto newItemList = m_migrator.fetchAllItems(WhichItems::OnlyContexts).items;
        Akonadi::Item::List contextItems;
        QVector<KCalCore::Todo::Ptr> contextTodos;
        for (auto it = newItemList.constBegin(); it != newItemList.constEnd(); ++it) {
            const auto item = *it;
            if (m_serializer.isContext(item)) {
                contextItems.append(item);
                auto todo = item.payload<KCalCore::Todo::Ptr>();
                contextTodos.append(todo);
                m_contextTodos.insert(todo->uid(), todo->summary());
            }
        }
        QCOMPARE(contextItems.size(), 3);
        QCOMPARE(contextTodos.size(), 3);
        QCOMPARE(contextTodos.at(0)->summary(), "Errands");
        QCOMPARE(contextTodos.at(1)->summary(), "Online");
        QCOMPARE(contextTodos.at(2)->summary(), "Unused");
    }

    void shouldAssociateContexts()
    {
        // GIVEN
        auto result = m_migrator.fetchAllItems(WhichItems::TasksToConvert);

        // WHEN
        m_migrator.associateContexts(result.items);

        // THEN
        auto newResult = m_migrator.fetchAllItems(WhichItems::AllTasks);
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
        const QMap<QString /*uid*/, Akonadi::Item> itemHash = fillHash(items);
        const QStringList uids = itemHash.keys();
        if (uids.count() != expectedItems.count()) // QCOMPARE for QStringList isn't verbose enough
            qWarning() << "Got" << uids << "expected" << expectedItems.keys();
        QCOMPARE(uids, QStringList(expectedItems.keys()));

        for (auto it = expectedItems.constBegin(); it != expectedItems.constEnd(); ++it) {
            const Akonadi::Item item = itemHash.value(it.key());
            //qDebug() << item.id() << it.key();
            auto todo = item.payload<KCalCore::Todo::Ptr>();
            QVERIFY(todo);
            const auto contextUids = todo->customProperty(Serializer::customPropertyAppName(), Serializer::customPropertyContextList()).split(',', QString::SkipEmptyParts);
            QStringList contextNames;
            std::transform(contextUids.cbegin(), contextUids.cend(), std::back_inserter(contextNames), [this](const QString &uid) { return m_contextTodos.value(uid); });
            contextNames.sort();
            if (contextNames != it.value()) // QCOMPARE for QStringList isn't verbose enough
                qWarning() << it.key() << "got" << contextNames << "expected" << it.value();
            QCOMPARE(contextNames, it.value());

            QVERIFY(!m_serializer.isContext(item));
        }
    }

    static QStringList tagNames(const Akonadi::Tag::List &tags)
    {
        QStringList itemTags;
        std::transform(tags.constBegin(), tags.constEnd(), std::back_inserter(itemTags), [](const Akonadi::Tag &tag) { return tag.name(); });
        itemTags.sort();
        return itemTags;
    }

    QMap<QString /*uid*/, QStringList /*tags --> contexts*/> m_expectedUids;
    Akonadi::Serializer m_serializer;
    QHash<QString /*uid*/, QString /*name*/> m_contextTodos;
    ZanshinContextItemsMigrator m_migrator;
};


ZANSHIN_TEST_MAIN(ZanshinContextItemsMigrationTest)

#include "zanshincontextitemsmigrationtest.moc"



