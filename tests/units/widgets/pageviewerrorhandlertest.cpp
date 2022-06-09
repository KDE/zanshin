/*
 * SPDX-FileCopyrightText: 2016 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_gui_zanshin.h>

#include <KMessageWidget>

#include "widgets/pageviewerrorhandler.h"
#include "widgets/pageview.h"

#include "testlib/fakejob.h"

class PageViewErrorHandlerTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldHaveDefaultState()
    {
        Widgets::PageViewErrorHandler errorHandler;
        QVERIFY(!errorHandler.pageView());
    }

    void shouldHaveAPageView()
    {
        // GIVEN
        Widgets::PageViewErrorHandler errorHandler;
        Widgets::PageView page;

        // WHEN
        errorHandler.setPageView(&page);

        // THEN
        QCOMPARE(errorHandler.pageView(), &page);
    }

    void shouldDisplayMessageOnJobError()
    {
        // GIVEN
        Widgets::PageViewErrorHandler errorHandler;
        Widgets::PageView page;
        page.show();
        QVERIFY(QTest::qWaitForWindowExposed(&page));
        QTest::qWait(100);

        errorHandler.setPageView(&page);

        auto messageWidget = page.findChild<KMessageWidget*>(QStringLiteral("messageWidget"));
        QVERIFY(messageWidget);
        QVERIFY(!messageWidget->isVisibleTo(&page));

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(1, "Bar's fault");
        errorHandler.installHandler(job, "Foo Failed");
        QSignalSpy spy(job, &KJob::result);
        QVERIFY(spy.wait(FakeJob::DURATION * 2));

        // THEN
        QVERIFY(messageWidget->isVisibleTo(&page));
        QVERIFY(messageWidget->isShowAnimationRunning());
        QCOMPARE(messageWidget->text(), QStringLiteral("Foo Failed: Bar's fault"));
    }

    void shouldNotCrashWhenNoViewIsAvailable()
    {
        // GIVEN
        Widgets::PageViewErrorHandler errorHandler;

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(1, "Bar's fault");
        errorHandler.installHandler(job, "Foo Failed");
        QSignalSpy spy(job, &KJob::result);
        QVERIFY(spy.wait(FakeJob::DURATION * 2));

        // THEN
        // We survive this test
    }
};

ZANSHIN_TEST_MAIN(PageViewErrorHandlerTest)

#include "pageviewerrorhandlertest.moc"
