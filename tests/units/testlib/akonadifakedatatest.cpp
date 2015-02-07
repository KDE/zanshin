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

uint qHash(const Akonadi::Collection &col)
{
    return qHash(col.id());
}

// More aggressive equal to make sure we just don't get collections with ids out
bool operator==(const Akonadi::Collection &left, const Akonadi::Collection &right)
{
    return left.operator==(right)
        && left.displayName() == right.displayName();
}

// More aggressive equal to make sure we just don't get items with ids out
bool operator==(const Akonadi::Item &left, const Akonadi::Item &right)
{
    return left.operator==(right)
        && left.payloadData() == right.payloadData();
}

#include <QtTest>


class AkonadiFakeDataTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldBeInitiallyEmpty()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();

        // THEN
        QVERIFY(data.collections().isEmpty());
        QVERIFY(data.items().isEmpty());
    }

    void shouldCreateCollections()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        auto c1 = Akonadi::Collection(42);
        c1.setName("42");
        auto c2 = Akonadi::Collection(43);
        c2.setName("43");
        const auto colSet = QSet<Akonadi::Collection>() << c1 << c2;

        // WHEN
        data.createCollection(c1);
        data.createCollection(c2);

        // THEN
        QCOMPARE(data.collections().toSet(), colSet);
        QCOMPARE(data.collection(c1.id()), c1);
        QCOMPARE(data.collection(c2.id()), c2);
    }

    void shouldModifyCollections()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        auto c1 = Akonadi::Collection(42);
        c1.setName("42");
        data.createCollection(c1);

        auto c2 = Akonadi::Collection(c1.id());
        c2.setName("42-bis");

        // WHEN
        data.modifyCollection(c2);

        // THEN
        QCOMPARE(data.collections().size(), 1);
        QCOMPARE(data.collection(c1.id()), c2);
    }

    void shouldListChildCollections()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        auto c1 = Akonadi::Collection(42);
        c1.setName("42");
        auto c2 = Akonadi::Collection(43);
        c2.setName("43");
        c2.setParentCollection(Akonadi::Collection(42));
        const auto colSet = QSet<Akonadi::Collection>() << c2;

        // WHEN
        data.createCollection(c1);
        data.createCollection(c2);

        // THEN
        QVERIFY(data.childCollections(c2.id()).isEmpty());
        QCOMPARE(data.childCollections(c1.id()).toSet(), colSet);
    }

    void shouldReparentCollectionsOnModify()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();

        auto c1 = Akonadi::Collection(42);
        c1.setName("42");
        data.createCollection(c1);

        auto c2 = Akonadi::Collection(43);
        c2.setName("43");
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
        QCOMPARE(data.childCollections(c2.id()).first(), c3);
    }

    void shouldCreateItems()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        auto i1 = Akonadi::Item(42);
        i1.setPayloadFromData("42");
        auto i2 = Akonadi::Item(43);
        i2.setPayloadFromData("43");
        const auto itemSet = QSet<Akonadi::Item>() << i1 << i2;

        // WHEN
        data.createItem(i1);
        data.createItem(i2);

        // THEN
        QCOMPARE(data.items().toSet(), itemSet);
        QCOMPARE(data.item(i1.id()), i1);
        QCOMPARE(data.item(i2.id()), i2);
    }

    void shouldListChildItems()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();
        auto c1 = Akonadi::Collection(42);
        c1.setName("42");
        data.createCollection(c1);

        auto i1 = Akonadi::Item(42);
        i1.setPayloadFromData("42");
        i1.setParentCollection(Akonadi::Collection(42));

        // WHEN
        data.createItem(i1);

        // THEN
        QCOMPARE(data.childItems(c1.id()).size(), 1);
        QCOMPARE(data.childItems(c1.id()).first(), i1);
    }
};

QTEST_MAIN(AkonadiFakeDataTest)

#include "akonadifakedatatest.moc"
