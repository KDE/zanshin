/*
 * SPDX-FileCopyrightText: 2017 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "akonadi/akonadicachingstorage.h"
#include "akonadi/akonadiserializer.h"

#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"

#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/gentodo.h"
#include "testlib/testhelpers.h"

Q_DECLARE_METATYPE(Akonadi::StorageInterface::FetchDepth)

using namespace Testlib;

class AkonadiCachingStorageTest : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiCachingStorageTest(QObject *parent = nullptr)
        : QObject(parent)
    {
        qRegisterMetaType<Akonadi::Collection>();
        qRegisterMetaType<Akonadi::Item>();
    }

    QStringList onlyWithSuffix(const QStringList &list, const QString &suffix)
    {
        return onlyWithSuffixes(list, {suffix});
    }

    QStringList onlyWithSuffixes(const QStringList &list, const QStringList &suffixes)
    {
        auto res = QStringList();
        std::copy_if(list.cbegin(), list.cend(),
                     std::back_inserter(res),
                     [suffixes](const QString &entry) {
                        for (const auto &suffix : suffixes) {
                            if (entry.endsWith(suffix))
                                return true;
                        }
                        return false;
                     });
        return res;
    }

private slots:
    void shouldCacheAllCollectionsPerFetchType_data()
    {
        QTest::addColumn<Akonadi::Collection>("rootCollection");
        QTest::addColumn<Akonadi::StorageInterface::FetchDepth>("fetchDepth");
        QTest::addColumn<QStringList>("expectedFetchNames");
        QTest::addColumn<QStringList>("expectedCachedNames");

        const auto allCollections = QStringList() << "42Task" << "43Task" << "44Note" << "45Stuff"
                                                  << "46Note" << "47Task" << "48Note" << "49Stuff"
                                                  << "50Stuff" << "51Task" << "52Note" << "53Stuff"
                                                  << "54Task" << "55Task" << "56Task"
                                                  << "57Note" << "58Note" << "59Note"
                                                  << "60Stuff" << "61Stuff" << "62Stuff";

        const auto taskCollections = QStringList() << "42Task" << "43Task"
                                                   << "46Note" << "47Task"
                                                   << "50Stuff" << "51Task"
                                                   << "54Task" << "55Task" << "56Task";

        QTest::newRow("rootRecursiveTask") << Akonadi::Collection::root() << Akonadi::StorageInterface::Recursive
                                           << onlyWithSuffix(taskCollections, "Task") << taskCollections;

        QTest::newRow("54RecursiveTask") << Akonadi::Collection(54) << Akonadi::StorageInterface::Recursive
                                         << (QStringList() << "55Task" << "56Task") << taskCollections;

        QTest::newRow("54FirstLevelTask") << Akonadi::Collection(54) << Akonadi::StorageInterface::FirstLevel
                                          << (QStringList() << "55Task") << taskCollections;

        QTest::newRow("54BaseTask") << Akonadi::Collection(54) << Akonadi::StorageInterface::Base
                                    << (QStringList() << "54Task") << taskCollections;
    }

    void shouldCacheAllCollectionsPerFetchType()
    {
        // GIVEN
        AkonadiFakeData data;

        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Task")).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43Task")).withParent(42).withTaskContent());
        data.createCollection(GenCollection().withId(44).withName(QStringLiteral("44Note")).withParent(42).withNoteContent());
        data.createCollection(GenCollection().withId(45).withName(QStringLiteral("45Stuff")).withParent(42));

        data.createCollection(GenCollection().withId(46).withName(QStringLiteral("46Note")).withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(47).withName(QStringLiteral("47Task")).withParent(46).withTaskContent());
        data.createCollection(GenCollection().withId(48).withName(QStringLiteral("48Note")).withParent(46).withNoteContent());
        data.createCollection(GenCollection().withId(49).withName(QStringLiteral("49Stuff")).withParent(46));

        data.createCollection(GenCollection().withId(50).withName(QStringLiteral("50Stuff")).withRootAsParent());
        data.createCollection(GenCollection().withId(51).withName(QStringLiteral("51Task")).withParent(50).withTaskContent());
        data.createCollection(GenCollection().withId(52).withName(QStringLiteral("52Note")).withParent(50).withNoteContent());
        data.createCollection(GenCollection().withId(53).withName(QStringLiteral("53Stuff")).withParent(50));

        data.createCollection(GenCollection().withId(54).withName(QStringLiteral("54Task")).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(55).withName(QStringLiteral("55Task")).withParent(54).withTaskContent());
        data.createCollection(GenCollection().withId(56).withName(QStringLiteral("56Task")).withParent(55).withTaskContent());

        data.createCollection(GenCollection().withId(57).withName(QStringLiteral("57Note")).withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(58).withName(QStringLiteral("58Note")).withParent(57).withNoteContent());
        data.createCollection(GenCollection().withId(59).withName(QStringLiteral("59Note")).withParent(58).withNoteContent());

        data.createCollection(GenCollection().withId(60).withName(QStringLiteral("60Stuff")).withRootAsParent());
        data.createCollection(GenCollection().withId(61).withName(QStringLiteral("61Stuff")).withParent(60));
        data.createCollection(GenCollection().withId(62).withName(QStringLiteral("62Stuff")).withParent(61));

        auto cache = Akonadi::Cache::Ptr::create(Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                 Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        Akonadi::CachingStorage storage(cache, Akonadi::StorageInterface::Ptr(data.createStorage()));

        // WHEN
        QFETCH(Akonadi::Collection, rootCollection);
        QFETCH(Akonadi::StorageInterface::FetchDepth, fetchDepth);
        auto job = storage.fetchCollections(rootCollection, fetchDepth, nullptr);
        QVERIFY2(job->kjob()->exec(), qPrintable(job->kjob()->errorString()));

        // THEN
        const auto toCollectionNames = [](const Akonadi::Collection::List &collections) {
            auto res = QStringList();
            res.reserve(collections.size());
            std::transform(collections.cbegin(), collections.cend(),
                           std::back_inserter(res),
                           std::mem_fn(&Akonadi::Collection::name));
            res.sort();
            return res;
        };

        QFETCH(QStringList, expectedFetchNames);
        QFETCH(QStringList, expectedCachedNames);

        {
            const auto collectionFetchNames = toCollectionNames(job->collections());
            QCOMPARE(collectionFetchNames, expectedFetchNames);

            const auto collections = cache->allCollections();
            const auto collectionCachedNames = toCollectionNames(collections);
            QCOMPARE(collectionCachedNames, expectedCachedNames);
        }

        // WHEN (second time shouldn't hit the original storage)
        data.storageBehavior().setFetchCollectionsBehavior(rootCollection.id(), AkonadiFakeStorageBehavior::EmptyFetch);
        data.storageBehavior().setFetchCollectionsErrorCode(rootCollection.id(), 128);
        job = storage.fetchCollections(rootCollection, fetchDepth, nullptr);
        QVERIFY2(job->kjob()->exec(), qPrintable(job->kjob()->errorString()));

        {
            const auto collectionFetchNames = toCollectionNames(job->collections());
            QCOMPARE(collectionFetchNames, expectedFetchNames);

            const auto collections = cache->allCollections();
            const auto collectionCachedNames = toCollectionNames(collections);
            QCOMPARE(collectionCachedNames, expectedCachedNames);
        }
    }

    void shouldCacheAllItemsPerCollection()
    {
        // GIVEN
        AkonadiFakeData data;

        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Col")).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43Col")).withRootAsParent().withTaskContent());

        data.createItem(GenTodo().withId(42).withTitle(QStringLiteral("42Task")).withParent(42));
        data.createItem(GenTodo().withId(45).withTitle(QStringLiteral("45Task")).withParent(42));
        data.createItem(GenTodo().withId(52).withTitle(QStringLiteral("52Task")).withParent(42));

        data.createItem(GenTodo().withId(44).withTitle(QStringLiteral("44Task")).withParent(43));
        data.createItem(GenTodo().withId(48).withTitle(QStringLiteral("48Task")).withParent(43));
        data.createItem(GenTodo().withId(50).withTitle(QStringLiteral("50Task")).withParent(43));

        auto cache = Akonadi::Cache::Ptr::create(Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                 Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        Akonadi::CachingStorage storage(cache, Akonadi::StorageInterface::Ptr(data.createStorage()));

        // WHEN
        auto job = storage.fetchItems(Akonadi::Collection(42), nullptr);
        QVERIFY2(job->kjob()->exec(), qPrintable(job->kjob()->errorString()));

        // THEN
        const auto toItemIds = [](const Akonadi::Item::List &items) {
            auto res = QList<Akonadi::Item::Id>();
            res.reserve(items.size());
            std::transform(items.cbegin(), items.cend(),
                           std::back_inserter(res),
                           std::mem_fn(&Akonadi::Item::id));
            std::sort(res.begin(), res.end());
            return res;
        };

        auto expectedIds = QList<Akonadi::Item::Id>() << 42 << 45 << 52;
        {
            const auto itemFetchIds = toItemIds(job->items());
            QCOMPARE(itemFetchIds, expectedIds);

            const auto items = cache->items(Akonadi::Collection(42));
            const auto itemCachedIds = toItemIds(items);
            QCOMPARE(itemCachedIds, expectedIds);
        }

        // WHEN (second time shouldn't hit the original storage)
        data.storageBehavior().setFetchItemsBehavior(42, AkonadiFakeStorageBehavior::EmptyFetch);
        data.storageBehavior().setFetchItemsErrorCode(42, 128);
        job = storage.fetchItems(Akonadi::Collection(42), nullptr);
        QVERIFY2(job->kjob()->exec(), qPrintable(job->kjob()->errorString()));

        {
            const auto itemFetchIds = toItemIds(job->items());
            QCOMPARE(itemFetchIds, expectedIds);

            const auto items = cache->items(Akonadi::Collection(42));
            const auto itemCachedIds = toItemIds(items);
            QCOMPARE(itemCachedIds, expectedIds);
        }
    }

    void shouldCacheSingleItems()
    {
        // GIVEN
        AkonadiFakeData data;

        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Col")).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43Col")).withRootAsParent().withTaskContent());

        data.createItem(GenTodo().withId(42).withTitle(QStringLiteral("42Task")).withParent(42));
        data.createItem(GenTodo().withId(45).withTitle(QStringLiteral("45Task")).withParent(42));
        data.createItem(GenTodo().withId(52).withTitle(QStringLiteral("52Task")).withParent(42));

        data.createItem(GenTodo().withId(44).withTitle(QStringLiteral("44Task")).withParent(43));
        data.createItem(GenTodo().withId(48).withTitle(QStringLiteral("48Task")).withParent(43));
        data.createItem(GenTodo().withId(50).withTitle(QStringLiteral("50Task")).withParent(43));

        auto cache = Akonadi::Cache::Ptr::create(Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                 Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        Akonadi::CachingStorage storage(cache, Akonadi::StorageInterface::Ptr(data.createStorage()));

        // WHEN
        auto job = storage.fetchItem(Akonadi::Item(44), nullptr);
        QVERIFY2(job->kjob()->exec(), qPrintable(job->kjob()->errorString()));

        // THEN
        const auto toItemIds = [](const Akonadi::Item::List &items) {
            auto res = QList<Akonadi::Item::Id>();
            res.reserve(items.size());
            std::transform(items.cbegin(), items.cend(),
                           std::back_inserter(res),
                           std::mem_fn(&Akonadi::Item::id));
            std::sort(res.begin(), res.end());
            return res;
        };

        auto expectedIds = QList<Akonadi::Item::Id>() << 44;
        {
            const auto itemFetchIds = toItemIds(job->items());
            QCOMPARE(itemFetchIds, expectedIds);
            QVERIFY(!cache->item(44).isValid());
        }

        // WHEN (if collection is populated, shouldn't hit the original storage)
        job = storage.fetchItems(Akonadi::Collection(43), nullptr);
        QVERIFY2(job->kjob()->exec(), qPrintable(job->kjob()->errorString()));

        data.storageBehavior().setFetchItemBehavior(44, AkonadiFakeStorageBehavior::EmptyFetch);
        data.storageBehavior().setFetchItemErrorCode(44, 128);
        job = storage.fetchItem(Akonadi::Item(44), nullptr);
        QVERIFY2(job->kjob()->exec(), qPrintable(job->kjob()->errorString()));

        {
            const auto itemFetchIds = toItemIds(job->items());
            QCOMPARE(itemFetchIds, expectedIds);
            QVERIFY(cache->item(44).isValid());
        }
    }
};

ZANSHIN_TEST_MAIN(AkonadiCachingStorageTest)

#include "akonadicachingstoragetest.moc"
