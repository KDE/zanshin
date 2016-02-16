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

#include "testlib/akonadifakedata.h"
#include "akonadi/akonadimonitorinterface.h"

#include <testlib/qtest_zanshin.h>

namespace QTest {

template<typename T>
inline bool zCompareHelper(bool isOk,
                           const T &left, const T &right,
                           const char *actual, const char *expected,
                           const char *file, int line)
{
    return compare_helper(isOk, isOk ? "COMPARE()" : "Compared values are not the same",
                          toString<T>(left), toString<T>(right),
                          actual, expected,
                          file, line);
}

// More aggressive compare to make sure we just don't get collections with ids out
template <>
inline bool qCompare(const Akonadi::Collection &left, const Akonadi::Collection &right,
                     const char *actual, const char *expected,
                     const char *file, int line)
{
    return zCompareHelper((left == right) && (left.displayName() == right.displayName()),
                          left, right, actual, expected, file, line);
}

// More aggressive compare to make sure we just don't get tags with ids out
template <>
inline bool qCompare(const Akonadi::Tag &left, const Akonadi::Tag &right,
                     const char *actual, const char *expected,
                     const char *file, int line)
{
    return zCompareHelper((left == right) && (left.name() == right.name()),
                          left, right, actual, expected, file, line);
}

// More aggressive compare to make sure we just don't get items with ids out
template <>
inline bool qCompare(const Akonadi::Item &left, const Akonadi::Item &right,
                     const char *actual, const char *expected,
                     const char *file, int line)
{
    return zCompareHelper((left == right) && (left.payloadData() == right.payloadData()),
                          left, right, actual, expected, file, line);
}

}

class AkonadiFakeDataTest : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiFakeDataTest(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
        qRegisterMetaType<Akonadi::Collection>();
        qRegisterMetaType<Akonadi::Tag>();
        qRegisterMetaType<Akonadi::Item>();
    }

