/* This file is part of Zanshin

   Copyright 2017 Kevin Ottens <ervin@kde.org>

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
    explicit AkonadiCachingStorageIntegrationTest(QObject *parent = Q_NULLPTR)
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
