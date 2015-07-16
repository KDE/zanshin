/* This file is part of Zanshin

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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
    explicit AkonadiFakeStorageTest(QObject *parent = Q_NULLPTR)
        : Testlib::AkonadiStorageTestBase(parent)
    {
        MonitorSpy::setExpirationDelay(100);
        auto loader = Testlib::AkonadiFakeDataXmlLoader(&m_data);
        loader.load(SOURCE_DIR "/akonadifakedataxmlloadertest.xml");
    }

    Akonadi::StorageInterface::Ptr createStorage() Q_DECL_OVERRIDE
    {
        return Akonadi::StorageInterface::Ptr(m_data.createStorage());
    }

    Akonadi::MonitorInterface::Ptr createMonitor() Q_DECL_OVERRIDE
    {
        return Akonadi::MonitorInterface::Ptr(m_data.createMonitor());
    }

private:
    Testlib::AkonadiFakeData m_data;
};

QTEST_MAIN(AkonadiFakeStorageTest)

#include "akonadifakestoragetest.moc"
