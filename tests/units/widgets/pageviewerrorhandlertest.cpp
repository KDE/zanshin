/* This file is part of Zanshin

   Copyright 2016 Kevin Ottens <ervin@kde.org>

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
        QVERIFY(QTest::qWaitForWindowShown(&page));
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
