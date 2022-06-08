/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include "testlib/akonadifakedata.h"
#include "testlib/akonadifakedataxmlloader.h"
#include <akonadi/akonadiserializer.h>

#include <KCalCore/Todo>

#include <testlib/qtest_zanshin.h>

using Akonadi::Serializer;

class AkonadiFakeDataXmlLoaderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void shouldLoadFromXmlFile()
    {
        // GIVEN
        auto data = Testlib::AkonadiFakeData();

        // Put a first collection with an idea to make sure it isn't lost on loading
        auto searchCollection = Akonadi::Collection(1);
        searchCollection.setParentCollection(Akonadi::Collection::root());
        searchCollection.setName(QStringLiteral("Search"));
        data.createCollection(searchCollection);

        auto loader = Testlib::AkonadiFakeDataXmlLoader(&data);

        // WHEN
        loader.load(SOURCE_DIR "/../akonadi/testenv/data/testdata.xml");

        // THEN
        QVERIFY(data.collections().size() > 1);
        QVERIFY(data.items().size() > 1);
        QVERIFY(data.contexts().size() > 1);

        // Do a bit of sanity checking, no need to do much more than that as
        // the xml loading will be extensively used in the AkonadiFakeStorageTest
        // and AkonadiFakeData has quite a few asserts.

        QCOMPARE(data.childCollections(Akonadi::Collection::root().id()).size(), 2);
        const auto res = data.childCollections(Akonadi::Collection::root().id()).at(1);
        QCOMPARE(res.id(), qint64(2));

        const auto children = data.childCollections(res.id());
        QCOMPARE(children.size(), 5);
        QCOMPARE(children.at(0).name(), QStringLiteral("Calendar1"));
        QCOMPARE(children.at(0).id(), qint64(3));
        QCOMPARE(children.at(0).remoteId(), QStringLiteral("{cdc229c7-a9b5-4d37-989d-a28e372be2a9}"));
        QCOMPARE(children.at(1).name(), QStringLiteral("Emails"));
        QCOMPARE(children.at(1).id(), qint64(4));
        QCOMPARE(children.at(1).remoteId(), QStringLiteral("{14096930-7bfe-46ca-8fba-7c04d3b62ec8}"));
        QCOMPARE(children.at(2).name(), QStringLiteral("Contacts"));
        QCOMPARE(children.at(2).id(), qint64(5));
        QCOMPARE(children.at(2).remoteId(), QStringLiteral("{36bf43ac-0988-4435-b602-d6c29e606630}"));
        QCOMPARE(children.at(3).name(), QStringLiteral("Destroy me!"));
        QCOMPARE(children.at(3).id(), qint64(6));
        QCOMPARE(children.at(3).remoteId(), QStringLiteral("{1f78b360-a01b-4785-9187-75450190342c}"));
        QCOMPARE(children.at(4).name(), QStringLiteral("Change me!"));
        QCOMPARE(children.at(4).id(), qint64(7));
        QCOMPARE(children.at(4).remoteId(), QStringLiteral("{28ef9f03-4ebc-4e33-970f-f379775894f9}"));

        const auto firstContext = data.contexts().at(0);
        QCOMPARE(firstContext.remoteId(), "rid-online-context");
        const auto contextAsTodo = firstContext.payload<KCalCore::Todo::Ptr>();
        QCOMPARE(contextAsTodo->uid(), "online-context");
        QCOMPARE(contextAsTodo->summary(), "Online");
        QCOMPARE(contextAsTodo->customProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsContext()), "1");
    }
};

ZANSHIN_TEST_MAIN(AkonadiFakeDataXmlLoaderTest)

#include "akonadifakedataxmlloadertest.moc"
