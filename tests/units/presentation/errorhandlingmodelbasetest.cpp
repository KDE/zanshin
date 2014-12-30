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

#include <QtTest>

#include "presentation/errorhandlingmodelbase.h"
#include "presentation/errorhandler.h"

#include "testlib/fakejob.h"

class FakeErrorHandler : public Presentation::ErrorHandler
{
public:
    void doDisplayMessage(const QString &message)
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
        job->setExpectedError(KJob::KilledJobError, "Foo");

        // create ErrorHandlingModelBase
        FakeErrorHandler errorHandler;
        FakeErrorHandlingModelBase errorHandling;
        errorHandling.setErrorHandler(&errorHandler);

        const QString message = "I Failed !!!!!!!!!!";

        // WHEN
        errorHandling.installHandler(job, message);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("I Failed !!!!!!!!!!: Foo"));
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

        const QString message = "I Failed !!!!!!!!!!";

        // WHEN
        errorHandling.installHandler(job, message);

        // THEN
        QTest::qWait(150);
        QVERIFY(errorHandler.m_message.isEmpty());
    }
};

QTEST_MAIN(ErrorHandlingModelBaseTest)

#include "errorhandlingModelBasetest.moc"
