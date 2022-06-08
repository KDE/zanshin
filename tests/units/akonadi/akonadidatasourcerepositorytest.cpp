/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "utils/mockobject.h"

#include "testlib/akonadifakejobs.h"
#include "testlib/akonadifakemonitor.h"

#include "akonadi/akonadidatasourcerepository.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;

Q_DECLARE_METATYPE(Testlib::AkonadiFakeItemFetchJob*)

class AkonadiDataSourceRepositoryTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void shouldUpdateExistingSources()
    {
        // GIVEN

        // A source and its corresponding collection already existing in storage
        Akonadi::Collection collection(42);
        auto source = Domain::DataSource::Ptr::create();

        // A mock modify job
        auto collectionModifyJob = new FakeJob(this);

        Utils::MockObject<Akonadi::StorageInterface> storageMock;
        Utils::MockObject<Akonadi::SerializerInterface> serializerMock;
        QScopedPointer<Akonadi::DataSourceRepository> repository(new Akonadi::DataSourceRepository(storageMock.getInstance(),
                                                                                                   serializerMock.getInstance()));

        // Storage mock returning the create job
        storageMock(&Akonadi::StorageInterface::updateCollection).when(collection, repository.get())
                                                                 .thenReturn(collectionModifyJob);

        // Serializer mock returning the item for the project
        serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).thenReturn(collection);

        // WHEN
        repository->update(source)->exec();

        // THEN
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createCollectionFromDataSource).when(source).exactly(1));
        QVERIFY(storageMock(&Akonadi::StorageInterface::updateCollection).when(collection, repository.get()).exactly(1));
    }
};

ZANSHIN_TEST_MAIN(AkonadiDataSourceRepositoryTest)

#include "akonadidatasourcerepositorytest.moc"
