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
#include "testlib/akonadifakedataxmlloader.h"

#include <QtTest>

class AkonadiFakeDataXmlLoaderTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldLoadFromXmlFile()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();

        // Put a first collection with an idea to make sure it isn't lost on loading
        auto searchCollection = Akonadi::Collection(1);
        searchCollection.setParentCollection(Akonadi::Collection::root());
        searchCollection.setName("Search");
        data.createCollection(searchCollection);

        auto loader = Testlib::AkonadiFakeDataXmlLoader(&data);

        // WHEN
        loader.load(SOURCE_DIR "/akonadifakedataxmlloadertest.xml");

        // THEN
        QVERIFY(data.collections().size() > 1);
        QVERIFY(data.items().size() > 1);
        QVERIFY(data.tags().size() > 1);

        // Do a bit of sanity checking, no need to do much more than that as
        // the xml loading will be extensively used in the AkonadiFakeStorageTest
        // and AkonadiFakeData has quite a few asserts.

        QCOMPARE(data.childCollections(Akonadi::Collection::root().id()).size(), 2);
        const auto res = data.childCollections(Akonadi::Collection::root().id()).at(1);
        QCOMPARE(res.id(), qint64(2));

        const auto children = data.childCollections(res.id());
        QCOMPARE(children.size(), 5);
        QCOMPARE(children.at(0).name(), QString("Calendar1"));
        QCOMPARE(children.at(0).id(), qint64(3));
        QCOMPARE(children.at(0).remoteId(), QString("{cdc229c7-a9b5-4d37-989d-a28e372be2a9}"));
        QCOMPARE(children.at(1).name(), QString("Emails"));
        QCOMPARE(children.at(1).id(), qint64(4));
        QCOMPARE(children.at(1).remoteId(), QString("{14096930-7bfe-46ca-8fba-7c04d3b62ec8}"));
        QCOMPARE(children.at(2).name(), QString("Contacts"));
        QCOMPARE(children.at(2).id(), qint64(5));
        QCOMPARE(children.at(2).remoteId(), QString("{36bf43ac-0988-4435-b602-d6c29e606630}"));
        QCOMPARE(children.at(3).name(), QString("Destroy me!"));
        QCOMPARE(children.at(3).id(), qint64(6));
        QCOMPARE(children.at(3).remoteId(), QString("{1f78b360-a01b-4785-9187-75450190342c}"));
        QCOMPARE(children.at(4).name(), QString("Change me!"));
        QCOMPARE(children.at(4).id(), qint64(7));
        QCOMPARE(children.at(4).remoteId(), QString("{28ef9f03-4ebc-4e33-970f-f379775894f9}"));
    }
};

QTEST_MAIN(AkonadiFakeDataXmlLoaderTest)

#include "akonadifakedataxmlloadertest.moc"
