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

#include "utils/mockobject.h"

#include "testlib/akonadifakejobs.h"
#include "testlib/akonadifakemonitor.h"

#include "akonadi/akonaditagrepository.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;
using namespace mockitopp::matcher;

Q_DECLARE_METATYPE(Testlib::AkonadiFakeItemFetchJob*)
Q_DECLARE_METATYPE(Testlib::AkonadiFakeTagFetchJob*)

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
        auto tagCreateJob = new FakeJob(this);

        // Storage mock returning the tagCreatejob
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::createTag).when(akonadiTag)
                                                          .thenReturn(tagCreateJob);

        // Serializer mock
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createAkonadiTagFromTag).when(tag).thenReturn(akonadiTag);


        // WHEN
        QScopedPointer<Akonadi::TagRepository> repository(new Akonadi::TagRepository(storageMock.getInstance(),
                                                                                     serializerMock.getInstance()));
        repository->create(tag)->exec();

        //THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::createTag).when(akonadiTag).exactly(1));
    }

    void shouldRemoveTag()
    {
        // GIVEN
        Akonadi::Tag akonadiTag(42);
        auto tag = Domain::Tag::Ptr::create();
        tag->setProperty("tagId", qint64(42)); // must be set
        tag->setName("42");

        // A mock of removal job
        auto tagRemoveJob = new FakeJob(this);

        // Storage mock returning the tagCreatejob
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::removeTag).when(akonadiTag)
                                                          .thenReturn(tagRemoveJob);
        // Serializer mock
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createAkonadiTagFromTag).when(tag).thenReturn(akonadiTag);

        // WHEN
        QScopedPointer<Akonadi::TagRepository> repository(new Akonadi::TagRepository(storageMock.getInstance(),
                                                                                     serializerMock.getInstance()));
        repository->remove(tag)->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::removeTag).when(akonadiTag).exactly(1));
    }

    void shouldAssociateTaskToTag()
    {
        // GIVEN
        auto tag = Domain::Tag::Ptr::create();
        Akonadi::Tag akonadiTag(42);

        Domain::Artifact::Ptr task = Domain::Task::Ptr::create();
        Akonadi::Item taskItem(42);

        // A mock of update job
        auto itemModifyJob = new FakeJob(this);

        auto itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << taskItem);

        // Storage mock returning the tagCreatejob
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::updateItem).when(taskItem, 0)
                                                          .thenReturn(itemModifyJob);

        storageMock(&Akonadi::StorageInterface::fetchItem).when(taskItem)
                                                          .thenReturn(itemFetchJob);
        // Serializer mock
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createAkonadiTagFromTag).when(tag).thenReturn(akonadiTag);
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task.objectCast<Domain::Task>()).thenReturn(taskItem);

        // WHEN
        QScopedPointer<Akonadi::TagRepository> repository(new Akonadi::TagRepository(storageMock.getInstance(),
                                                                                     serializerMock.getInstance()));
        repository->associate(tag, task)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createAkonadiTagFromTag).when(tag).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task.objectCast<Domain::Task>()).exactly(1));

        QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(taskItem, 0).exactly(1));
    }

    void shouldDissociateTaskFromTag()
    {
        // GIVEN
        Akonadi::Item item(42);
        Domain::Task::Ptr task(new Domain::Task);

        Akonadi::Tag akonadiTag(qint64(42));
        auto tag = Domain::Tag::Ptr::create();

        auto itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setExpectedError(KJob::KilledJobError);

        auto itemFetchJobFilled = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJobFilled->setItems(Akonadi::Item::List() << item);

        // A mock update job
        auto itemModifyJob = new FakeJob(this);

        // Storage mock returning the create job
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchItem).when(item)
                                                          .thenReturn(itemFetchJob)
                                                          .thenReturn(itemFetchJobFilled);
        storageMock(&Akonadi::StorageInterface::updateItem).when(item, 0)
                                                           .thenReturn(itemModifyJob);

        // Serializer mock returning the item for the task
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task)
                                                                         .thenReturn(item);
        serializerMock(&Akonadi::SerializerInterface::createAkonadiTagFromTag).when(tag)
                                                                           .thenReturn(akonadiTag);
        // WHEN
        QScopedPointer<Akonadi::TagRepository> repository(new Akonadi::TagRepository(storageMock.getInstance(),
                                                                                     serializerMock.getInstance()));
        repository->dissociate(tag, task)->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(item).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createAkonadiTagFromTag).when(tag).exactly(0));
        QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, 0).exactly(0));

        // WHEN
        repository->dissociate(tag, task)->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(item).exactly(2));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createAkonadiTagFromTag).when(tag).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, 0).exactly(1));
    }

    void shouldDissociateNoteFromTag()
    {
        // GIVEN
        Akonadi::Item item(42);
        Domain::Note::Ptr note(new Domain::Note);

        Akonadi::Tag akonadiTag(qint64(42));
        auto tag = Domain::Tag::Ptr::create();

        auto itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setExpectedError(KJob::KilledJobError);

        auto itemFetchJobFilled = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJobFilled->setItems(Akonadi::Item::List() << item);

        // A mock update job
        auto itemModifyJob = new FakeJob(this);

        // Storage mock returning the create job
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchItem).when(item)
                                                          .thenReturn(itemFetchJob)
                                                          .thenReturn(itemFetchJobFilled);
        storageMock(&Akonadi::StorageInterface::updateItem).when(item, 0)
                                                           .thenReturn(itemModifyJob);

        // Serializer mock returning the item for the note
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromNote).when(note)
                                                                         .thenReturn(item);
        serializerMock(&Akonadi::SerializerInterface::createAkonadiTagFromTag).when(tag)
                                                                           .thenReturn(akonadiTag);
        // WHEN
        QScopedPointer<Akonadi::TagRepository> repository(new Akonadi::TagRepository(storageMock.getInstance(),
                                                                                     serializerMock.getInstance()));
        repository->dissociate(tag, note)->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(item).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createAkonadiTagFromTag).when(tag).exactly(0));
        QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, 0).exactly(0));

        // WHEN
        repository->dissociate(tag, note)->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(item).exactly(2));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createAkonadiTagFromTag).when(tag).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, 0).exactly(1));
    }
};

QTEST_MAIN(AkonadiTagRepositoryTest)

#include "akonaditagrepositorytest.moc"
