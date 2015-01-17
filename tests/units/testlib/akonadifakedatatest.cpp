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
};

QTEST_MAIN(AkonadiFakeDataTest)

#include "akonadifakedatatest.moc"
