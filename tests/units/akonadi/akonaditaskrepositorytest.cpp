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

#include <testlib/qtest_zanshin.h>

#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>

#include "utils/mockobject.h"

#include "testlib/akonadifakedata.h"
#include "testlib/akonadifakejobs.h"
#include "testlib/akonadifakemonitor.h"
#include "testlib/akonadifakestorage.h"
#include "testlib/gencollection.h"
#include "testlib/gentodo.h"

#include "akonadi/akonaditaskrepository.h"
#include "akonadi/akonadiserializer.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;
using namespace mockitopp::matcher;
using namespace Testlib;

Q_DECLARE_METATYPE(Testlib::AkonadiFakeItemFetchJob*)

class AkonadiTaskRepositoryTest : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiTaskRepositoryTest(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

private slots:
    void shouldCreateNewItems()
    {
        // GIVEN

        // A default collection for saving
        Akonadi::Collection col(42);

        // A task and its corresponding item not existing in storage yet
        Akonadi::Item item;
        Domain::Task::Ptr task(new Domain::Task);

        // A mock create job
        auto itemCreateJob = new FakeJob(this);

        // Storage mock returning the create job
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::defaultCollection).when().thenReturn(col);
        storageMock(&Akonadi::StorageInterface::createItem).when(item, col)
                                                           .thenReturn(itemCreateJob);

        // Serializer mock returning the item for the task
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).thenReturn(item);

        // WHEN
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));
        repository->create(task)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::defaultCollection).when().exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::createItem).when(item, col).exactly(1));
    }

    void shouldCreateNewItemsInFirstWritableCollectionIsNothingInSettings()
    {
        // GIVEN

        // A few collections
        auto col1 = Akonadi::Collection(GenCollection().withId(42).withRootAsParent().withTaskContent());
        col1.setRights(Akonadi::Collection::ReadOnly);
        auto col2 = Akonadi::Collection(GenCollection().withId(43).withRootAsParent().withTaskContent());
        col2.setRights(Akonadi::Collection::CanCreateItem);
        auto col3 = Akonadi::Collection(GenCollection().withId(44).withRootAsParent().withTaskContent());
        col3.setRights(Akonadi::Collection::CanCreateItem
                     | Akonadi::Collection::CanChangeItem
                     | Akonadi::Collection::CanDeleteItem);
        auto collectionFetchJob = new Testlib::AkonadiFakeCollectionFetchJob;
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2 << col3);

        // A task and its corresponding item not existing in storage yet
        Akonadi::Item item;
        Domain::Task::Ptr task(new Domain::Task);

        // A mock create job
        auto itemCreateJob = new FakeJob(this);

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));

        // Storage mock returning the create job and with no default collection
        storageMock(&Akonadi::StorageInterface::defaultCollection).when().thenReturn(Akonadi::Collection());
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       repository.get())
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::createItem).when(item, col3)
                                                           .thenReturn(itemCreateJob);

        // Serializer mock returning the item for the task
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).thenReturn(item);

        // WHEN
        repository->create(task)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::defaultCollection).when().exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::createItem).when(item, col3).exactly(1));
    }

    void shouldEmitErrorIfNoFallbackCollectionIsFound()
    {
        // GIVEN

        // A few collections
        auto col1 = Akonadi::Collection(GenCollection().withId(42).withRootAsParent().withTaskContent());
        col1.setRights(Akonadi::Collection::ReadOnly);
        auto col2 = Akonadi::Collection(GenCollection().withId(43).withRootAsParent().withTaskContent());
        col2.setRights(Akonadi::Collection::CanCreateItem);
        auto col3 = Akonadi::Collection(GenCollection().withId(44).withRootAsParent().withTaskContent());
        col3.setRights(Akonadi::Collection::ReadOnly);
        auto collectionFetchJob = new Testlib::AkonadiFakeCollectionFetchJob;
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2 << col3);

        // A task and its corresponding item not existing in storage yet
        Akonadi::Item item;
        Domain::Task::Ptr task(new Domain::Task);

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));

        // Storage mock returning the create job and with no default collection
        storageMock(&Akonadi::StorageInterface::defaultCollection).when().thenReturn(Akonadi::Collection());
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       repository.get())
                                                                 .thenReturn(collectionFetchJob);

        // Serializer mock returning the item for the task
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).thenReturn(item);

        // WHEN
        auto job = repository->create(task);
        job->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::defaultCollection).when().exactly(1));
        QVERIFY(job->error());
        QVERIFY(!job->errorText().isEmpty());
    }

    void shouldCreateNewChildrenInParentCollection()
    {
        // GIVEN

        // A parent item with a collection
        Akonadi::Collection col(42);
        Akonadi::Item parentItem(43);
        parentItem.setParentCollection(col);
        auto parent = Domain::Task::Ptr::create();

        // A task and its corresponding item not existing in storage yet
        Akonadi::Item childItem;
        auto child = Domain::Task::Ptr::create();

        // A mock create job
        auto itemCreateJob = new FakeJob(this);

        // Storage mock returning the create job
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::createItem).when(childItem, col)
                                                           .thenReturn(itemCreateJob);

        // Serializer mock returning the item for the task
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(parent).thenReturn(parentItem);
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(child).thenReturn(childItem);
        serializerMock(&Akonadi::SerializerInterface::updateItemParent).when(childItem, parent).thenReturn();

        // WHEN
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));
        repository->createChild(child, parent)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(parent).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(child).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::updateItemParent).when(childItem, parent).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::createItem).when(childItem, col).exactly(1));
    }

    void shouldCreateNewItemsInProjectCollection()
    {
        // GIVEN

        // A project item with a collection
        Akonadi::Collection col(42);
        Akonadi::Item projectItem(43);
        projectItem.setParentCollection(col);
        auto project = Domain::Project::Ptr::create();

        // A task and its corresponding item not existing in storage yet
        Akonadi::Item taskItem;
        auto task = Domain::Task::Ptr::create();

        // A mock create job
        auto itemCreateJob = new FakeJob(this);

        // Storage mock returning the create job
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::createItem).when(taskItem, col)
                .thenReturn(itemCreateJob);

        // Serializer mock returning the item for the task
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project).thenReturn(projectItem);
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).thenReturn(taskItem);
        serializerMock(&Akonadi::SerializerInterface::updateItemProject).when(taskItem, project).thenReturn();

        // WHEN
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));
        repository->createInProject(task, project)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::updateItemProject).when(taskItem, project).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::createItem).when(taskItem, col).exactly(1));
    }

    void shouldCreateNewItemsInContext()
    {
        // GIVEN
        // a context item
        Akonadi::Item contextItem;
        contextItem.setId(42);

        // the context related to the item
        auto context = Domain::Context::Ptr::create();

        // a default collection
        Akonadi::Collection defaultCollection(42);

        // A task and its corresponding item not existing in storage yet
        Akonadi::Item taskItem;
        auto task = Domain::Task::Ptr::create();

        // A mock create job
        auto itemCreateJob = new FakeJob(this);

        // serializer mock returning the item for the task
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;

        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).thenReturn(taskItem);
        serializerMock(&Akonadi::SerializerInterface::addContextToTask).when(context, taskItem).thenReturn();

        // Storage mock returning the create job
        Utils::MockObject<Akonadi::StorageInterface> storageMock;

        storageMock(&Akonadi::StorageInterface::defaultCollection).when().thenReturn(defaultCollection);
        storageMock(&Akonadi::StorageInterface::createItem).when(taskItem, defaultCollection).thenReturn(itemCreateJob);

        // WHEN
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));

        repository->createInContext(task, context)->exec();

        // THEN

        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::addContextToTask).when(context, taskItem).exactly(1));

        QVERIFY(storageMock(&Akonadi::StorageInterface::defaultCollection).when().exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::createItem).when(taskItem, defaultCollection).exactly(1));
    }

    void shouldUpdateExistingItems()
    {
        // GIVEN

        // A default collection for saving
        Akonadi::Collection col(42);

        // A task and its corresponding item already existing in storage
        Akonadi::Item item(42);
        Domain::Task::Ptr task(new Domain::Task);

        // A mock create job
        auto itemModifyJob = new FakeJob(this);

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));

        // Storage mock returning the create job
        storageMock(&Akonadi::StorageInterface::updateItem).when(item, repository.get())
                                                           .thenReturn(itemModifyJob);

        // Serializer mock returning the item for the task
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).thenReturn(item);

        // WHEN
        repository->update(task)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, repository.get()).exactly(1));
    }

    void shouldRemoveATask_data()
    {
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<Testlib::AkonadiFakeItemFetchJob*>("itemFetchJob1");
        QTest::addColumn<Testlib::AkonadiFakeItemFetchJob*>("itemFetchJob2");
        QTest::addColumn<Akonadi::Item::List>("list");
        QTest::addColumn<bool>("itemFetchJobSucceeded");
        QTest::addColumn<bool>("collectionItemsFetchJobSucceeded");

        Akonadi::Collection col(40);

        Akonadi::Item item(42);
        item.setParentCollection(col);
        Akonadi::Item item2(43);

        auto itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << item);
        auto itemFetchJob2 = new Testlib::AkonadiFakeItemFetchJob(this);

        Akonadi::Item::List list;

        QTest::newRow("nominal case") << item << itemFetchJob1 << itemFetchJob2 << list << true << true;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setExpectedError(KJob::KilledJobError);
        QTest::newRow("item job error with empty list") << item << itemFetchJob1 << itemFetchJob2 << list << false << false;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setExpectedError(KJob::KilledJobError);
        itemFetchJob1->setItems(Akonadi::Item::List() << item);
        QTest::newRow("item job error with item") << item << itemFetchJob1 << itemFetchJob2 << list << false << false;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << item);
        itemFetchJob2 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob2->setExpectedError(KJob::KilledJobError);
        QTest::newRow("items job error with empty list") << item << itemFetchJob1 << itemFetchJob2 << list << true << false;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << item);
        itemFetchJob2 = new Testlib::AkonadiFakeItemFetchJob(this);
        list << item2;
        itemFetchJob2->setItems(list);
        QTest::newRow("remove item and his child") << item << itemFetchJob1 << itemFetchJob2 << list << true << true;
    }

    void shouldRemoveATask()
    {
        // GIVEN
        QFETCH(Akonadi::Item, item);
        QFETCH(Testlib::AkonadiFakeItemFetchJob*, itemFetchJob1);
        QFETCH(Testlib::AkonadiFakeItemFetchJob*, itemFetchJob2);
        QFETCH(Akonadi::Item::List, list);
        QFETCH(bool, itemFetchJobSucceeded);
        QFETCH(bool, collectionItemsFetchJobSucceeded);

        Domain::Task::Ptr task(new Domain::Task);

        Akonadi::Item::List removedList;
        removedList << list << item;

        // A mock delete job
        auto itemDeleteJob = new FakeJob(this);

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));

        // Storage mock returning the delete job
        storageMock(&Akonadi::StorageInterface::fetchItem).when(item, repository.get())
                                                          .thenReturn(itemFetchJob1);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(item.parentCollection(), repository.get())
                                                          .thenReturn(itemFetchJob2);
        storageMock(&Akonadi::StorageInterface::removeItems).when(removedList, repository.get())
                                                              .thenReturn(itemDeleteJob);

        // Serializer mock returning the item for the task
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).thenReturn(item);
        serializerMock(&Akonadi::SerializerInterface::filterDescendantItems).when(list, item).thenReturn(list);

        // WHEN
        repository->remove(task)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(item, repository.get()).exactly(1));
        if (itemFetchJobSucceeded) {
            QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(item.parentCollection(), repository.get()).exactly(1));
            if (collectionItemsFetchJobSucceeded) {
                QVERIFY(storageMock(&Akonadi::StorageInterface::removeItems).when(removedList, repository.get()).exactly(1));
            }
        }
    }

    void shouldPromoteTaskToProject()
    {
        // GIVEN

        // A default collection for saving
        Akonadi::Collection col(42);

        // A task and its corresponding item already existing in storage
        Akonadi::Item item(42);
        Domain::Task::Ptr task(new Domain::Task);

        // A mock fetch job
        auto itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << item);

        // A mock modify job
        auto itemModifyJob = new FakeJob(this);

        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));

        // Serializer mock returning the item for the task and transforming it into a project
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).thenReturn(item);
        serializerMock(&Akonadi::SerializerInterface::promoteItemToProject).when(item).thenReturn();

        // Storage mock returning the modify job
        storageMock(&Akonadi::StorageInterface::fetchItem).when(item, repository.get())
                                                          .thenReturn(itemFetchJob);
        storageMock(&Akonadi::StorageInterface::updateItem).when(item, repository.get())
                                                           .thenReturn(itemModifyJob);

        // WHEN
        repository->promoteToProject(task)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(task).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::promoteItemToProject).when(item).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(item, repository.get()).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, repository.get()).exactly(1));
    }

    void shouldAssociateATaskToAnother_data()
    {
        QTest::addColumn<Akonadi::Item>("childItem");
        QTest::addColumn<Akonadi::Item>("parentItem");
        QTest::addColumn<Domain::Task::Ptr>("child");
        QTest::addColumn<Domain::Task::Ptr>("parent");
        QTest::addColumn<Testlib::AkonadiFakeItemFetchJob*>("itemFetchJob1");
        QTest::addColumn<Testlib::AkonadiFakeItemFetchJob*>("itemFetchJob2");
        QTest::addColumn<Testlib::AkonadiFakeItemFetchJob*>("itemFetchJob3");
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

        auto itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        auto itemFetchJob2 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << childItem << parentItem);
        auto itemFetchJob3 = new Testlib::AkonadiFakeItemFetchJob(this);

        Akonadi::Item::List list;

        QTest::newRow("nominal case") << childItem << parentItem << child << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << true << true << list;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setExpectedError(KJob::KilledJobError);
        QTest::newRow("child job error with empty list") << childItem << parentItem << child << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << false << false << list;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setExpectedError(KJob::KilledJobError);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        QTest::newRow("child job error with item") << childItem << parentItem << child << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << false << false << list;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        itemFetchJob2 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob2->setExpectedError(KJob::KilledJobError);
        QTest::newRow("parent job error with empty list") << childItem << parentItem << child << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << true << false << list;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        itemFetchJob2 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob2->setExpectedError(KJob::KilledJobError);
        itemFetchJob2->setItems(Akonadi::Item::List() << childItem << parentItem);
        QTest::newRow("parent job error with item") << childItem << parentItem << child << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << true << false << list;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        itemFetchJob2 = new Testlib::AkonadiFakeItemFetchJob(this);
        Akonadi::Collection col2(39);
        Akonadi::Item parentItem2(41);
        parentItem2.setParentCollection(col2);
        itemFetchJob2->setItems(Akonadi::Item::List() << parentItem2);
        itemFetchJob3 = new Testlib::AkonadiFakeItemFetchJob(this);
        QTest::newRow("update and move item") << childItem << parentItem2 << child << parent << itemFetchJob1 << itemFetchJob2 << itemFetchJob3 << true << true << list;

        itemFetchJob1 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob1->setItems(Akonadi::Item::List() << childItem);
        itemFetchJob2 = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob2->setItems(Akonadi::Item::List() << parentItem2);
        itemFetchJob3 = new Testlib::AkonadiFakeItemFetchJob(this);
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
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));

        // Storage mock returning the create job
        storageMock(&Akonadi::StorageInterface::fetchItem).when(childItem, repository.get())
                                                          .thenReturn(itemFetchJob1);
        storageMock(&Akonadi::StorageInterface::fetchItems).when(parentItem.parentCollection(), repository.get())
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
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(parent).thenReturn(parentItem);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(childItem).thenReturn(child);
        serializerMock(&Akonadi::SerializerInterface::createTaskFromItem).when(parentItem).thenReturn(parent);
        serializerMock(&Akonadi::SerializerInterface::updateItemParent).when(childItem, parent).thenReturn();
        serializerMock(&Akonadi::SerializerInterface::itemUid).when(parentItem).thenReturn(QStringLiteral("parent"));
        serializerMock(&Akonadi::SerializerInterface::itemUid).when(childItem).thenReturn(QStringLiteral("child"));
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(parentItem).thenReturn(QString());
        serializerMock(&Akonadi::SerializerInterface::relatedUidFromItem).when(childItem).thenReturn(QString());
        if (execParentJob)
            serializerMock(&Akonadi::SerializerInterface::filterDescendantItems).when(list, childItem).thenReturn(list);

        // WHEN
        auto associateJob = repository->associate(parent, child);
        if (execJob)
            associateJob->exec();


        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(child).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(childItem, repository.get()).exactly(1));
        if (execJob) {
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::updateItemParent).when(childItem, parent).exactly(1));
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(parent).exactly(1));
            QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(parentItem.parentCollection(), repository.get()).exactly(1));
            if (execParentJob) {
                if (parentItem.parentCollection().id() == childItem.parentCollection().id())
                    QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, repository.get()).exactly(1));
                else {
                    //QVERIFY(serializerMock(&Akonadi::SerializerInterface::filterDescendantItems).when(list, childItem).exactly(1));
                    QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItems).when(childItem.parentCollection(), repository.get()).exactly(1));
                    QVERIFY(storageMock(&Akonadi::StorageInterface::createTransaction).when(repository.get()).thenReturn(transactionJob).exactly(1));
                    QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, transactionJob).exactly(1));
                    QVERIFY(storageMock(&Akonadi::StorageInterface::moveItems).when(movedList, parentItem.parentCollection(), transactionJob).exactly(1));
                }
            }
        }
    }

    void shouldPreventCyclesDuringAssociation()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());

        // Three tasks in the collection (one being child of the second one)
        data.createItem(GenTodo().withId(42).withParent(42)
                                 .withTitle(QStringLiteral("42")).withUid(QStringLiteral("uid-42")));
        data.createItem(GenTodo().withId(43).withParent(42)
                                 .withTitle(QStringLiteral("43")).withUid(QStringLiteral("uid-43"))
                                 .withParentUid(QStringLiteral("uid-42")));
        data.createItem(GenTodo().withId(44).withParent(42)
                                 .withTitle(QStringLiteral("44")).withUid(QStringLiteral("uid-44"))
                                 .withParentUid(QStringLiteral("uid-43")));

        auto serializer = Akonadi::Serializer::Ptr(new Akonadi::Serializer);
        auto task42 = serializer->createTaskFromItem(data.item(42));
        auto task44 = serializer->createTaskFromItem(data.item(44));

        auto monitor = Akonadi::MonitorInterface::Ptr(data.createMonitor());
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(Akonadi::StorageInterface::Ptr(data.createStorage()),
                                                                                       serializer));
        QSignalSpy spy(monitor.data(), &Akonadi::MonitorInterface::itemChanged);

        // WHEN
        auto job = repository->associate(task44, task42);
        QVERIFY(!job->exec());

        // THEN
        QVERIFY(spy.isEmpty());
        QVERIFY(job->error() != KJob::NoError);
        QVERIFY(!job->errorText().isEmpty());
    }

    void shouldDissociateATaskFromItsParent_data()
    {
        QTest::addColumn<Domain::Task::Ptr>("child");
        QTest::addColumn<Akonadi::Item>("childItem");
        QTest::addColumn<Testlib::AkonadiFakeItemFetchJob*>("itemFetchJob");
        QTest::addColumn<bool>("childJobFailed");

        Domain::Task::Ptr child(new Domain::Task);
        Akonadi::Item childItem(42);

        auto itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setItems(Akonadi::Item::List() << childItem);

        QTest::newRow("nominal case") << child << childItem << itemFetchJob << false;

        itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setExpectedError(KJob::KilledJobError);
        QTest::newRow("child job error with empty list") << child << childItem << itemFetchJob << true;

        itemFetchJob = new Testlib::AkonadiFakeItemFetchJob(this);
        itemFetchJob->setExpectedError(KJob::KilledJobError);
        itemFetchJob->setItems(Akonadi::Item::List() << childItem);
        QTest::newRow("child job error with item") << child << childItem << itemFetchJob << true;
    }

    void shouldDissociateATaskFromItsParent()
    {
        // GIVEN
        QFETCH(Domain::Task::Ptr, child);
        QFETCH(Akonadi::Item, childItem);
        QFETCH(Testlib::AkonadiFakeItemFetchJob*, itemFetchJob);
        QFETCH(bool, childJobFailed);

        auto itemModifyJob = new FakeJob(this);

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(storageMock.getInstance(),
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
        if (!childJobFailed) {
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::removeItemParent).when(childItem).exactly(1));;
            QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, repository.get()).exactly(1));
        }
    }

    void shouldDissociateAllLinksOfTask_data()
    {
        shouldDissociateATaskFromItsParent_data();
    }

    void shouldDissociateAllLinksOfTask()
    {
        // GIVEN
        QFETCH(Domain::Task::Ptr, child);
        QFETCH(Akonadi::Item, childItem);
        QFETCH(Testlib::AkonadiFakeItemFetchJob*, itemFetchJob);
        QFETCH(bool, childJobFailed);

        auto itemModifyJob = new FakeJob(this);

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::TaskRepository> repository(new Akonadi::TaskRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));

        // Storage mock returning the delete job
        storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, repository.get())
                                                           .thenReturn(itemModifyJob);
        storageMock(&Akonadi::StorageInterface::fetchItem).when(childItem, repository.get())
                                                          .thenReturn(itemFetchJob);

        // Serializer mock returning the item for the task
        serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(child).thenReturn(childItem);
        serializerMock(&Akonadi::SerializerInterface::removeItemParent).when(childItem).thenReturn();
        serializerMock(&Akonadi::SerializerInterface::clearItem).when(any<Akonadi::Item*>()).thenReturn();

        // WHEN
        repository->dissociateAll(child)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromTask).when(child).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchItem).when(childItem, repository.get()).exactly(1));
        if (!childJobFailed) {
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::removeItemParent).when(childItem).exactly(1));
            QVERIFY(serializerMock(&Akonadi::SerializerInterface::clearItem).when(any<Akonadi::Item*>()).exactly(1));
            QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(childItem, repository.get()).exactly(1));
        }

        // Give a chance to job to delete themselves
        // in case of an error (since they use deleteLater() internally)
        QTest::qWait(10);
    }
};

ZANSHIN_TEST_MAIN(AkonadiTaskRepositoryTest)

#include "akonaditaskrepositorytest.moc"
