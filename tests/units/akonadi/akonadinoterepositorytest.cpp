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

#include "utils/mockobject.h"

#include "testlib/akonadifakejobs.h"
#include "testlib/akonadifakemonitor.h"

#include "akonadi/akonadinoterepository.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"
#include "akonadi/akonadistoragesettings.h"

using namespace mockitopp;

Q_DECLARE_METATYPE(Testlib::AkonadiFakeItemFetchJob*)

class AkonadiNoteRepositoryTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldCheckIfASourceIsDefaultFromSettings()
    {
        // GIVEN

        // A default collection for saving
        Akonadi::Collection col(42);
        Akonadi::StorageSettings::instance().setDefaultNoteCollection(col);

        // The data source corresponding to the default collection
        auto source = Domain::DataSource::Ptr::create();

        // Storage mock sitting here doing nothing
        Utils::MockObject<Akonadi::StorageInterface> storageMock;

        // Serializer mock returning the collection for the source
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).thenReturn(col);

        // WHEN
        QScopedPointer<Akonadi::NoteRepository> repository(new Akonadi::NoteRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));

        // THEN
        QVERIFY(repository->isDefaultSource(source));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).exactly(1));
    }

    void shouldNoticeIfSourceIsNotDefault()
    {
        // GIVEN

        // A default collection for saving
        Akonadi::Collection defaultCol(42);
        Akonadi::StorageSettings::instance().setDefaultNoteCollection(defaultCol);

        // Another random collection
        Akonadi::Collection col(43);

        // The data source corresponding to the random collection
        auto source = Domain::DataSource::Ptr::create();

        // Storage mock sitting here doing nothing
        Utils::MockObject<Akonadi::StorageInterface> storageMock;

        // Serializer mock returning the collection for the source
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).thenReturn(col);

        // WHEN
        QScopedPointer<Akonadi::NoteRepository> repository(new Akonadi::NoteRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));

        // THEN
        QVERIFY(!repository->isDefaultSource(source));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).exactly(1));
    }

    void shouldStoreDefaultSourceInTheSettings()
    {
        // GIVEN

        // A value in the settings
        auto &settings = Akonadi::StorageSettings::instance();
        settings.setDefaultNoteCollection(Akonadi::Collection(21));

        // The new default data source we want
        auto source = Domain::DataSource::Ptr::create();

        // A collection corresponding to the data source
        Akonadi::Collection col(42);

        // Storage mock sitting here doing nothing
        Utils::MockObject<Akonadi::StorageInterface> storageMock;

        // Serializer mock returning the collection for the source
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).thenReturn(col);

        // WHEN
        QScopedPointer<Akonadi::NoteRepository> repository(new Akonadi::NoteRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));
        repository->setDefaultSource(source);

        // THEN
        QCOMPARE(settings.defaultNoteCollection(), col);
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).exactly(1));
    }


    void shouldCreateNewItems()
    {
        // GIVEN

        // A default collection for saving
        auto col = Akonadi::Collection(42);

        // A note and its corresponding item not existing in storage yet
        auto item = Akonadi::Item();
        auto note = Domain::Note::Ptr::create();

        // A mock create job
        auto itemCreateJob = new FakeJob(this);

        // Storage mock returning the create job
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::defaultNoteCollection).when().thenReturn(col);
        storageMock(&Akonadi::StorageInterface::createItem).when(item, col)
                                                           .thenReturn(itemCreateJob);

        // Serializer mock returning the item for the note
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromNote).when(note).thenReturn(item);

        // WHEN
        QScopedPointer<Akonadi::NoteRepository> repository(new Akonadi::NoteRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));
        repository->create(note)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromNote).when(note).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::defaultNoteCollection).when().exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::createItem).when(item, col).exactly(1));
    }

    void shouldCreateNewItemsInFirstWritableCollectionIfNothingInSettings()
    {
        // GIVEN

        // A few collections
        auto col1 = Akonadi::Collection(42);
        col1.setRights(Akonadi::Collection::ReadOnly);
        auto col2 = Akonadi::Collection(43);
        auto col3 = Akonadi::Collection(44);
        auto collectionFetchJob = new Testlib::AkonadiFakeCollectionFetchJob;
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2 << col3);

        // A note and its corresponding item not existing in storage yet
        auto item = Akonadi::Item();
        auto note = Domain::Note::Ptr::create();

        // A mock create job
        auto itemCreateJob = new FakeJob(this);

        // Storage mock returning the create job and with no default collection
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::defaultNoteCollection).when().thenReturn(Akonadi::Collection());
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Notes)
                                                                 .thenReturn(collectionFetchJob);
        storageMock(&Akonadi::StorageInterface::createItem).when(item, col2)
                                                           .thenReturn(itemCreateJob);

        // Serializer mock returning the item for the note
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromNote).when(note).thenReturn(item);

        // WHEN
        QScopedPointer<Akonadi::NoteRepository> repository(new Akonadi::NoteRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));
        repository->create(note)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromNote).when(note).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::defaultNoteCollection).when().exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::createItem).when(item, col2).exactly(1));
    }

    void shouldCreateNewItemsInTag()
    {
        // GIVEN
        // a tag
        Akonadi::Tag akonadiTag;
        akonadiTag.setName("akonadiTag42");
        akonadiTag.setId(42);

        // the domain Tag related to the Akonadi Tag
        auto tag = Domain::Tag::Ptr::create();

        // a default collection
        Akonadi::Collection defaultCollection(42);

        // A note and its corresponding item not existing in storage yet
        Akonadi::Item noteItem;
        auto note = Domain::Note::Ptr::create();

        // A mock create job
        auto itemCreateJob = new FakeJob(this);

        // serializer mock returning the item for the task
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;

        serializerMock(&Akonadi::SerializerInterface::createAkonadiTagFromTag).when(tag).thenReturn(akonadiTag);
        serializerMock(&Akonadi::SerializerInterface::createItemFromNote).when(note).thenReturn(noteItem);

        // Storage mock returning the create job
        Utils::MockObject<Akonadi::StorageInterface> storageMock;

        storageMock(&Akonadi::StorageInterface::defaultNoteCollection).when().thenReturn(defaultCollection);
        storageMock(&Akonadi::StorageInterface::createItem).when(noteItem, defaultCollection).thenReturn(itemCreateJob);

        // WHEN
        QScopedPointer<Akonadi::NoteRepository> repository(new Akonadi::NoteRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));

        repository->createInTag(note, tag)->exec();

        // THEN

        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromNote).when(note).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createAkonadiTagFromTag).when(tag).exactly(1));

        QVERIFY(storageMock(&Akonadi::StorageInterface::defaultNoteCollection).when().exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::createItem).when(noteItem, defaultCollection).exactly(1));
    }

    void shouldUpdateExistingItems()
    {
        // GIVEN

        // A default collection for saving
        auto col = Akonadi::Collection(42);

        // A note and its corresponding item already existing in storage
        auto item = Akonadi::Item(42);
        auto note = Domain::Note::Ptr::create();

        // A mock create job
        auto itemModifyJob = new FakeJob(this);

        // Storage mock returning the create job
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::updateItem).when(item, Q_NULLPTR)
                                                           .thenReturn(itemModifyJob);

        // Serializer mock returning the item for the task
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromNote).when(note).thenReturn(item);

        // WHEN
        QScopedPointer<Akonadi::NoteRepository> repository(new Akonadi::NoteRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));
        repository->update(note)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromNote).when(note).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, Q_NULLPTR).exactly(1));
    }

    void shouldRemoveANote()
    {
        // GIVEN
        Akonadi::Item item(42);
        Domain::Note::Ptr note(new Domain::Note);

        // A mock delete job
        auto itemDeleteJob = new FakeJob(this);

        // Storage mock returning the delete job
        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::removeItem).when(item)
                                                           .thenReturn(itemDeleteJob);

        // Serializer mock returning the item for the note
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromNote).when(note).thenReturn(item);

        // WHEN
        QScopedPointer<Akonadi::NoteRepository> repository(new Akonadi::NoteRepository(storageMock.getInstance(),
                                                                                       serializerMock.getInstance()));
        repository->remove(note)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromNote).when(note).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::removeItem).when(item).exactly(1));
    }
};

QTEST_MAIN(AkonadiNoteRepositoryTest)

#include "akonadinoterepositorytest.moc"
