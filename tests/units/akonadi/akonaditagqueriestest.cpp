/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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

#include <QtTest>

#include <mockitopp/mockitopp.hpp>

#include "testlib/akonadimocks.h"

#include "akonadi/akonaditagqueries.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;

Q_DECLARE_METATYPE(MockItemFetchJob*)
Q_DECLARE_METATYPE(MockCollectionFetchJob*)

class AkonadiTagQueriesTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldLookInAllReportedForAllTag()
    {
        // GIVEN

        // Two tag with their corresponding akonadi plain tag !
        Akonadi::Tag akonadiTag1("tag42");
        akonadiTag1.setId(Akonadi::Tag::Id(42));
        // domain tag
        Domain::Tag::Ptr tag1(new Domain::Tag);
        tag1->setName("tag42");

        Akonadi::Tag akonadiTag2("tag43");
        akonadiTag2.setId(Akonadi::Tag::Id(43));
        // domain tag
        Domain::Tag::Ptr tag2(new Domain::Tag);
        tag2->setName("tag43");

        MockTagFetchJob *tagFetchJob = new MockTagFetchJob(this);
        tagFetchJob->setTags(Akonadi::Tag::List() << akonadiTag1 << akonadiTag2);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchTags).when().thenReturn(tagFetchJob);

        // Serializer mock returning tags from tags
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag1).thenReturn(tag1);
        serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag2).thenReturn(tag2);

        // WHEN
        QScopedPointer<Domain::TagQueries> queries(new Akonadi::TagQueries(&storageMock.getInstance(),
                                                                                   &serializerMock.getInstance(),
                                                                                   new MockMonitor(this)));

        Domain::QueryResult<Domain::Tag::Ptr>::Ptr result = queries->findAll();
        result->data();
        result = queries->findAll(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);

        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchTags).when().exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTagFromAkonadiTag).when(akonadiTag2).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), tag1);
        QCOMPARE(result->data().at(1), tag2);

    }
};

QTEST_MAIN(AkonadiTagQueriesTest)

#include "akonaditagqueriestest.moc"
