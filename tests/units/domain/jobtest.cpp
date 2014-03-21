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

#include <QtTest>

#include "domain/job.h"

using namespace Domain;

class FakeJob : public Job
{
    Q_OBJECT
public:
    static const int DURATION = 100;

    explicit FakeJob(QObject *parent = 0)
        : Job(parent)
    {
    }

    void start()
    {
        QTimer::singleShot(DURATION, this, SLOT(onTimeout()));
    }

private slots:
    void onTimeout()
    {
        emitResult();
    }
};

class JobTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldAutodelete()
    {
        QPointer<FakeJob> job(new FakeJob(this));
        QVERIFY(!job.isNull());
        job->start();
        QVERIFY(!job.isNull());
        QTest::qWait(FakeJob::DURATION + 10);
        QVERIFY(job.isNull());
    }

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
        job1->addHandler(handler);
        job1->addHandler(handlerWithJob);
        job1->start();

        FakeJob *job2 = new FakeJob(this);
        job2->addHandler(handler);
        job2->addHandler(handlerWithJob);
        job2->start();

        QTest::qWait(FakeJob::DURATION + 10);

        QCOMPARE(callCount, 4);
        QCOMPARE(seenJobs.toSet(), QSet<KJob*>() << job1 << job2);
    }
};

QTEST_MAIN(JobTest)

#include "jobtest.moc"
