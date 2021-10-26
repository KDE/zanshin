/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include <testlib/akonadistoragetestbase.h>

#include "testlib/akonadifakedata.h"
#include "testlib/akonadifakedataxmlloader.h"
#include "testlib/akonadifakemonitor.h"
#include "testlib/akonadifakestorage.h"
#include "testlib/monitorspy.h"


class AkonadiFakeStorageTest : public Testlib::AkonadiStorageTestBase
{
    Q_OBJECT
public:
    explicit AkonadiFakeStorageTest(QObject *parent = nullptr)
        : Testlib::AkonadiStorageTestBase(parent)
    {
        MonitorSpy::setExpirationDelay(100);
        auto loader = Testlib::AkonadiFakeDataXmlLoader(&m_data);
        loader.load(SOURCE_DIR "/../akonadi/testenv/data/testdata.xml");
    }

    Akonadi::StorageInterface::Ptr createStorage() override
    {
        return Akonadi::StorageInterface::Ptr(m_data.createStorage());
    }

    Akonadi::MonitorInterface::Ptr createMonitor() override
    {
        return Akonadi::MonitorInterface::Ptr(m_data.createMonitor());
    }

private:
    Testlib::AkonadiFakeData m_data;
};

ZANSHIN_TEST_MAIN(AkonadiFakeStorageTest)

#include "akonadifakestoragetest.moc"
