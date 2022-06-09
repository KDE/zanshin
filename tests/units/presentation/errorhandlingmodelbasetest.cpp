/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "presentation/errorhandlingmodelbase.h"
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

class FakeErrorHandlingModelBase : public Presentation::ErrorHandlingModelBase
{
public:
    void install(KJob *job, const QString &message)
    {
        installHandler(job, message);
    }
};

class ErrorHandlingModelBaseTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldDisplayMessageOnError()
    {
        // GIVEN

        // create job
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));

        // create ErrorHandlingModelBase
        FakeErrorHandler errorHandler;
        FakeErrorHandlingModelBase errorHandling;
        errorHandling.setErrorHandler(&errorHandler);

        const QString message = QStringLiteral("I Failed !!!!!!!!!!");

        // WHEN
        errorHandling.install(job, message);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("I Failed !!!!!!!!!!: Foo"));
        QCOMPARE(errorHandling.errorHandler(), &errorHandler);
    }

    void shouldNotDisplayMessageWhenNoErrorOccurred()
    {
        // GIVEN

        // create job
        auto job = new FakeJob(this);

        // create ErrorHandlingModelBase
        FakeErrorHandler errorHandler;
        FakeErrorHandlingModelBase errorHandling;
        errorHandling.setErrorHandler(&errorHandler);

        const QString message = QStringLiteral("I Failed !!!!!!!!!!");

        // WHEN
        errorHandling.install(job, message);

        // THEN
        QTest::qWait(150);
        QVERIFY(errorHandler.m_message.isEmpty());
    }
};

ZANSHIN_TEST_MAIN(ErrorHandlingModelBaseTest)

#include "errorhandlingmodelbasetest.moc"
