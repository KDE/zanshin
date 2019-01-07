/* This file is part of Zanshin

   Copyright 2014 Mario Bensi <mbensi@ipsquad.net>

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

#include "utils/compositejob.h"

#include "testlib/fakejob.h"

using namespace Utils;

class CompositeJobTest : public QObject
{
    Q_OBJECT
public:
    explicit CompositeJobTest(QObject *parent = nullptr)
        : QObject(parent)
        , m_callCount(0)
    {
    }

private:
    int m_callCount;

private slots:
    void shouldCallHandlers()
    {
        // GIVEN
        int callCount = 0;

        auto handler = [&]() {
            callCount++;
        };

        FakeJob *job1 = new FakeJob(this);
        FakeJob *job2 = new FakeJob(this);
        CompositeJob *compositeJob = new CompositeJob(this);
        compositeJob->setAutoDelete(false);
        QVERIFY(compositeJob->install(job1, handler));
        QVERIFY(compositeJob->install(job2, handler));

        // WHEN
        compositeJob->start();
        QTest::qWait(FakeJob::DURATION + 10);

        // THEN
        QCOMPARE(callCount, 2);
        QVERIFY(!compositeJob->error());
        delete compositeJob;
    }

    void shouldCallHandlersWithJob()
    {
        // GIVEN
        int callCount = 0;
        QList<KJob*> seenJobs;

        auto handlerWithJob = [&](KJob *job) {
            callCount++;
            seenJobs << job;
        };

        FakeJob *job1 = new FakeJob(this);
        FakeJob *job2 = new FakeJob(this);
        CompositeJob *compositeJob = new CompositeJob(this);
        compositeJob->setAutoDelete(false);
        QVERIFY(compositeJob->install(job1, handlerWithJob));
        QVERIFY(compositeJob->install(job2, handlerWithJob));

        // WHEN
        compositeJob->start();
        QTest::qWait(FakeJob::DURATION + 10);

        // THEN
        QCOMPARE(callCount, 2);
        QCOMPARE(seenJobs.toSet(), QSet<KJob*>() << job1 << job2);
        QVERIFY(!compositeJob->error());
        delete compositeJob;
    }

    void handleJobResult(KJob*)
    {
        m_callCount++;
    }

    void shouldCallJobInHandler()
    {
        // GIVEN
        CompositeJob *compositeJob = new CompositeJob(this);
        compositeJob->setAutoDelete(false);

        auto handler = [&]() {
            FakeJob *job2 = new FakeJob(this);
            QObject::connect(job2, &KJob::result, this, &CompositeJobTest::handleJobResult);
            compositeJob->addSubjob(job2);
            job2->start();
        };

        FakeJob *job1 = new FakeJob(this);
        QVERIFY(compositeJob->install(job1, handler));

        // WHEN
        compositeJob->start();
        QTest::qWait(FakeJob::DURATION*2 + 10);

        QCOMPARE(m_callCount, 1);
        QVERIFY(!compositeJob->error());
        delete compositeJob;
    }

    void shouldEmitErrorFromHandler()
    {
        // GIVEN
        CompositeJob *compositeJob = new CompositeJob(this);
        compositeJob->setAutoDelete(false);

        auto handler = [&]() {
            compositeJob->emitError(QStringLiteral("Error reached"));
        };

        FakeJob *job = new FakeJob(this);
        QVERIFY(compositeJob->install(job, handler));

        // WHEN
        compositeJob->start();
        QTest::qWait(FakeJob::DURATION*2 + 10);

        QCOMPARE(compositeJob->error(), static_cast<int>(KJob::UserDefinedError));
        QCOMPARE(compositeJob->errorText(), QStringLiteral("Error reached"));
        delete compositeJob;
    }

    void shouldNotEmitResultTwiceOnSecondSubJobError()
    {
        // GIVEN
        int callCount = 0;

        auto handler = [&]() {
            callCount++;
        };

        FakeJob *job1 = new FakeJob(this);
        FakeJob *job2 = new FakeJob(this);
        job2->setExpectedError(KJob::UserDefinedError, "Fake error");

        CompositeJob *compositeJob = new CompositeJob(this);
        compositeJob->setAutoDelete(false);
        compositeJob->install(job1, handler);
        compositeJob->install(job2, handler);

        QSignalSpy spy(compositeJob, &KJob::result);

        // WHEN
        compositeJob->start();
        QTest::qWait(FakeJob::DURATION*2 + 10);

        // THEN
        QCOMPARE(spy.count(), 1);
        delete compositeJob;

    }
};

ZANSHIN_TEST_MAIN(CompositeJobTest)

#include "compositejobtest.moc"
