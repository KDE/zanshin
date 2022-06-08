/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "presentation/errorhandler.h"

#include "testlib/fakejob.h"

class FakeErrorHandler : public Presentation::ErrorHandler
{
public:
    void doDisplayMessage(const QString &message) override
    {
        m_message = message;
    }

    QString m_message;
};

class ErrorHandlerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void shouldDisplayErrorMessage()
    {
        // GIVEN

        // create job
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));

        // create ErrorHandler
        FakeErrorHandler errorHandler;

        const QString message = QStringLiteral("I Failed !!!!!!!!!!");

        // WHEN
        errorHandler.installHandler(job, message);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("I Failed !!!!!!!!!!: Foo"));
    }

    void shouldDisplayNothing()
    {
        // GIVEN

        // create job
        auto job = new FakeJob(this);

        // create ErrorHandler
        FakeErrorHandler errorHandler;

        const QString message = QStringLiteral("I Failed !!!!!!!!!!");

        // WHEN
        errorHandler.installHandler(job, message);

        // THEN
        QTest::qWait(150);
        QVERIFY(errorHandler.m_message.isEmpty());
    }
};

ZANSHIN_TEST_MAIN(ErrorHandlerTest)

#include "errorhandlertest.moc"
