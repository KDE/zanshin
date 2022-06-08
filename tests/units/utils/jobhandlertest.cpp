/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
        return {list.cbegin(), list.cend()};
    }
}

class JobHandlerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
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