private slots:
    void shouldBeInitiallyEmpty()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();

        // THEN
        QVERIFY(data.collections().isEmpty());
        QVERIFY(data.tags().isEmpty());
        QVERIFY(data.items().isEmpty());
    }

    void shouldCreateCollections()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        QScopedPointer<Akonadi::MonitorInterface> monitor(data.createMonitor());
        QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::collectionAdded);

        auto c1 = Akonadi::Collection(42);
        c1.setName(QStringLiteral("42"));
        auto c2 = Akonadi::Collection(43);
        c2.setName(QStringLiteral("43"));
        const auto colSet = QSet<Akonadi::Collection>() << c1 << c2;

        // WHEN
        data.createCollection(c1);
        data.createCollection(c2);

        // THEN
        QCOMPARE(data.collections().toList().toSet(), colSet);
        QCOMPARE(data.collection(c1.id()), c1);
        QCOMPARE(data.collection(c2.id()), c2);

        QCOMPARE(spy.size(), 2);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Collection>(), c1);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Collection>(), c2);
    }

    void shouldModifyCollections()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        QScopedPointer<Akonadi::MonitorInterface> monitor(data.createMonitor());
        QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::collectionChanged);

        auto c1 = Akonadi::Collection(42);
        c1.setName(QStringLiteral("42"));
        data.createCollection(c1);

        auto c2 = Akonadi::Collection(c1.id());
        c2.setName(QStringLiteral("42-bis"));

        // WHEN
        data.modifyCollection(c2);

        // THEN
        QCOMPARE(data.collections().size(), 1);
        QCOMPARE(data.collection(c1.id()), c2);

        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Collection>(), c2);
    }

    void shouldNotLooseParentCollectionOnModifyCollection()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();

        auto root = Akonadi::Collection(42);
        root.setName(QStringLiteral("root"));
        data.createCollection(root);

        auto c1 = Akonadi::Collection(43);
        c1.setName(QStringLiteral("43"));
        c1.setParentCollection(Akonadi::Collection(root.id()));
        data.createCollection(c1);

        auto c2 = Akonadi::Collection(c1.id());
        c2.setName(QStringLiteral("43-bis"));

        // WHEN
        data.modifyCollection(c2);

        // THEN
        QCOMPARE(data.collections().size(), 2);
        QCOMPARE(data.collection(c1.id()), c2);
        QCOMPARE(data.collection(c1.id()).parentCollection().id(), root.id());
    }

    void shouldListChildCollections()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        auto c1 = Akonadi::Collection(42);
        c1.setName(QStringLiteral("42"));
        auto c2 = Akonadi::Collection(43);
        c2.setName(QStringLiteral("43"));
        c2.setParentCollection(Akonadi::Collection(42));
        const auto colSet = QSet<Akonadi::Collection>() << c2;

        // WHEN
        data.createCollection(c1);
        data.createCollection(c2);

        // THEN
        QVERIFY(data.childCollections(c2.id()).isEmpty());
        QCOMPARE(data.childCollections(c1.id()).toList().toSet(), colSet);
    }

    void shouldReparentCollectionsOnModify()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();

        auto c1 = Akonadi::Collection(42);
        c1.setName(QStringLiteral("42"));
        data.createCollection(c1);

        auto c2 = Akonadi::Collection(43);
        c2.setName(QStringLiteral("43"));
        data.createCollection(c2);

        auto c3 = Akonadi::Collection(44);
        c3.setParentCollection(Akonadi::Collection(42));
        data.createCollection(c3);

        // WHEN
        c3.setParentCollection(Akonadi::Collection(43));
        data.modifyCollection(c3);

        // THEN
        QVERIFY(data.childCollections(c1.id()).isEmpty());
        QCOMPARE(data.childCollections(c2.id()).size(), 1);
        QCOMPARE(data.childCollections(c2.id()).at(0), c3);
    }

    void shouldRemoveCollections()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        QScopedPointer<Akonadi::MonitorInterface> monitor(data.createMonitor());
        QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::collectionRemoved);

        auto c1 = Akonadi::Collection(42);
        c1.setName(QStringLiteral("42"));
        data.createCollection(c1);

        auto c2 = Akonadi::Collection(43);
        c2.setName(QStringLiteral("43"));
        c2.setParentCollection(Akonadi::Collection(42));
        data.createCollection(c2);

        auto c3 = Akonadi::Collection(44);
        c3.setName(QStringLiteral("44"));
        c3.setParentCollection(Akonadi::Collection(43));
        data.createCollection(c3);

        auto i1 = Akonadi::Item(42);
        i1.setPayloadFromData("42");
        i1.setParentCollection(Akonadi::Collection(43));
        data.createItem(i1);

        auto i2 = Akonadi::Item(43);
        i2.setPayloadFromData("43");
        i2.setParentCollection(Akonadi::Collection(44));
        data.createItem(i2);

        // WHEN
        data.removeCollection(c2);

        // THEN
        QCOMPARE(data.collections().size(), 1);
        QCOMPARE(data.collections().at(0), c1);

        QVERIFY(!data.collection(c2.id()).isValid());
        QVERIFY(!data.collection(c3.id()).isValid());

        QVERIFY(data.childCollections(c1.id()).isEmpty());
        QVERIFY(data.childCollections(c2.id()).isEmpty());
        QVERIFY(data.childCollections(c3.id()).isEmpty());

        QVERIFY(data.items().isEmpty());

        QVERIFY(!data.item(i1.id()).isValid());
        QVERIFY(!data.item(i2.id()).isValid());

        QVERIFY(data.childItems(c2.id()).isEmpty());
        QVERIFY(data.childItems(c3.id()).isEmpty());

        QCOMPARE(spy.size(), 2);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Collection>(), c3);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Collection>(), c2);
    }

    void shouldCreateTags()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        QScopedPointer<Akonadi::MonitorInterface> monitor(data.createMonitor());
        QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::tagAdded);

        auto t1 = Akonadi::Tag(42);
        t1.setName(QStringLiteral("42"));
        auto t2 = Akonadi::Tag(43);
        t2.setName(QStringLiteral("43"));

        // WHEN
        data.createTag(t1);
        data.createTag(t2);

        // THEN
        QCOMPARE(data.tags().size(), 2);
        QVERIFY(data.tags().contains(t1));
        QVERIFY(data.tags().contains(t2));
        QCOMPARE(data.tag(t1.id()), t1);
        QCOMPARE(data.tag(t2.id()), t2);

        QCOMPARE(spy.size(), 2);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Tag>(), t1);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Tag>(), t2);
    }

    void shouldModifyTags()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        QScopedPointer<Akonadi::MonitorInterface> monitor(data.createMonitor());
        QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::tagChanged);

        auto t1 = Akonadi::Tag(42);
        t1.setName(QStringLiteral("42"));
        data.createTag(t1);

        auto t2 = Akonadi::Tag(t1.id());
        t2.setName(QStringLiteral("42-bis"));

        // WHEN
        data.modifyTag(t2);

        // THEN
        QCOMPARE(data.tags().size(), 1);
        QCOMPARE(data.tag(t1.id()), t2);

        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Tag>(), t2);
    }

    void shouldRemoveTags()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        QScopedPointer<Akonadi::MonitorInterface> monitor(data.createMonitor());
        QSignalSpy tagSpy(monitor.data(), &Akonadi::MonitorInterface::tagRemoved);
        QSignalSpy itemSpy(monitor.data(), &Akonadi::MonitorInterface::itemChanged);

        auto c1 = Akonadi::Collection(42);
        data.createCollection(c1);

        auto t1 = Akonadi::Tag(42);
        t1.setName(QStringLiteral("42"));
        data.createTag(t1);

        auto t2 = Akonadi::Tag(43);
        t2.setName(QStringLiteral("43"));
        data.createTag(t2);

        auto i1 = Akonadi::Item(42);
        i1.setPayloadFromData("42");
        i1.setParentCollection(c1);
        i1.setTag(Akonadi::Tag(t1.id()));
        data.createItem(i1);

        auto i2 = Akonadi::Item(43);
        i2.setPayloadFromData("43");
        i2.setParentCollection(c1);
        i2.setTag(Akonadi::Tag(t2.id()));
        data.createItem(i2);

        const auto itemSet = QSet<Akonadi::Item>() << i1 << i2;

        // WHEN
        data.removeTag(t2);

        // THEN
        QCOMPARE(data.tags().size(), 1);
        QCOMPARE(data.tags().at(0), t1);

        QVERIFY(!data.tag(t2.id()).isValid());

        QCOMPARE(data.tagItems(t1.id()).size(), 1);
        QCOMPARE(data.tagItems(t1.id()).at(0), i1);
        QVERIFY(data.tagItems(t2.id()).isEmpty());

        QCOMPARE(data.items().toList().toSet(), itemSet);

        QVERIFY(data.item(i1.id()).isValid());
        QVERIFY(data.item(i2.id()).isValid());
        QVERIFY(!data.item(i2.id()).tags().contains(t2));

        QCOMPARE(tagSpy.size(), 1);
        QCOMPARE(tagSpy.takeFirst().at(0).value<Akonadi::Tag>(), t2);

        QCOMPARE(itemSpy.size(), 1);
        QCOMPARE(itemSpy.first().at(0).value<Akonadi::Item>(), i2);
        QVERIFY(!itemSpy.first().at(0).value<Akonadi::Item>().tags().contains(t2));
    }

    void shouldCreateItems()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        QScopedPointer<Akonadi::MonitorInterface> monitor(data.createMonitor());
        QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemAdded);

        auto i1 = Akonadi::Item(42);
        i1.setPayloadFromData("42");
        auto i2 = Akonadi::Item(43);
        i2.setPayloadFromData("43");
        const auto itemSet = QSet<Akonadi::Item>() << i1 << i2;

        // WHEN
        data.createItem(i1);
        data.createItem(i2);

        // THEN
        QCOMPARE(data.items().toList().toSet(), itemSet);
        QCOMPARE(data.item(i1.id()), i1);
        QCOMPARE(data.item(i2.id()), i2);

        QCOMPARE(spy.size(), 2);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Item>(), i1);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Item>(), i2);
    }

    void shouldModifyItems()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        QScopedPointer<Akonadi::MonitorInterface> monitor(data.createMonitor());
        QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemChanged);
        QSignalSpy moveSpy(monitor.data(), &Akonadi::MonitorInterface::itemMoved);

        auto c1 = Akonadi::Collection(42);
        c1.setName(QStringLiteral("42"));
        data.createCollection(c1);

        auto i1 = Akonadi::Item(42);
        i1.setPayloadFromData("42");
        i1.setParentCollection(Akonadi::Collection(42));
        data.createItem(i1);

        auto i2 = Akonadi::Item(i1.id());
        i2.setPayloadFromData("42-bis");
        i2.setParentCollection(Akonadi::Collection(42));

        // WHEN
        data.modifyItem(i2);

        // THEN
        QCOMPARE(data.items().size(), 1);
        QCOMPARE(data.item(i1.id()), i2);

        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Item>(), i2);

        QCOMPARE(moveSpy.size(), 0);
    }

    void shouldNotLooseParentCollectionOnModifyItem()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();

        auto c1 = Akonadi::Collection(42);
        c1.setName(QStringLiteral("42"));
        data.createCollection(c1);

        auto i1 = Akonadi::Item(42);
        i1.setPayloadFromData("42");
        i1.setParentCollection(Akonadi::Collection(42));
        data.createItem(i1);

        auto i2 = Akonadi::Item(i1.id());
        i2.setPayloadFromData("42-bis");

        // WHEN
        data.modifyItem(i2);

        // THEN
        QCOMPARE(data.items().size(), 1);
        QCOMPARE(data.item(i1.id()), i2);
        QCOMPARE(data.item(i1.id()).parentCollection().id(), c1.id());
    }

    void shouldListChildItems()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        auto c1 = Akonadi::Collection(42);
        c1.setName(QStringLiteral("42"));
        data.createCollection(c1);

        auto i1 = Akonadi::Item(42);
        i1.setPayloadFromData("42");
        i1.setParentCollection(Akonadi::Collection(42));

        // WHEN
        data.createItem(i1);

        // THEN
        QCOMPARE(data.childItems(c1.id()).size(), 1);
        QCOMPARE(data.childItems(c1.id()).at(0), i1);
    }

    void shouldReparentItemsOnModify()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        QScopedPointer<Akonadi::MonitorInterface> monitor(data.createMonitor());
        QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemMoved);

        auto c1 = Akonadi::Collection(42);
        c1.setName(QStringLiteral("42"));
        data.createCollection(c1);

        auto c2 = Akonadi::Collection(43);
        c2.setName(QStringLiteral("43"));
        data.createCollection(c2);

        auto i1 = Akonadi::Item(42);
        i1.setPayloadFromData("42");
        i1.setParentCollection(Akonadi::Collection(42));
        data.createItem(i1);

        // WHEN
        i1.setPayloadFromData("42-bis");
        i1.setParentCollection(Akonadi::Collection(43));
        data.modifyItem(i1);

        // THEN
        QVERIFY(data.childItems(c1.id()).isEmpty());
        QCOMPARE(data.childItems(c2.id()).size(), 1);
        QCOMPARE(data.childItems(c2.id()).at(0), i1);

        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Item>(), i1);
    }

    void shouldListTagItems()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        auto t1 = Akonadi::Tag(42);
        t1.setName(QStringLiteral("42"));
        data.createTag(t1);

        auto i1 = Akonadi::Item(42);
        i1.setPayloadFromData("42");
        i1.setTag(Akonadi::Tag(42));

        // WHEN
        data.createItem(i1);

        // THEN
        QCOMPARE(data.tagItems(t1.id()).size(), 1);
        QCOMPARE(data.tagItems(t1.id()).at(0), i1);
    }

    void shouldRetagItemsOnModify()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        QScopedPointer<Akonadi::MonitorInterface> monitor(data.createMonitor());
        QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemChanged);

        auto t1 = Akonadi::Tag(42);
        t1.setName(QStringLiteral("42"));
        data.createTag(t1);

        auto t2 = Akonadi::Tag(43);
        t2.setName(QStringLiteral("43"));
        data.createTag(t2);

        auto i1 = Akonadi::Item(42);
        i1.setPayloadFromData("42");
        i1.setTag(Akonadi::Tag(42));
        data.createItem(i1);

        // WHEN
        i1.setPayloadFromData("42-bis");
        i1.clearTag(Akonadi::Tag(42));
        i1.setTag(Akonadi::Tag(43));
        data.modifyItem(i1);

        // THEN
        QVERIFY(data.tagItems(t1.id()).isEmpty());
        QCOMPARE(data.tagItems(t2.id()).size(), 1);
        QCOMPARE(data.tagItems(t2.id()).at(0), i1);

        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Item>(), i1);
    }

    void shouldRemoveItems()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        QScopedPointer<Akonadi::MonitorInterface> monitor(data.createMonitor());
        QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemRemoved);

        auto c1 = Akonadi::Collection(42);
        c1.setName(QStringLiteral("42"));
        data.createCollection(c1);

        auto i1 = Akonadi::Item(42);
        i1.setPayloadFromData("42");
        i1.setParentCollection(Akonadi::Collection(42));
        data.createItem(i1);

        // WHEN
        data.removeItem(i1);

        // THEN
        QVERIFY(data.items().isEmpty());
        QVERIFY(!data.item(i1.id()).isValid());
        QVERIFY(data.childItems(c1.id()).isEmpty());

        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Item>(), i1);
    }
};

ZANSHIN_TEST_MAIN(AkonadiFakeDataTest)

#include "akonadifakedatatest.moc"
