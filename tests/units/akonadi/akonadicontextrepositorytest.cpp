/*
 * SPDX-FileCopyrightText: 2014 Franck Arrecot <franck.arrecot@gmail.com>
   SPDX-FileCopyrightText: 2014 Rémi Benoit <r3m1.benoit@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

class AkonadiContextRepositoryTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void shouldCreateContext()
    {
        // GIVEN
        auto source = Domain::DataSource::Ptr::create();
        source->setName(QStringLiteral("Source1"));

        // A Context and its corresponding item not existing in akonadi
        Akonadi::Item contextItem;
        Akonadi::Collection collection(23);
        auto context = Domain::Context::Ptr::create();

        // A mock creating job
        auto itemCreateJob = new FakeJob(this);

        // Storage mock returning the context-item creation job
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::createItem).when(contextItem, collection)
                                                          .thenReturn(itemCreateJob);

        // Serializer mock
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromContext).when(context).thenReturn(contextItem);
        serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).thenReturn(collection);

        // WHEN
        QScopedPointer<Akonadi::ContextRepository> repository(new Akonadi::ContextRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));
        repository->create(context, source)->exec();

        //THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::createItem).when(contextItem, collection).exactly(1));
    }

    void shouldUpdateContext()
    {
        // GIVEN
        Akonadi::Item contextItem;
        contextItem.setId(42);
        auto context = Domain::Context::Ptr::create();

        // A mock job
        auto itemModifyJob = new FakeJob(this);

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::ContextRepository> repository(new Akonadi::ContextRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));

        // Storage mock returning the item modify job
        storageMock(&Akonadi::StorageInterface::updateItem).when(contextItem, repository.get())
                                                           .thenReturn(itemModifyJob);

        // Serializer mock
        serializerMock(&Akonadi::SerializerInterface::createItemFromContext).when(context).thenReturn(contextItem);

        // WHEN
        repository->update(context)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromContext).when(context).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(contextItem, repository.get()).exactly(1));
    }

    void shouldRemoveContext()
    {
        // GIVEN
        Akonadi::Item contextItem;
        contextItem.setId(42);
        auto context = Domain::Context::Ptr::create();

        // A mock job
        auto contextItemDeleteJob = new FakeJob(this);

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::ContextRepository> repository(new Akonadi::ContextRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));

        // Storage mock returning the mock job
        storageMock(&Akonadi::StorageInterface::removeItem).when(contextItem, repository.get())
                                                           .thenReturn(contextItemDeleteJob);

        // Serializer mock
        serializerMock(&Akonadi::SerializerInterface::createItemFromContext).when(context).thenReturn(contextItem);

        // WHEN
        repository->remove(context)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromContext).when(context).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::removeItem).when(contextItem, repository.get()).exactly(1));
    }

    void shouldAssociateATaskToAContext_data()
    {
        QTest::addColumn<Akonadi::Item>("associatedContextItem");
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<Domain::Context::Ptr>("context");
        QTest::addColumn<Domain::Task::Ptr>("task");
        QTest::addColumn<Testlib::AkonadiFakeItemFetchJob*>("itemFetchJob");
        QTest::addColumn<bool>("execJob");

        Akonadi::Collection col(40);

        Akonadi::Item item(42);
        item.setParentCollection(col);
        Domain::Task::Ptr task(new Domain::Task);

        Akonadi::Item associatedContextItem(43);
        auto associatedContext = Domain::Context::Ptr::create();

        auto itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item);
        QTest::newRow("nominal case") << associatedContextItem << item << associatedContext << task << itemFetchJob << true;

        itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setExpectedError(KJob::KilledJobError);
        QTest::newRow("task job error, cannot find task") << associatedContextItem << item << associatedContext << task << itemFetchJob << false;
    }

    void shouldAssociateATaskToAContext()
    {
        // GIVEN
        QFETCH(Akonadi::Item,associatedContextItem);
        QFETCH(Akonadi::Item,item);
        QFETCH(Domain::Context::Ptr,context);
        QFETCH(Domain::Task::Ptr,task);
        QFETCH(Testlib::AkonadiFakeItemFetchJob*,itemFetchJob);
        QFETCH(bool,execJob);

        // A mock update job
        auto itemModifyJob = new FakeJob(this);

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::ContextRepository> repository(new Akonadi::ContextRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));

        // Storage mock returning the create job
        storageMock(&Akonadi::StorageInterface::fetchItem).when(item, repository.get())
                                                          .thenReturn(itemFetchJob);
        storageMock(&Akonadi::StorageInterface::updateItem).when(item, repository.get())
                                                           .thenReturn(itemModifyJob);

        // Serializer mock returning the item for the task
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task)
                                                                         .thenReturn(item);
        serializerMock(&Akonadi::SerializerInterface::addContextToTask).when(context, item)
                                                                       .thenReturn();

        // WHEN
        auto associateJob = repository->associate(context, task);
        if (execJob)
            associateJob->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(item, repository.get()).exactly(1));
        if (execJob) {
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).exactly(1));
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::addContextToTask).when(context, item).exactly(1));
            QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, repository.get()).exactly(1));
        }
    }

    void shoudDissociateTaskFromContext_data()
    {
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<Domain::Context::Ptr>("context");
        QTest::addColumn<Domain::Task::Ptr>("task");
        QTest::addColumn<Testlib::AkonadiFakeItemFetchJob*>("itemFetchJob");
        QTest::addColumn<bool>("execJob");

        Akonadi::Item item(42);
        Domain::Task::Ptr task(new Domain::Task);

        auto associatedContext = Domain::Context::Ptr::create();

        auto itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item);
        QTest::newRow("nominal case") << item << associatedContext << task << itemFetchJob << true;

        itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setExpectedError(KJob::KilledJobError);
        QTest::newRow("task job error, cannot find task") << item << associatedContext << task << itemFetchJob << false;
    }

    void shoudDissociateTaskFromContext()
    {
        QFETCH(Akonadi::Item,item);
        QFETCH(Domain::Context::Ptr,context);
        QFETCH(Domain::Task::Ptr,task);
        QFETCH(Testlib::AkonadiFakeItemFetchJob*,itemFetchJob);
        QFETCH(bool,execJob);

        // A mock update job
        auto itemModifyJob = new FakeJob(this);

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::ContextRepository> repository(new Akonadi::ContextRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));

        // Storage mock returning the create job
        storageMock(&Akonadi::StorageInterface::fetchItem).when(item, repository.get())
                                                          .thenReturn(itemFetchJob);
        storageMock(&Akonadi::StorageInterface::updateItem).when(item, repository.get())
                                                           .thenReturn(itemModifyJob);

        // Serializer mock returning the item for the task
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task)
                                                                         .thenReturn(item);
        serializerMock(&Akonadi::SerializerInterface::removeContextFromTask).when(context, item)
                                                                            .thenReturn();

        // WHEN
        auto dissociateJob = repository->dissociate(context, task);

        if (execJob)
            dissociateJob->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(item, repository.get()).exactly(1));
        if (execJob) {
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::removeContextFromTask).when(context, item).exactly(1));
            QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, repository.get()).exactly(1));
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

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::ContextRepository> repository(new Akonadi::ContextRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));

        // Storage mock returning the create job
        storageMock(&Akonadi::StorageInterface::fetchItem).when(item, repository.get())
                                                          .thenReturn(itemFetchJob);
        storageMock(&Akonadi::StorageInterface::updateItem).when(item, repository.get())
                                                           .thenReturn(itemModifyJob);

        // Serializer mock returning the item for the task
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task)
                                                                         .thenReturn(item);

        // WHEN
        auto dissociateJob = repository->dissociateAll(task);

        if (execJob)
            dissociateJob->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(item, repository.get()).exactly(1));
        if (execJob) {
            QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, repository.get()).exactly(1));
        } else {
            delete dissociateJob;
        }

        // Give a chance to itemFetchJob to delete itself
        // in case of an error (since it uses deleteLater() internally)
        QTest::qWait(10);
    }
};

ZANSHIN_TEST_MAIN(AkonadiContextRepositoryTest)

#include "akonadicontextrepositorytest.moc"
