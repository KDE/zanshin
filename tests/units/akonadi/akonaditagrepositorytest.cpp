/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Franck Arrecot <franck.arrecot@gmail.com>

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

#include "akonadi/akonaditagrepository.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;
using namespace mockitopp::matcher;

Q_DECLARE_METATYPE(MockItemFetchJob*)
Q_DECLARE_METATYPE(MockTagFetchJob*)

class AkonadiTagRepositoryTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldCreateTag()
    {
        // GIVEN

        // A Tag and its corresponding Akonadi Tag
        Akonadi::Tag akonadiTag; // not existing yet
        auto tag = Domain::Tag::Ptr::create();

        // A mock creating job
        auto tagCreateJob = new MockAkonadiJob(this);

        // Storage mock returning the tagCreatejob
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::createTag).when(akonadiTag)
                                                          .thenReturn(tagCreateJob);

        // Serializer mock
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createAkonadiTagFromTag).when(tag).thenReturn(akonadiTag);


        // WHEN
        QScopedPointer<Akonadi::TagRepository> repository(new Akonadi::TagRepository(&storageMock.getInstance(), &serializerMock.getInstance()));
        repository->create(tag)->exec();

        //THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::createTag).when(akonadiTag).exactly(1));
    }

    void shouldRemoveTag()
    {
        // GIVEN
        Akonadi::Tag akonadiTag;
        auto tag = Domain::Tag::Ptr::create();
        tag->setProperty("tagId", qint64(42)); // must be set
        tag->setName("42");

        // A mock of removal job
        auto tagRemoveJob = new MockAkonadiJob(this);

        // Storage mock returning the tagCreatejob
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::removeTag).when(akonadiTag)
                                                          .thenReturn(tagRemoveJob);
        // Serializer mock
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createAkonadiTagFromTag).when(tag).thenReturn(akonadiTag);

        // WHEN
        QScopedPointer<Akonadi::TagRepository> repository(new Akonadi::TagRepository(&storageMock.getInstance(), &serializerMock.getInstance()));
        repository->remove(tag)->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::removeTag).when(akonadiTag).exactly(1));
    }
};

QTEST_MAIN(AkonadiTagRepositoryTest)

#include "akonaditagrepositorytest.moc"
