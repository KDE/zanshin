/* This file is part of Zanshin

   Copyright 2014-2015 Kevin Ottens <ervin@kde.org>

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

private slots:
    void initTestCase()
    {
        QVERIFY(TestLib::TestSafety::checkTestIsIsolated());
    }
};

ZANSHIN_TEST_MAIN(AkonadiStorageTest)

#include "akonadistoragetest.moc"
