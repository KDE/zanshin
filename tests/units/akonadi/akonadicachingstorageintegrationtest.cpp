/*
 * SPDX-FileCopyrightText: 2017 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include <testlib/akonadistoragetestbase.h>
#include <testlib/testsafety.h>

#include "akonadi/akonadicachingstorage.h"
#include "akonadi/akonadimonitorimpl.h"
#include "akonadi/akonadiserializer.h"
#include "akonadi/akonadistorage.h"

class AkonadiCachingStorageIntegrationTest : public Testlib::AkonadiStorageTestBase
{
    Q_OBJECT
public:
    explicit AkonadiCachingStorageIntegrationTest(QObject *parent = nullptr)
        : AkonadiStorageTestBase(parent)
    {
    }

    Akonadi::StorageInterface::Ptr createStorage() override
    {
        auto serializer = Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
        return Akonadi::StorageInterface::Ptr(new Akonadi::CachingStorage(Akonadi::Cache::Ptr::create(serializer,
                                                                                                      createMonitor()),
                                                                          Akonadi::StorageInterface::Ptr(new Akonadi::Storage)));
    }

    Akonadi::MonitorInterface::Ptr createMonitor() override
    {
        return Akonadi::MonitorInterface::Ptr(new Akonadi::MonitorImpl);
    }

private slots:
    void initTestCase()
    {
        QVERIFY(TestLib::TestSafety::checkTestIsIsolated());
    }
};

ZANSHIN_TEST_MAIN(AkonadiCachingStorageIntegrationTest)

#include "akonadicachingstorageintegrationtest.moc"
