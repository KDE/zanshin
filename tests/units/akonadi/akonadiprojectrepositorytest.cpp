/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "utils/mockobject.h"

#include "testlib/akonadifakejobs.h"
#include "testlib/akonadifakemonitor.h"

#include "akonadi/akonadiprojectrepository.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;

Q_DECLARE_METATYPE(Testlib::AkonadiFakeItemFetchJob*)

class AkonadiProjectRepositoryTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void shouldCreateProjectInDataSource()
    {
        // GIVEN

        // A project and its corresponding item already not existing in storage
        Akonadi::Item item;
        auto project = Domain::Project::Ptr::create();

        // A data source and its corresponding collection existing in storage
        Akonadi::Collection collection(42);
        auto source = Domain::DataSource::Ptr::create();

        // A mock create job
        auto itemCreateJob = new FakeJob(this);

        // Storage mock returning the create job
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::createItem).when(item, collection)
                                                           .thenReturn(itemCreateJob);

        // Serializer mock
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project).thenReturn(item);
        serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).thenReturn(collection);

        // WHEN
        QScopedPointer<Akonadi::ProjectRepository> repository(new Akonadi::ProjectRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));
        repository->create(project, source)->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::createItem).when(item, collection).exactly(1));
    }

    void shouldUpdateExistingProject()
    {
        // GIVEN

        // A project and its corresponding item already existing in storage
        Akonadi::Item item(42);
        Domain::Project::Ptr project(new Domain::Project);

        // A mock modify job
        auto itemModifyJob = new FakeJob(this);

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::ProjectRepository> repository(new Akonadi::ProjectRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));

        // Storage mock returning the create job
        storageMock(&Akonadi::StorageInterface::updateItem).when(item, repository.get())
                                                           .thenReturn(itemModifyJob);

        // Serializer mock returning the item for the project
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project).thenReturn(item);

        // WHEN
        repository->update(project)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, repository.get()).exactly(1));
    }

    void shouldRemoveExistingProject()
    {
        // GIVEN

        // A project and its corresponding item already existing in storage
        Akonadi::Item item(42);
        auto project = Domain::Project::Ptr::create();

        // A mock remove job
        auto itemRemoveJob = new FakeJob(this);

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::ProjectRepository> repository(new Akonadi::ProjectRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));

        // Storage mock returning the create job
        storageMock(&Akonadi::StorageInterface::removeItem).when(item, repository.get())
                                                           .thenReturn(itemRemoveJob);

        // Serializer mock returning the item for the project
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project).thenReturn(item);

        // WHEN
        repository->remove(project)->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::removeItem).when(item, repository.get()).exactly(1));
    }

    void shouldAssociateATaskToAProject_data()
    {
        QTest::addColumn<Akonadi::Item>("childItem");
        QTest::addColumn<Akonadi::Item>("parentItem");
        QTest::addColumn<Domain::Task::Ptr>("child");
        QTest::addColumn<Domain::Project::Ptr>("parent");
        QTest::addColumn<Testlib::AkonadiFakeItemFetchJob*>("itemFetchJob1");
        QTest::addColumn<Testlib::AkonadiFakeItemFetchJob*>("itemFetchJob2");
        QTest::addColumn<Testlib::AkonadiFakeItemFetchJob*>("itemFetchJob3");
        QTest::addColumn<bool>("execJob");
        QTest::addColumn<bool>("execParentJob");
        QTest::addColumn<Akonadi::Item::List>("list");

        Akonadi::Collection col(40);

        Akonadi::Item childItem(42);
        childItem.setParentCollection(col);
        Domain::Task::Ptr childTask(new Domain::Task);

        Akonadi::Item parentItem(41);
        parentItem.setParentCollection(col);
        auto parent = Domain::Project::Ptr::create();

        auto itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        auto itemFetchJob2 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << parentItem);
        auto itemFetchJob3 = new Testlib::AkonadiFakeItemFetchJob(this);

        Akonadi::Item::List list;

        QTest::newRow("nominal case (task)") << childItem << parentItem << childTask << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << true << true << list;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setExpectedError(KJob::KilledJobError);
        QTest::newRow("child job error with empty list") << childItem << parentItem << childTask << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << false << false << list;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setExpectedError(KJob::KilledJobError);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        QTest::newRow("child job error with item (task)") << childItem << parentItem << childTask << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << false << false << list;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        itemFetchJob2 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob2->setExpectedError(KJob::KilledJobError);
        QTest::newRow("parent job error with empty list (task)") << childItem << parentItem << childTask << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << true << false << list;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        itemFetchJob2 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob2->setExpectedError(KJob::KilledJobError);
        itemFetchJob2->setItems(Akonadi::Item::List() << parentItem);
        QTest::newRow("parent job error with item (task)") << childItem << parentItem << childTask << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << true << false << list;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        itemFetchJob2 = new Testlib::AkonadiFakeItemFetchJob(this);
        Akonadi::Collection col2(39);
        Akonadi::Item parentItem2(41);
        parentItem2.setParentCollection(col2);
        itemFetchJob2->setItems(Akonadi::Item::List() << parentItem2);
        itemFetchJob3 = new Testlib::AkonadiFakeItemFetchJob(this);
        QTest::newRow("update and move item (task)") << childItem << parentItem2 << childTask << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << true << true << list;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        itemFetchJob2 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << parentItem2);
        itemFetchJob3 = new Testlib::AkonadiFakeItemFetchJob(this);
        Akonadi::Item childItem2(43);
        Akonadi::Item::List list2;
        list2 << childItem2;
        itemFetchJob3->setItems(list2);
        QTest::newRow("update and move item and his child (task)") << childItem << parentItem2 << childTask << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << true << true << list2;
    }

    void shouldAssociateATaskToAProject()
    {
        // GIVEN
        QFETCH(Akonadi::Item, childItem);
        QFETCH(Akonadi::Item, parentItem);
        QFETCH(Domain::Task::Ptr, child);
        QFETCH(Domain::Project::Ptr, parent);
        QFETCH(Testlib::AkonadiFakeItemFetchJob*, itemFetchJob1);
        QFETCH(Testlib::AkonadiFakeItemFetchJob*, itemFetchJob2);
        QFETCH(Testlib::AkonadiFakeItemFetchJob*, itemFetchJob3);
        QFETCH(bool, execJob);
        QFETCH(bool, execParentJob);
        QFETCH(Akonadi::Item::List, list);

        // A mock create job
        auto itemModifyJob = new FakeJob(this);
        auto transactionJob = new FakeJob(this);
        auto itemsMoveJob = new FakeJob(this);

        Akonadi::Item::List movedList;
        movedList << childItem << list;

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::ProjectRepository> repository(new Akonadi::ProjectRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));

        // Storage mock returning the create job
        storageMock(&Akonadi::StorageInterface::fetchItem).when(childItem, repository.get())
                                                          .thenReturn(itemFetchJob1);
        storageMock(&Akonadi::StorageInterface::fetchItem).when(parentItem, repository.get())
                                                          .thenReturn(itemFetchJob2);
        if (parentItem.parentCollection().id() != childItem.parentCollection().id()) {
            storageMock(&Akonadi::StorageInterface::fetchItems).when(childItem.parentCollection(), repository.get())
                                                               .thenReturn(itemFetchJob3);
            storageMock(&Akonadi::StorageInterface::createTransaction).when(repository.get()).thenReturn(transactionJob);
            storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, transactionJob)
                                                               .thenReturn(itemModifyJob);
            storageMock(&Akonadi::StorageInterface::moveItems).when(movedList, parentItem.parentCollection(), transactionJob)
                                                              .thenReturn(itemsMoveJob);
        } else {
            storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, repository.get())
                                                               .thenReturn(itemModifyJob);
        }

        // Serializer mock returning the item for the task
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(child).thenReturn(childItem);
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(parent).thenReturn(parentItem);
        serializerMock(&Akonadi::SerializerInterface::updateItemProject).when(childItem, parent).thenReturn();
        if (execParentJob)
            serializerMock(&Akonadi::SerializerInterface::filterDescendantItems).when(list, childItem).thenReturn(list);

        // WHEN
        auto associateJob = repository->associate(parent, child);
        if (execJob)
            associateJob->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(childItem, repository.get()).exactly(1));
        if (execJob) {
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::updateItemProject).when(childItem, parent).exactly(1));
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(parent).exactly(1));
            QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(parentItem, repository.get()).exactly(1));
            if (execParentJob) {
                if (parentItem.parentCollection().id() != childItem.parentCollection().id()) {
                    QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(childItem.parentCollection(), repository.get()).exactly(1));
                    QVERIFY(storageMock(&Akonadi::StorageInterface::createTransaction).when(repository.get()).thenReturn(transactionJob).exactly(1));
                    QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, transactionJob).exactly(1));
                    QVERIFY(storageMock(&Akonadi::StorageInterface::moveItems).when(movedList, parentItem.parentCollection(), transactionJob).exactly(1));
                } else {
                    QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, repository.get()).exactly(1));
                }
            }
        }
    }

    void shouldDissociateATaskFromItsProject_data()
    {
        QTest::addColumn<Domain::Task::Ptr>("child");
        QTest::addColumn<Akonadi::Item>("childItem");
        QTest::addColumn<Testlib::AkonadiFakeItemFetchJob*>("itemFetchJob");
        QTest::addColumn<bool>("fetchJobFailed");

        Domain::Task::Ptr taskChild(new Domain::Task);
        Akonadi::Item childItem(42);

        auto itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << childItem);
        QTest::newRow("task nominal case") << taskChild << childItem << itemFetchJob << false;

        itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setExpectedError(KJob::KilledJobError);
        QTest::newRow("task job error with empty list") << taskChild << childItem << itemFetchJob << true;

        itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setExpectedError(KJob::KilledJobError);
        itemFetchJob->setItems(Akonadi::Item::List() << childItem);
        QTest::newRow("task job error with item") << taskChild << childItem << itemFetchJob << true;
    }

    void shouldDissociateATaskFromItsProject()
    {
        // GIVEN
        QFETCH(Domain::Task::Ptr, child);
        QFETCH(Akonadi::Item, childItem);
        QFETCH(Testlib::AkonadiFakeItemFetchJob*, itemFetchJob);
        QFETCH(bool, fetchJobFailed);

        auto itemModifyJob = new FakeJob(this);

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::ProjectRepository> repository(new Akonadi::ProjectRepository(storageMock.getInstance(),
                                                                                             serializerMock.getInstance()));

        // Storage mock returning the delete job
        storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, repository.get())
                                                           .thenReturn(itemModifyJob);
        storageMock(&Akonadi::StorageInterface::fetchItem).when(childItem, repository.get())
                                                          .thenReturn(itemFetchJob);

        // Serializer mock returning the item for the task
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(child).thenReturn(childItem);
        serializerMock(&Akonadi::SerializerInterface::removeItemParent).when(childItem).thenReturn();

        // WHEN
        repository->dissociate(child)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(child).exactly(1));

        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(childItem, repository.get()).exactly(1));
        if (!fetchJobFailed) {
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::removeItemParent).when(childItem).exactly(1));;
            QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, repository.get()).exactly(1));
        }

        // Give a chance to job to delete themselves
        // in case of an error (since they use deleteLater() internally)
        QTest::qWait(10);
    }
};

ZANSHIN_TEST_MAIN(AkonadiProjectRepositoryTest)

#include "akonadiprojectrepositorytest.moc"
