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

#include "akonadi/akonadiprojectrepository.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;

class AkonadiProjectRepositoryTest : public QObject
{
    Q_OBJECT
private slots:
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
        auto itemCreateJob = new MockAkonadiJob(this);

        // Storage mock returning the create job
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::createItem).when(item, collection)
                                                           .thenReturn(itemCreateJob);

        // Serializer mock
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project).thenReturn(item);
        serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).thenReturn(collection);

        // WHEN
        QScopedPointer<Akonadi::ProjectRepository> repository(new Akonadi::ProjectRepository(&storageMock.getInstance(),
                                                                                             &serializerMock.getInstance()));
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
        auto itemModifyJob = new MockAkonadiJob(this);

        // Storage mock returning the create job
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::updateItem).when(item, 0)
                                                           .thenReturn(itemModifyJob);

        // Serializer mock returning the item for the project
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project).thenReturn(item);

        // WHEN
        QScopedPointer<Akonadi::ProjectRepository> repository(new Akonadi::ProjectRepository(&storageMock.getInstance(),
                                                                                             &serializerMock.getInstance()));
        repository->update(project)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::updateItem).when(item, 0).exactly(1));
    }

    void shouldRemoveExistingProject()
    {
        // GIVEN

        // A project and its corresponding item already existing in storage
        Akonadi::Item item(42);
        auto project = Domain::Project::Ptr::create();

        // A mock remove job
        auto itemRemoveJob = new MockAkonadiJob(this);

        // Storage mock returning the create job
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::removeItem).when(item)
                                                           .thenReturn(itemRemoveJob);

        // Serializer mock returning the item for the project
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createItemFromProject).when(project).thenReturn(item);

        // WHEN
        QScopedPointer<Akonadi::ProjectRepository> repository(new Akonadi::ProjectRepository(&storageMock.getInstance(),
                                                                                             &serializerMock.getInstance()));
        repository->remove(project)->exec();

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::removeItem).when(item).exactly(1));
    }
};

QTEST_MAIN(AkonadiProjectRepositoryTest)

#include "akonadiprojectrepositorytest.moc"
