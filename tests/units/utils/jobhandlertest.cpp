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

#include "utils/jobhandler.h"
#include "utils/compositejob.h"

#include "testlib/fakejob.h"

using namespace Utils;

namespace
{
    template<typename T>
    QSet<T> listToSet(const QList<T> &list)
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        return list.toSet();
#else
        return {list.cbegin(), list.cend()};
#endif
    }
}

class JobHandlerTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldCallHandlers()
    {
        int callCount = 0;
        QList<KJob*> seenJobs;

        auto handler = [&]() {
            callCount++;
        };

        auto handlerWithJob = [&](KJob *job) {
            callCount++;
            seenJobs << job;
        };

        FakeJob *job1 = new FakeJob(this);
        JobHandler::install(job1, handler);
        JobHandler::install(job1, handlerWithJob);
        QCOMPARE(JobHandler::jobCount(), 2);
        job1->start();

        FakeJob *job2 = new FakeJob(this);
        JobHandler::install(job2, handler);
        JobHandler::install(job2, handlerWithJob);
        QCOMPARE(JobHandler::jobCount(), 4);
        job2->start();

        QTest::qWait(FakeJob::DURATION + 10);

        QCOMPARE(callCount, 4);
        QCOMPARE(listToSet(seenJobs), QSet<KJob*>() << job1 << job2);
        QCOMPARE(JobHandler::jobCount(), 0);
    }
};

ZANSHIN_TEST_MAIN(JobHandlerTest)

#include "jobhandlertest.moc"
