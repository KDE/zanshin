/*
 * SPDX-FileCopyrightText: 2014-2015 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include <testlib/akonadistoragetestbase.h>
#include <testlib/testsafety.h>

#include "akonadi/akonadimonitorimpl.h"
#include "akonadi/akonadistorage.h"

class AkonadiStorageTest : public Testlib::AkonadiStorageTestBase
{
    Q_OBJECT
public:
    explicit AkonadiStorageTest(QObject *parent = nullptr)
        : AkonadiStorageTestBase(parent)
    {
    }

    Akonadi::StorageInterface::Ptr createStorage() override
    {
        return Akonadi::StorageInterface::Ptr(new Akonadi::Storage);
    }

    Akonadi::MonitorInterface::Ptr createMonitor() override
    {
        Akonadi::MonitorInterface::Ptr ptr(new Akonadi::MonitorImpl);
        QTest::qWait(10); // give Monitor time to upload settings
        return ptr;
    }

private Q_SLOTS:
    void initTestCase()
    {
        QVERIFY(TestLib::TestSafety::checkTestIsIsolated());
    }
};

ZANSHIN_TEST_MAIN(AkonadiStorageTest)

#include "akonadistoragetest.moc"
