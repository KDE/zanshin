/* This file is part of Zanshin

   Copyright 2017 Kevin Ottens <ervin@kde.org>

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

#include "akonadi/akonadicachingstorage.h"
#include "akonadi/akonadiserializer.h"

#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonaditagfetchjobinterface.h"

#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/gentodo.h"
#include "testlib/gentag.h"
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
        qRegisterMetaType<Akonadi::Tag>();
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
        QTest::addColumn<int>("contentTypesInt");
        QTest::addColumn<QStringList>("expectedFetchNames");
        QTest::addColumn<QStringList>("expectedCachedNames");

        const auto allCollections = QStringList() << "42Task" << "43Task" << "44Note" << "45Stuff"
                                                  << "46Note" << "47Task" << "48Note" << "49Stuff"
                                                  << "50Stuff" << "51Task" << "52Note" << "53Stuff"
                                                  << "54Task" << "55Task" << "56Task"
                                                  << "57Note" << "58Note" << "59Note"
                                                  << "60Stuff" << "61Stuff" << "62Stuff";

        const auto noteCollections = QStringList() << "42Task" << "44Note"
                                                   << "46Note" << "48Note"
                                                   << "50Stuff" << "52Note"
                                                   << "57Note" << "58Note" << "59Note";

        const auto taskCollections = QStringList() << "42Task" << "43Task"
                                                   << "46Note" << "47Task"
                                                   << "50Stuff" << "51Task"
                                                   << "54Task" << "55Task" << "56Task";

        const auto noteTaskCollections = QStringList() << "42Task" << "43Task" << "44Note"
                                                       << "46Note" << "47Task" << "48Note"
                                                       << "50Stuff" << "51Task" << "52Note"
                                                       << "54Task" << "55Task" << "56Task"
                                                       << "57Note" << "58Note" << "59Note";

        QTest::newRow("rootRecursiveAll") << Akonadi::Collection::root() << Akonadi::StorageInterface::Recursive << int(Akonadi::StorageInterface::AllContent)
                                          << allCollections << allCollections;
        QTest::newRow("rootRecursiveTask") << Akonadi::Collection::root() << Akonadi::StorageInterface::Recursive << int(Akonadi::StorageInterface::Tasks)
                                          << onlyWithSuffix(taskCollections, "Task") << taskCollections;
        QTest::newRow("rootRecursiveNote") << Akonadi::Collection::root() << Akonadi::StorageInterface::Recursive << int(Akonadi::StorageInterface::Notes)
                                           << onlyWithSuffix(noteCollections, "Note") << noteCollections;
        QTest::newRow("rootRecursiveNoteTask") << Akonadi::Collection::root() << Akonadi::StorageInterface::Recursive
                                               << int(Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                               << onlyWithSuffixes(noteTaskCollections, {"Task", "Note"}) << noteTaskCollections;

        QTest::newRow("60RecursiveAll") << Akonadi::Collection(60) << Akonadi::StorageInterface::Recursive << int(Akonadi::StorageInterface::AllContent)
                                        << (QStringList() << "61Stuff" << "62Stuff") << allCollections;
        QTest::newRow("54RecursiveTask") << Akonadi::Collection(54) << Akonadi::StorageInterface::Recursive << int(Akonadi::StorageInterface::Tasks)
                                         << (QStringList() << "55Task" << "56Task") << taskCollections;
        QTest::newRow("57RecursiveNote") << Akonadi::Collection(57) << Akonadi::StorageInterface::Recursive << int(Akonadi::StorageInterface::Notes)
                                         << (QStringList() << "58Note" << "59Note") << noteCollections;
        QTest::newRow("54RecursiveNoteTask") << Akonadi::Collection(54) << Akonadi::StorageInterface::Recursive
                                             << int(Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                             << (QStringList() << "55Task" << "56Task") << noteTaskCollections;
        QTest::newRow("57RecursiveNoteTask") << Akonadi::Collection(57) << Akonadi::StorageInterface::Recursive
                                             << int(Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                             << (QStringList() << "58Note" << "59Note") << noteTaskCollections;

        QTest::newRow("60FirstLevelAll") << Akonadi::Collection(60) << Akonadi::StorageInterface::FirstLevel << int(Akonadi::StorageInterface::AllContent)
                                         << (QStringList() << "61Stuff") << allCollections;
        QTest::newRow("54FirstLevelTask") << Akonadi::Collection(54) << Akonadi::StorageInterface::FirstLevel << int(Akonadi::StorageInterface::Tasks)
                                          << (QStringList() << "55Task") << taskCollections;
        QTest::newRow("57FirstLevelNote") << Akonadi::Collection(57) << Akonadi::StorageInterface::FirstLevel << int(Akonadi::StorageInterface::Notes)
                                          << (QStringList() << "58Note") << noteCollections;
        QTest::newRow("54FirstLevelNoteTask") << Akonadi::Collection(54) << Akonadi::StorageInterface::FirstLevel
                                              << int(Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                              << (QStringList() << "55Task") << noteTaskCollections;
        QTest::newRow("57FirstLevelNoteTask") << Akonadi::Collection(57) << Akonadi::StorageInterface::FirstLevel
                                              << int(Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                              << (QStringList() << "58Note") << noteTaskCollections;

        QTest::newRow("60BaseAll") << Akonadi::Collection(60) << Akonadi::StorageInterface::Base << int(Akonadi::StorageInterface::AllContent)
                                   << (QStringList() << "60Stuff") << allCollections;
        QTest::newRow("54BaseTask") << Akonadi::Collection(54) << Akonadi::StorageInterface::Base << int(Akonadi::StorageInterface::Tasks)
                                    << (QStringList() << "54Task") << taskCollections;
        QTest::newRow("57BaseNote") << Akonadi::Collection(57) << Akonadi::StorageInterface::Base << int(Akonadi::StorageInterface::Notes)
                                    << (QStringList() << "57Note") << noteCollections;
        QTest::newRow("54BaseNoteTask") << Akonadi::Collection(54) << Akonadi::StorageInterface::Base
                                        << int(Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                        << (QStringList() << "54Task") << noteTaskCollections;
        QTest::newRow("57BaseNoteTask") << Akonadi::Collection(57) << Akonadi::StorageInterface::Base
                                        << int(Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                        << (QStringList() << "57Note") << noteTaskCollections;
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
        QFETCH(int, contentTypesInt);
        const auto contentTypes = Akonadi::StorageInterface::FetchContentTypes(contentTypesInt);
        auto job = storage.fetchCollections(rootCollection, fetchDepth, contentTypes);
        QVERIFY2(job->kjob()->exec(), job->kjob()->errorString().toUtf8().constData());

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
            const auto collectionFetchNames = [job, toCollectionNames]{
                return toCollectionNames(job->collections());
            }();
            QCOMPARE(collectionFetchNames, expectedFetchNames);

            const auto collectionCachedNames = [cache, toCollectionNames]{
                const auto collections = cache->collections(Akonadi::StorageInterface::AllContent);
                return toCollectionNames(collections);
            }();
            QCOMPARE(collectionCachedNames, expectedCachedNames);
        }

        // WHEN (second time shouldn't hit the original storage)
        data.storageBehavior().setFetchCollectionsBehavior(rootCollection.id(), AkonadiFakeStorageBehavior::EmptyFetch);
        data.storageBehavior().setFetchCollectionsErrorCode(rootCollection.id(), 128);
        job = storage.fetchCollections(rootCollection, fetchDepth, contentTypes);
        QVERIFY2(job->kjob()->exec(), job->kjob()->errorString().toUtf8().constData());

        {
            const auto collectionFetchNames = [job, toCollectionNames]{
                return toCollectionNames(job->collections());
            }();
            QCOMPARE(collectionFetchNames, expectedFetchNames);

            const auto collectionCachedNames = [cache, toCollectionNames]{
                const auto collections = cache->collections(Akonadi::StorageInterface::AllContent);
                return toCollectionNames(collections);
            }();
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
        auto job = storage.fetchItems(Akonadi::Collection(42));
        QVERIFY2(job->kjob()->exec(), job->kjob()->errorString().toUtf8().constData());

        // THEN
        const auto toItemIds = [](const Akonadi::Item::List &items) {
            auto res = QVector<Akonadi::Item::Id>();
            res.reserve(items.size());
            std::transform(items.cbegin(), items.cend(),
                           std::back_inserter(res),
                           std::mem_fn(&Akonadi::Item::id));
            std::sort(res.begin(), res.end());
            return res;
        };

        auto expectedIds = QVector<Akonadi::Item::Id>() << 42 << 45 << 52;
        {
            const auto itemFetchIds = [job, toItemIds]{
                return toItemIds(job->items());
            }();
            QCOMPARE(itemFetchIds, expectedIds);

            const auto itemCachedIds = [cache, toItemIds]{
                const auto items = cache->items(Akonadi::Collection(42));
                return toItemIds(items);
            }();
            QCOMPARE(itemCachedIds, expectedIds);
        }

        // WHEN (second time shouldn't hit the original storage)
        data.storageBehavior().setFetchItemsBehavior(42, AkonadiFakeStorageBehavior::EmptyFetch);
        data.storageBehavior().setFetchItemsErrorCode(42, 128);
        job = storage.fetchItems(Akonadi::Collection(42));
        QVERIFY2(job->kjob()->exec(), job->kjob()->errorString().toUtf8().constData());

        {
            const auto itemFetchIds = [job, toItemIds]{
                return toItemIds(job->items());
            }();
            QCOMPARE(itemFetchIds, expectedIds);

            const auto itemCachedIds = [cache, toItemIds]{
                const auto items = cache->items(Akonadi::Collection(42));
                return toItemIds(items);
            }();
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
        auto job = storage.fetchItem(Akonadi::Item(44));
        QVERIFY2(job->kjob()->exec(), job->kjob()->errorString().toUtf8().constData());

        // THEN
        const auto toItemIds = [](const Akonadi::Item::List &items) {
            auto res = QVector<Akonadi::Item::Id>();
            res.reserve(items.size());
            std::transform(items.cbegin(), items.cend(),
                           std::back_inserter(res),
                           std::mem_fn(&Akonadi::Item::id));
            std::sort(res.begin(), res.end());
            return res;
        };

        auto expectedIds = QVector<Akonadi::Item::Id>() << 44;
        {
            const auto itemFetchIds = [job, toItemIds]{
                return toItemIds(job->items());
            }();
            QCOMPARE(itemFetchIds, expectedIds);
            QVERIFY(!cache->item(44).isValid());
        }

        // WHEN (if collection is populated, shouldn't hit the original storage)
        job = storage.fetchItems(Akonadi::Collection(43));
        QVERIFY2(job->kjob()->exec(), job->kjob()->errorString().toUtf8().constData());

        data.storageBehavior().setFetchItemBehavior(44, AkonadiFakeStorageBehavior::EmptyFetch);
        data.storageBehavior().setFetchItemErrorCode(44, 128);
        job = storage.fetchItem(Akonadi::Item(44));
        QVERIFY2(job->kjob()->exec(), job->kjob()->errorString().toUtf8().constData());

        {
            const auto itemFetchIds = [job, toItemIds]{
                return toItemIds(job->items());
            }();
            QCOMPARE(itemFetchIds, expectedIds);
            QVERIFY(cache->item(44).isValid());
        }
    }

    void shouldCacheAllItemsPerTag()
    {
        // GIVEN
        AkonadiFakeData data;

        data.createCollection(GenCollection().withId(42).withName(QStringLiteral("42Col")).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withName(QStringLiteral("43Col")).withRootAsParent().withTaskContent());

        data.createTag(GenTag().withId(42).withName(QStringLiteral("42Plain")).asPlain());
        data.createTag(GenTag().withId(43).withName(QStringLiteral("43Context")).asContext());

        data.createItem(GenTodo().withId(42).withTitle(QStringLiteral("42Task")).withParent(42).withTags({42}));
        data.createItem(GenTodo().withId(45).withTitle(QStringLiteral("45Task")).withParent(42).withTags({42, 43}));
        data.createItem(GenTodo().withId(52).withTitle(QStringLiteral("52Task")).withParent(42).withTags({43}));

        data.createItem(GenTodo().withId(44).withTitle(QStringLiteral("44Task")).withParent(43).withTags({42}));
        data.createItem(GenTodo().withId(48).withTitle(QStringLiteral("48Task")).withParent(43).withTags({42, 43}));
        data.createItem(GenTodo().withId(50).withTitle(QStringLiteral("50Task")).withParent(43).withTags({43}));

        auto cache = Akonadi::Cache::Ptr::create(Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                 Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        Akonadi::CachingStorage storage(cache, Akonadi::StorageInterface::Ptr(data.createStorage()));

        // WHEN
        auto job = storage.fetchTagItems(Akonadi::Tag(43));
        QVERIFY2(job->kjob()->exec(), job->kjob()->errorString().toUtf8().constData());

        // THEN
        const auto toItemIds = [](const Akonadi::Item::List &items) {
            auto res = QVector<Akonadi::Item::Id>();
            res.reserve(items.size());
            std::transform(items.cbegin(), items.cend(),
                           std::back_inserter(res),
                           std::mem_fn(&Akonadi::Item::id));
            std::sort(res.begin(), res.end());
            return res;
        };

        auto expectedIds = QVector<Akonadi::Item::Id>() << 45 << 48 << 50 << 52;
        {
            const auto itemFetchIds = [job, toItemIds]{
                return toItemIds(job->items());
            }();
            QCOMPARE(itemFetchIds, expectedIds);

            const auto itemCachedIds = [cache, toItemIds]{
                const auto items = cache->items(Akonadi::Tag(43));
                return toItemIds(items);
            }();
            QCOMPARE(itemCachedIds, expectedIds);
        }

        // WHEN (second time shouldn't hit the original storage)
        data.storageBehavior().setFetchTagItemsBehavior(43, AkonadiFakeStorageBehavior::EmptyFetch);
        data.storageBehavior().setFetchTagItemsErrorCode(43, 128);
        job = storage.fetchTagItems(Akonadi::Tag(43));
        QVERIFY2(job->kjob()->exec(), job->kjob()->errorString().toUtf8().constData());

        {
            const auto itemFetchIds = [job, toItemIds]{
                return toItemIds(job->items());
            }();
            QCOMPARE(itemFetchIds, expectedIds);

            const auto itemCachedIds = [cache, toItemIds]{
                const auto items = cache->items(Akonadi::Tag(43));
                return toItemIds(items);
            }();
            QCOMPARE(itemCachedIds, expectedIds);
        }
    }

    void shouldCacheTags()
    {
        // GIVEN
        AkonadiFakeData data;

        data.createTag(GenTag().withId(42).withName(QStringLiteral("42Plain")).asPlain());
        data.createTag(GenTag().withId(43).withName(QStringLiteral("43Context")).asContext());
        data.createTag(GenTag().withId(44).withName(QStringLiteral("44Plain")).asPlain());

        auto cache = Akonadi::Cache::Ptr::create(Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer),
                                                 Akonadi::MonitorInterface::Ptr(data.createMonitor()));
        Akonadi::CachingStorage storage(cache, Akonadi::StorageInterface::Ptr(data.createStorage()));

        // WHEN
        auto job = storage.fetchTags();
        QVERIFY2(job->kjob()->exec(), job->kjob()->errorString().toUtf8().constData());

        // THEN
        const auto toTagNames = [](const Akonadi::Tag::List &tags) {
            auto res = QStringList();
            res.reserve(tags.size());
            std::transform(tags.cbegin(), tags.cend(),
                           std::back_inserter(res),
                           std::mem_fn(&Akonadi::Tag::name));
            res.sort();
            return res;
        };

        auto expectedNames = QStringList() << "42Plain" << "43Context" << "44Plain";

        {
            const auto tagFetchNames = [job, toTagNames]{
                return toTagNames(job->tags());
            }();
            QCOMPARE(tagFetchNames, expectedNames);

            const auto tagCachedNames = [cache, toTagNames]{
                const auto tags = cache->tags();
                return toTagNames(tags);
            }();
            QCOMPARE(tagCachedNames, expectedNames);
        }

        // WHEN (second time shouldn't hit the original storage)
        data.storageBehavior().setFetchTagsBehavior(AkonadiFakeStorageBehavior::EmptyFetch);
        data.storageBehavior().setFetchTagsErrorCode(128);
        job = storage.fetchTags();
        QVERIFY2(job->kjob()->exec(), job->kjob()->errorString().toUtf8().constData());

        {
            const auto tagFetchNames = [job, toTagNames]{
                return toTagNames(job->tags());
            }();
            QCOMPARE(tagFetchNames, expectedNames);

            const auto tagCachedNames = [cache, toTagNames]{
                const auto tags = cache->tags();
                return toTagNames(tags);
            }();
            QCOMPARE(tagCachedNames, expectedNames);
        }
    }
};

ZANSHIN_TEST_MAIN(AkonadiCachingStorageTest)

#include "akonadicachingstoragetest.moc"
