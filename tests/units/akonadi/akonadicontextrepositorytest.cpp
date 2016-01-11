/* This file is part of Zanshin

   Copyright 2014 Franck Arrecot <franck.arrecot@gmail.com>
   Copyright 2014 Rémi Benoit <r3m1.benoit@gmail.com>

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

#include "utils/mockobject.h"

#include "testlib/akonadifakejobs.h"
#include "testlib/akonadifakemonitor.h"

#include "akonadi/akonadicontextrepository.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;
using namespace mockitopp::matcher;

Q_DECLARE_METATYPE(Testlib::AkonadiFakeItemFetchJob*)
Q_DECLARE_METATYPE(Testlib::AkonadiFakeTagFetchJob*)

class AkonadiContextRepositoryTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldCreateContext()
    {
        // GIVEN

        // A Context and its corresponding Tag not existing in akonadi
        Akonadi::Tag tag;
        auto context = Domain::Context::Ptr::create();

        // A mock creating job
        auto tagCreateJob = new FakeJob(this);

        // Storage mock returning the tagCreatejob
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::createTag).when(tag)
                                                          .thenReturn(tagCreateJob);

        // Serializer mock
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createTagFromContext).when(context).thenReturn(tag);


        // WHEN
        QScopedPointer<Akonadi::ContextRepository> repository(new Akonadi::ContextRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));
        repository->create(context)->exec();

        //THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::createTag).when(tag).exactly(1));
    }

    void shouldUpdateContext()
    {
        // GIVEN
        Akonadi::Tag tag;
        tag.setName("tag42");
        tag.setId(42);
        auto context = Domain::Context::Ptr::create();

        // A mock creating job
        auto tagModifyJob = new FakeJob(this);

        // Storage mock returning the tagCreatejob
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::updateTag).when(tag)
                                                          .thenReturn(tagModifyJob);

        // Serializer mock
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createTagFromContext).when(context).thenReturn(tag);

        // WHEN
        QScopedPointer<Akonadi::ContextRepository> repository(new Akonadi::ContextRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));

        repository->update(context)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTagFromContext).when(context).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::updateTag).when(tag).exactly(1));
    }

    void shouldRemoveContext()
    {
        // GIVEN
        Akonadi::Tag tag;
        tag.setName("tag42");
        tag.setId(42);
        auto context = Domain::Context::Ptr::create();

        // A mock creating job
        auto tagDeleteJob= new FakeJob(this);

        // Storage mock returning the tagCreatejob
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::removeTag).when(tag)
                                                          .thenReturn(tagDeleteJob);

        // Serializer mock
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createTagFromContext).when(context).thenReturn(tag);

        // WHEN
        QScopedPointer<Akonadi::ContextRepository> repository(new Akonadi::ContextRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));
        repository->remove(context)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTagFromContext).when(context).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::removeTag).when(tag).exactly(1));
    }

    void shouldAssociateATaskToAContext_data()
    {
        QTest::addColumn<Akonadi::Tag>("associatedTag");
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<Domain::Context::Ptr>("context");
        QTest::addColumn<Domain::Task::Ptr>("task");
        QTest::addColumn<Testlib::AkonadiFakeItemFetchJob*>("itemFetchJob");
        QTest::addColumn<bool>("execJob");

        Akonadi::Collection col(40);

        Akonadi::Item item(42);
        item.setParentCollection(col);
        Domain::Task::Ptr task(new Domain::Task);

        Akonadi::Tag associatedTag(qint64(43));
        auto associatedContext = Domain::Context::Ptr::create();

        auto itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item);
        QTest::newRow("nominal case") << associatedTag << item << associatedContext << task << itemFetchJob << true;

        itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setExpectedError(KJob::KilledJobError);
        QTest::newRow("task job error, cannot find task") << associatedTag << item << associatedContext << task << itemFetchJob << false;
    }

    void shouldAssociateATaskToAContext()
    {
        // GIVEN
        QFETCH(Akonadi::Tag,associatedTag);
        QFETCH(Akonadi::Item,item);
        QFETCH(Domain::Context::Ptr,context);
        QFETCH(Domain::Task::Ptr,task);
        QFETCH(Testlib::AkonadiFakeItemFetchJob*,itemFetchJob);
        QFETCH(bool,execJob);

        // A mock update job
        auto itemModifyJob = new FakeJob(this);

        // Storage mock returning the create job
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchItem).when(item)
                                                          .thenReturn(itemFetchJob);
        storageMock(&Akonadi::StorageInterface::updateItem).when(item, Q_NULLPTR)
                                                           .thenReturn(itemModifyJob);

        // Serializer mock returning the item for the task
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task)
                                                                         .thenReturn(item);
        serializerMock(&Akonadi::SerializerInterface::createTagFromContext).when(context)
                                                                           .thenReturn(associatedTag);

        // WHEN
        QScopedPointer<Akonadi::ContextRepository> repository(new Akonadi::ContextRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));

        auto associateJob = repository->associate(context, task);
        if (execJob)
            associateJob->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(item).exactly(1));
        if (execJob) {
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTagFromContext).when(context).exactly(1));
            QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, Q_NULLPTR).exactly(1));
        }
    }

    void shoudDissociateTaskFromContext_data()
    {
        QTest::addColumn<Akonadi::Tag>("associatedTag");
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<Domain::Context::Ptr>("context");
        QTest::addColumn<Domain::Task::Ptr>("task");
        QTest::addColumn<Testlib::AkonadiFakeItemFetchJob*>("itemFetchJob");
        QTest::addColumn<bool>("execJob");

        Akonadi::Item item(42);
        Domain::Task::Ptr task(new Domain::Task);

        Akonadi::Tag associatedTag(qint64(43));
        auto associatedContext = Domain::Context::Ptr::create();

        auto itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item);
        QTest::newRow("nominal case") << associatedTag << item << associatedContext << task << itemFetchJob << true;

        itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setExpectedError(KJob::KilledJobError);
        QTest::newRow("task job error, cannot find task") << associatedTag << item << associatedContext << task << itemFetchJob << false;
    }

    void shoudDissociateTaskFromContext()
    {
        QFETCH(Akonadi::Tag,associatedTag);
        QFETCH(Akonadi::Item,item);
        QFETCH(Domain::Context::Ptr,context);
        QFETCH(Domain::Task::Ptr,task);
        QFETCH(Testlib::AkonadiFakeItemFetchJob*,itemFetchJob);
        QFETCH(bool,execJob);

        // A mock update job
        auto itemModifyJob = new FakeJob(this);

        // Storage mock returning the create job
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchItem).when(item)
                                                          .thenReturn(itemFetchJob);
        storageMock(&Akonadi::StorageInterface::updateItem).when(item, Q_NULLPTR)
                                                           .thenReturn(itemModifyJob);

        // Serializer mock returning the item for the task
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task)
                                                                         .thenReturn(item);
        serializerMock(&Akonadi::SerializerInterface::createTagFromContext).when(context)
                                                                           .thenReturn(associatedTag);

        // WHEN
        QScopedPointer<Akonadi::ContextRepository> repository(new Akonadi::ContextRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));

        auto dissociateJob = repository->dissociate(context, task);

        if (execJob)
            dissociateJob->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(item).exactly(1));
        if (execJob) {
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::createTagFromContext).when(context).exactly(1));
            QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, Q_NULLPTR).exactly(1));
        }
    }

    void shouldDissociateTaskFromAllContext_data()
    {
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<Domain::Task::Ptr>("task");
        QTest::addColumn<Testlib::AkonadiFakeItemFetchJob*>("itemFetchJob");
        QTest::addColumn<bool>("execJob");

        Akonadi::Item item(42);
        Domain::Task::Ptr task(new Domain::Task);

        auto itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item);
        QTest::newRow("nominal case") << item << task << itemFetchJob << true;

        itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setExpectedError(KJob::KilledJobError);
        QTest::newRow("task job error, cannot find task") << item << task << itemFetchJob << false;
    }

    void shouldDissociateTaskFromAllContext()
    {
        QFETCH(Akonadi::Item,item);
        QFETCH(Domain::Task::Ptr,task);
        QFETCH(Testlib::AkonadiFakeItemFetchJob*,itemFetchJob);
        QFETCH(bool,execJob);

        // A mock update job
        auto itemModifyJob = new FakeJob(this);

        // Storage mock returning the create job
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchItem).when(item)
                                                          .thenReturn(itemFetchJob);
        storageMock(&Akonadi::StorageInterface::updateItem).when(item, Q_NULLPTR)
                                                           .thenReturn(itemModifyJob);

        // Serializer mock returning the item for the task
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task)
                                                                         .thenReturn(item);

        // WHEN
        QScopedPointer<Akonadi::ContextRepository> repository(new Akonadi::ContextRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));

        auto dissociateJob = repository->dissociateAll(task);

        if (execJob)
            dissociateJob->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(item).exactly(1));
        if (execJob) {
            QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, Q_NULLPTR).exactly(1));
        }
    }
};

ZANSHIN_TEST_MAIN(AkonadiContextRepositoryTest)

#include "akonadicontextrepositorytest.moc"
