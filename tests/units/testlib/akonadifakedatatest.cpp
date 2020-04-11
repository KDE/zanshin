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
#include "akonadi/akonadiserializer.h"

#include <testlib/qtest_zanshin.h>

#include <akonadiserializer.h>

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

namespace
{
    template<typename T>
    QSet<T> listToSet(const QVector<T> &list)
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        return list.toList().toSet();
#else
        return {list.cbegin(), list.cend()};
#endif
    }
}

class AkonadiFakeDataTest : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiFakeDataTest(QObject *parent = nullptr)
        : QObject(parent)
    {
        qRegisterMetaType<Akonadi::Collection>();
        qRegisterMetaType<Akonadi::Item>();
    }

private slots:
    void shouldBeInitiallyEmpty()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();

        // THEN
        QVERIFY(data.collections().isEmpty());
        QVERIFY(data.contexts().isEmpty());
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
        QCOMPARE(listToSet(data.collections()), colSet);
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

    void shouldNotLoseParentCollectionOnModifyCollection()
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
        QCOMPARE(listToSet(data.childCollections(c1.id())), colSet);
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
        Akonadi::Serializer serializer;

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

        auto task1 = Domain::Task::Ptr::create();
        auto i1 = serializer.createItemFromTask(task1);
        i1.setId(42);
        i1.setParentCollection(Akonadi::Collection(43));
        data.createItem(i1);

        auto task2 = Domain::Task::Ptr::create();
        auto i2 = serializer.createItemFromTask(task2);
        i2.setId(43);
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

    void shouldCreateContexts()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        QScopedPointer<Akonadi::MonitorInterface> monitor(data.createMonitor());
        QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemAdded);
        Akonadi::Serializer serializer;

        auto context1 = Domain::Context::Ptr::create();
        context1->setName(QStringLiteral("42"));
        context1->setProperty("todoUid", "ctx-42");
        auto t1 = serializer.createItemFromContext(context1);

        auto context2 = Domain::Context::Ptr::create();
        context2->setName(QStringLiteral("43"));
        context2->setProperty("todoUid", "ctx-43");
        auto t2 = serializer.createItemFromContext(context2);

        // WHEN
        data.createContext(t1);
        data.createContext(t2);

        // THEN
        QCOMPARE(data.contexts().size(), 2);
        QVERIFY(data.contexts().contains(t1));
        QVERIFY(data.contexts().contains(t2));
        QCOMPARE(data.contextItem("ctx-42"), t1);
        QCOMPARE(data.contextItem("ctx-43"), t2);

        QCOMPARE(spy.size(), 2);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Item>(), t1);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Item>(), t2);
    }

    void shouldModifyContexts()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        QScopedPointer<Akonadi::MonitorInterface> monitor(data.createMonitor());
        QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemChanged);
        Akonadi::Serializer serializer;

        auto context1 = Domain::Context::Ptr::create();
        context1->setName(QStringLiteral("42"));
        context1->setProperty("todoUid", "ctx-42");
        auto t1 = serializer.createItemFromContext(context1);
        data.createContext(t1);

        auto context2 = Domain::Context::Ptr::create();
        context2->setName(QStringLiteral("42-bis"));
        context2->setProperty("todoUid", "ctx-42");
        auto t2 = serializer.createItemFromContext(context2);

        // WHEN
        data.modifyContext(t2);

        // THEN
        QCOMPARE(data.contexts().size(), 1);
        QCOMPARE(data.contextItem("ctx-42"), t2);

        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Item>(), t2);
    }

    void shouldRemoveContexts()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        QScopedPointer<Akonadi::MonitorInterface> monitor(data.createMonitor());
        QSignalSpy contextSpy(monitor.data(), &Akonadi::MonitorInterface::itemRemoved);
        QSignalSpy itemSpy(monitor.data(), &Akonadi::MonitorInterface::itemChanged);
        Akonadi::Serializer serializer;

        auto c1 = Akonadi::Collection(42);
        data.createCollection(c1);

        auto context1 = Domain::Context::Ptr::create();
        context1->setName(QStringLiteral("1"));
        context1->setProperty("todoUid", "ctx-1");
        auto t1 = serializer.createItemFromContext(context1);
        data.createContext(t1);

        auto context2 = Domain::Context::Ptr::create();
        context2->setName(QStringLiteral("2"));
        context2->setProperty("todoUid", "ctx-2");
        auto t2 = serializer.createItemFromContext(context2);
        data.createContext(t2);

        auto task1 = Domain::Task::Ptr::create();
        auto i1 = serializer.createItemFromTask(task1);
        i1.setParentCollection(c1);
        i1.setId(42);
        serializer.addContextToTask(context1, i1);
        data.createItem(i1);
        QVERIFY(serializer.isContextChild(context1, i1));

        auto task2 = Domain::Task::Ptr::create();
        auto i2 = serializer.createItemFromTask(task2);
        i2.setParentCollection(c1);
        i2.setId(43);
        serializer.addContextToTask(context2, i2);
        data.createItem(i2);
        QVERIFY(serializer.isContextChild(context2, i2));

        const auto itemSet = QSet<Akonadi::Item>() << i1 << i2;

        // WHEN
        data.removeContext(t2);

        // THEN
        QCOMPARE(data.contexts().size(), 1);
        QCOMPARE(data.contexts().at(0), t1);

        QVERIFY(!data.contextItem("ctx-2").isValid());

        QCOMPARE(data.contextItems("ctx-1").size(), 1);
        QCOMPARE(data.contextItems("ctx-1").at(0), i1);
        QVERIFY(data.contextItems("ctx-2").isEmpty());

        QCOMPARE(listToSet(data.items()), itemSet);

        QVERIFY(data.item(i1.id()).isValid());
        const auto item2 = data.item(i2.id());
        QVERIFY(item2.isValid());
        QVERIFY(!serializer.isContextChild(context2, item2));

        QCOMPARE(contextSpy.size(), 1);
        QCOMPARE(contextSpy.takeFirst().at(0).value<Akonadi::Item>().id(), t2.id());

        QCOMPARE(itemSpy.size(), 1);
        const auto emittedItem2 = itemSpy.first().at(0).value<Akonadi::Item>();
        QCOMPARE(emittedItem2, i2);
        QVERIFY(!serializer.isContextChild(context2, emittedItem2));
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
        QCOMPARE(listToSet(data.items()), itemSet);
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

    void shouldNotLoseParentCollectionOnModifyItem()
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

    void shouldListContextItems()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        Akonadi::Serializer serializer;

        auto context1 = Domain::Context::Ptr::create();
        context1->setName(QStringLiteral("42"));
        context1->setProperty("todoUid", "ctx-42");
        auto t1 = serializer.createItemFromContext(context1);
        data.createContext(t1);

        auto task1 = Domain::Task::Ptr::create();
        auto i1 = serializer.createItemFromTask(task1);
        i1.setId(1);
        serializer.addContextToTask(context1, i1);

        // WHEN
        data.createItem(i1);

        // THEN
        QCOMPARE(data.contextItems("ctx-42").size(), 1);
        QCOMPARE(data.contextItems("ctx-42").at(0), i1);
    }

    void shouldRetagItemsOnModify()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        QScopedPointer<Akonadi::MonitorInterface> monitor(data.createMonitor());
        QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemChanged);
        Akonadi::Serializer serializer;

        auto context1 = Domain::Context::Ptr::create();
        context1->setName(QStringLiteral("42"));
        context1->setProperty("todoUid", "ctx-42");
        auto t1 = serializer.createItemFromContext(context1);
        data.createContext(t1);

        auto context2 = Domain::Context::Ptr::create();
        context2->setName(QStringLiteral("43"));
        context2->setProperty("todoUid", "ctx-43");
        auto t2 = serializer.createItemFromContext(context2);
        data.createContext(t2);

        auto task1 = Domain::Task::Ptr::create();
        auto i1 = serializer.createItemFromTask(task1);
        i1.setId(1);
        serializer.addContextToTask(context1, i1);
        data.createItem(i1);

        // WHEN
        auto i2 = serializer.createItemFromTask(task1);
        i2.setId(1);
        serializer.addContextToTask(context2, i2);
        data.modifyItem(i2);

        // THEN
        QVERIFY(data.contextItems("ctx-42").isEmpty());
        QCOMPARE(data.contextItems("ctx-43").size(), 1);
        QCOMPARE(data.contextItems("ctx-43").at(0), i2);

        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Item>(), i2);
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
        QCOMPARE(spy.takeFirst().at(0).value<Akonadi::Item>().id(), i1.id());
    }
};

ZANSHIN_TEST_MAIN(AkonadiFakeDataTest)

#include "akonadifakedatatest.moc"
