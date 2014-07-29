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

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <mockitopp/mockitopp.hpp>
#include "testlib/akonadimocks.h"

#include "akonadi/akonaditaskrepository.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"
#include "akonadi/akonadistoragesettings.h"

using namespace mockitopp;

Q_DECLARE_METATYPE(MockItemFetchJob*)

class AkonadiTaskRepositoryTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldCheckIfASourceIsDefaultFromSettings()
    {
        // GIVEN

        // A default collection for saving
        Akonadi::Collection col(42);
        Akonadi::StorageSettings::instance().setDefaultTaskCollection(col);

        // The data source corresponding to the default collection
        auto source = Domain::DataSource::Ptr::create();

        // Storage mock sitting here doing nothing
        mock_object<Akonadi::StorageInterface> storageMock;

        // Serializer mock returning the collection for the source
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).thenReturn(col);

        // WHEN
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(&storageMock.getInstance(),
                                                                                       &serializerMock.getInstance()));

        // THEN
        QVERIFY(repository->isDefaultSource(source));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).exactly(1));
    }

    void shouldNoticeIfSourceIsNotDefault()
    {
        // GIVEN

        // A default collection for saving
        Akonadi::Collection defaultCol(42);
        Akonadi::StorageSettings::instance().setDefaultTaskCollection(defaultCol);

        // Another random collection
        Akonadi::Collection col(43);

        // The data source corresponding to the random collection
        auto source = Domain::DataSource::Ptr::create();

        // Storage mock sitting here doing nothing
        mock_object<Akonadi::StorageInterface> storageMock;

        // Serializer mock returning the collection for the source
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).thenReturn(col);

        // WHEN
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(&storageMock.getInstance(),
                                                                                       &serializerMock.getInstance()));

        // THEN
        QVERIFY(!repository->isDefaultSource(source));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).exactly(1));
    }

    void shouldStoreDefaultSourceInTheSettings()
    {
        // GIVEN

        // A value in the settings
        auto &settings = Akonadi::StorageSettings::instance();
        settings.setDefaultTaskCollection(Akonadi::Collection(21));

        // The new default data source we want
        auto source = Domain::DataSource::Ptr::create();

        // A collection corresponding to the data source
        Akonadi::Collection col(42);

        // Storage mock sitting here doing nothing
        mock_object<Akonadi::StorageInterface> storageMock;

        // Serializer mock returning the collection for the source
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).thenReturn(col);

        // WHEN
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(&storageMock.getInstance(),
                                                                                       &serializerMock.getInstance()));
        repository->setDefaultSource(source);

        // THEN
        QCOMPARE(settings.defaultTaskCollection(), col);
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).exactly(1));
    }


    void shouldCreateNewItemsOnSave()
    {
        // GIVEN

        // A default collection for saving
        Akonadi::Collection col(42);

        // A task and its corresponding item not existing in storage yet
        Akonadi::Item item(42);
        Domain::Task::Ptr task(new Domain::Task);

        // A mock create job
        auto itemCreateJob = new MockAkonadiJob(this);

        // Storage mock returning the create job
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::defaultTaskCollection).when().thenReturn(col);
        storageMock(&Akonadi::StorageInterface::createItem).when(item, col)
                                                           .thenReturn(itemCreateJob);

        // Serializer mock returning the item for the task
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).thenReturn(item);

        // WHEN
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(&storageMock.getInstance(),
                                                                                       &serializerMock.getInstance()));
        repository->save(task)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::defaultTaskCollection).when().exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::createItem).when(item, col).exactly(1));
    }

    void shouldCreateNewItemsInFirstWritableCollectionIsNothingInSettings()
    {
        // GIVEN

        // A few collections
        Akonadi::Collection col1(42);
        col1.setRights(Akonadi::Collection::ReadOnly);
        Akonadi::Collection col2(42);
        Akonadi::Collection col3(42);
        auto collectionFetchJob = new MockCollectionFetchJob;
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2 << col3);

        // A task and its corresponding item not existing in storage yet
        Akonadi::Item item(42);
        Domain::Task::Ptr task(new Domain::Task);

        // A mock create job
        auto itemCreateJob = new MockAkonadiJob(this);

        // Storage mock returning the create job and with no default collection
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::defaultTaskCollection).when().thenReturn(Akonadi::Collection());
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Tasks)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::createItem).when(item, col2)
                                                           .thenReturn(itemCreateJob);

        // Serializer mock returning the item for the task
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).thenReturn(item);

        // WHEN
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(&storageMock.getInstance(),
                                                                                       &serializerMock.getInstance()));
        repository->save(task)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::defaultTaskCollection).when().exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::createItem).when(item, col2).exactly(1));
    }

    void shouldUpdateExistingItemsOnSave()
    {
        // GIVEN

        // A default collection for saving
        Akonadi::Collection col(42);

        // A task and its corresponding item already existing in storage
        Akonadi::Item item(42);
        Domain::Task::Ptr task(new Domain::Task);
        task->setProperty("itemId", item.id());

        // A mock create job
        auto itemModifyJob = new MockAkonadiJob(this);

        // Storage mock returning the create job
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::updateItem).when(item, 0)
                                                           .thenReturn(itemModifyJob);

        // Serializer mock returning the item for the task
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).thenReturn(item);

        // WHEN
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(&storageMock.getInstance(),
                                                                                       &serializerMock.getInstance()));
        repository->save(task)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, 0).exactly(1));
    }

    void shouldRemoveATask()
    {
        // GIVEN
        Akonadi::Item item(42);
        Domain::Task::Ptr task(new Domain::Task);

        // A mock delete job
        auto itemDeleteJob = new MockAkonadiJob(this);

        // Storage mock returning the delete job
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::removeItem).when(item)
                                                           .thenReturn(itemDeleteJob);

        // Serializer mock returning the item for the task
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).thenReturn(item);

        // WHEN
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(&storageMock.getInstance(),
                                                                                       &serializerMock.getInstance()));
        repository->remove(task)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::removeItem).when(item).exactly(1));
    }

    void shouldAssociateATaskToAnother_data()
    {
        QTest::addColumn<Akonadi::Item>("childItem");
        QTest::addColumn<Akonadi::Item>("parentItem");
        QTest::addColumn<Domain::Task::Ptr>("child");
        QTest::addColumn<Domain::Task::Ptr>("parent");
        QTest::addColumn<MockItemFetchJob*>("itemFetchJob1");
        QTest::addColumn<MockItemFetchJob*>("itemFetchJob2");
        QTest::addColumn<MockItemFetchJob*>("itemFetchJob3");
        QTest::addColumn<bool>("execJob");
        QTest::addColumn<bool>("execParentJob");
        QTest::addColumn<Akonadi::Item::List>("list");

        Akonadi::Collection col(40);

        Akonadi::Item childItem(42);
        childItem.setParentCollection(col);
        Domain::Task::Ptr child(new Domain::Task);

        Akonadi::Item parentItem(41);
        parentItem.setParentCollection(col);
        Domain::Task::Ptr parent(new Domain::Task);

        auto itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        auto itemFetchJob2 = new MockItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << parentItem);
        auto itemFetchJob3 = new MockItemFetchJob(this);

        Akonadi::Item::List list;

        QTest::newRow("nominal case") << childItem << parentItem << child << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << true << true << list;

        itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setExpectedError(KJob::KilledJobError);
        QTest::newRow("child job error with empty list") << childItem << parentItem << child << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << false << false << list;

        itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setExpectedError(KJob::KilledJobError);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        QTest::newRow("child job error with item") << childItem << parentItem << child << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << false << false << list;

        itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        itemFetchJob2 = new MockItemFetchJob(this);
        itemFetchJob2->setExpectedError(KJob::KilledJobError);
        QTest::newRow("parent job error with empty list") << childItem << parentItem << child << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << true << false << list;

        itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        itemFetchJob2 = new MockItemFetchJob(this);
        itemFetchJob2->setExpectedError(KJob::KilledJobError);
        itemFetchJob2->setItems(Akonadi::Item::List() << parentItem);
        QTest::newRow("parent job error with item") << childItem << parentItem << child << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << true << false << list;

        itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        itemFetchJob2 = new MockItemFetchJob(this);
        Akonadi::Collection col2(39);
        Akonadi::Item parentItem2(41);
        parentItem2.setParentCollection(col2);
        itemFetchJob2->setItems(Akonadi::Item::List() << parentItem2);
        itemFetchJob3 = new MockItemFetchJob(this);
        QTest::newRow("update and move item") << childItem << parentItem2 << child << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << true << true << list;

        itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        itemFetchJob2 = new MockItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << parentItem2);
        itemFetchJob3 = new MockItemFetchJob(this);
        Akonadi::Item childItem2(43);
        Akonadi::Item::List list2;
        list2 << childItem2;
        itemFetchJob3->setItems(list2);
        QTest::newRow("update and move item and his child") << childItem << parentItem2 << child << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << true << true << list2;
    }

    void shouldAssociateATaskToAnother()
    {
        // GIVEN
        QFETCH(Akonadi::Item, childItem);
        QFETCH(Akonadi::Item, parentItem);
        QFETCH(Domain::Task::Ptr, child);
        QFETCH(Domain::Task::Ptr, parent);
        QFETCH(MockItemFetchJob*, itemFetchJob1);
        QFETCH(MockItemFetchJob*, itemFetchJob2);
        QFETCH(MockItemFetchJob*, itemFetchJob3);
        QFETCH(bool, execJob);
        QFETCH(bool, execParentJob);
        QFETCH(Akonadi::Item::List, list);

        // A mock create job
        auto itemModifyJob = new MockAkonadiJob(this);
        auto transactionJob = new MockAkonadiJob(this);
        auto itemsMoveJob = new MockAkonadiJob(this);

        Akonadi::Item::List movedList;
        movedList << childItem << list;

        // Storage mock returning the create job
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchItem).when(childItem)
                                                          .thenReturn(itemFetchJob1);
        storageMock(&Akonadi::StorageInterface::fetchItem).when(parentItem)
                                                          .thenReturn(itemFetchJob2);
        if (parentItem.parentCollection().id() != childItem.parentCollection().id()) {
            storageMock(&Akonadi::StorageInterface::fetchItems).when(childItem.parentCollection())
                                                               .thenReturn(itemFetchJob3);
            storageMock(&Akonadi::StorageInterface::createTransaction).when().thenReturn(transactionJob);
            storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, transactionJob)
                                                               .thenReturn(itemModifyJob);
            storageMock(&Akonadi::StorageInterface::moveItems).when(movedList, parentItem.parentCollection(), transactionJob)
                                                              .thenReturn(itemsMoveJob);
        } else {
            storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, 0)
                                                               .thenReturn(itemModifyJob);
        }

        // Serializer mock returning the item for the task
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(child).thenReturn(childItem);
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(parent).thenReturn(parentItem);
        serializerMock(&Akonadi::SerializerInterface::updateItemParent).when(childItem, parent).thenReturn();
        if (execParentJob)
            serializerMock(&Akonadi::SerializerInterface::filterDescendantItems).when(list, childItem).thenReturn(list);

        // WHEN
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(&storageMock.getInstance(),
                                                                                       &serializerMock.getInstance()));
        auto associateJob = repository->associate(parent, child);
        if (execJob)
            associateJob->exec();


        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(child).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(childItem).exactly(1));
        if (execJob) {
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::updateItemParent).when(childItem, parent).exactly(1));
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(parent).exactly(1));
            QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(parentItem).exactly(1));
            if (execParentJob) {
                if (parentItem.parentCollection().id() == childItem.parentCollection().id())
                    QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, 0).exactly(1));
                else {
                    //QVERIFY(serializerMock(&Akonadi::SerializerInterface::filterDescendantItems).when(list, childItem).exactly(1));
                    QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(childItem.parentCollection()).exactly(1));
                    QVERIFY(storageMock(&Akonadi::StorageInterface::createTransaction).when().thenReturn(transactionJob).exactly(1));
                    QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, transactionJob).exactly(1));
                    QVERIFY(storageMock(&Akonadi::StorageInterface::moveItems).when(movedList, parentItem.parentCollection(), transactionJob).exactly(1));
                }
            }
        }
    }

    void shouldDissociateATaskFromHisParent_data()
    {
        QTest::addColumn<Domain::Task::Ptr>("child");
        QTest::addColumn<Domain::Task::Ptr>("parent");
        QTest::addColumn<Akonadi::Item>("childItem");
        QTest::addColumn<Akonadi::Item>("parentItem");
        QTest::addColumn<MockItemFetchJob*>("itemFetchJob1");
        QTest::addColumn<MockItemFetchJob*>("itemFetchJob2");
        QTest::addColumn<bool>("parentJobFailed");
        QTest::addColumn<bool>("childJobFailed");

        Domain::Task::Ptr child(new Domain::Task);
        Domain::Task::Ptr parent(new Domain::Task);

        Akonadi::Item childItem(42);
        Akonadi::Item parentItem(41);

        auto itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        auto itemFetchJob2 = new MockItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << parentItem);

        QTest::newRow("nominal case") << child << parent << childItem << parentItem << itemFetchJob1 << itemFetchJob2 <<  false << false;

        itemFetchJob2 = new MockItemFetchJob(this);
        itemFetchJob2->setExpectedError(KJob::KilledJobError);
        QTest::newRow("parent job error with empty list") << child << parent << childItem << parentItem << itemFetchJob1 << itemFetchJob2 << true << false;

        itemFetchJob2 = new MockItemFetchJob(this);
        itemFetchJob2->setExpectedError(KJob::KilledJobError);
        itemFetchJob2->setItems(Akonadi::Item::List() << parentItem);
        QTest::newRow("parent job error with item") << child << parent << childItem << parentItem << itemFetchJob1 << itemFetchJob2 << true << false;

        itemFetchJob2 = new MockItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << parentItem);
        itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setExpectedError(KJob::KilledJobError);
        QTest::newRow("child job error with empty list") << child << parent << childItem << parentItem << itemFetchJob1 << itemFetchJob2 << false << true;

        itemFetchJob2 = new MockItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << parentItem);
        itemFetchJob1 = new MockItemFetchJob(this);
        itemFetchJob1->setExpectedError(KJob::KilledJobError);
        itemFetchJob1->setItems(Akonadi::Item::List() <<childItem);
        QTest::newRow("child job error with item") << child << parent << childItem << parentItem << itemFetchJob1 << itemFetchJob2 << false << true;
    }

    void shouldDissociateATaskFromHisParent()
    {
        // GIVEN
        QFETCH(Domain::Task::Ptr, child);
        QFETCH(Domain::Task::Ptr, parent);
        QFETCH(Akonadi::Item, childItem);
        QFETCH(Akonadi::Item, parentItem);
        QFETCH(MockItemFetchJob*, itemFetchJob1);
        QFETCH(MockItemFetchJob*, itemFetchJob2);
        QFETCH(bool, parentJobFailed);
        QFETCH(bool, childJobFailed);

        auto itemModifyJob = new MockAkonadiJob(this);

        // Storage mock returning the delete job
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, 0)
                                                           .thenReturn(itemModifyJob);
        storageMock(&Akonadi::StorageInterface::fetchItem).when(childItem)
                                                          .thenReturn(itemFetchJob1);
        storageMock(&Akonadi::StorageInterface::fetchItem).when(parentItem)
                                                          .thenReturn(itemFetchJob2);

        // Serializer mock returning the item for the task
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(parent).thenReturn(parentItem);
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(child).thenReturn(childItem);
        serializerMock(&Akonadi::SerializerInterface::removeItemParent).when(childItem).thenReturn();
        serializerMock(&Akonadi::SerializerInterface::isTaskChild).when(child, parentItem).thenReturn(true);

        // WHEN
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(&storageMock.getInstance(),
                                                                                       &serializerMock.getInstance()));
        repository->dissociate(parent, child)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(parent).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(parentItem).exactly(1));
        if (!parentJobFailed) {
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::isTaskChild).when(child, parentItem).exactly(1));
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(child).exactly(1));
            QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(childItem).exactly(1));
            if (!childJobFailed) {
                QVERIFY(serializerMock(&Akonadi::SerializerInterface::removeItemParent).when(childItem).exactly(1));;
                QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, 0).exactly(1));
            }
        }
    }
};

QTEST_MAIN(AkonadiTaskRepositoryTest)

#include "akonaditaskrepositorytest.moc"
