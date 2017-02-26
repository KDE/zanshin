/* This file is part of Zanshin

   Copyright 2017 David Faure <faure@kde.org>

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

#include "widgets/pageview.h"
#include "domain/task.h"

#include "widgets/taskapplicationcomponents.h"
#include "widgets/runningtaskwidget.h"
#include "presentation/runningtaskmodelinterface.h"

class RunningTaskModelStub : public Presentation::RunningTaskModelInterface
{
    Q_OBJECT
public:
    Domain::Task::Ptr runningTask() const Q_DECL_OVERRIDE { return {}; }
    void setRunningTask(const Domain::Task::Ptr &) Q_DECL_OVERRIDE {}
public slots:
    void stopTask() Q_DECL_OVERRIDE {}
    void doneTask() Q_DECL_OVERRIDE {}
};

class TaskApplicationComponentsTest : public QObject
{
    Q_OBJECT
public:
    explicit TaskApplicationComponentsTest(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
    }

private slots:
    void shouldApplyRunningTaskModelToPageView()
    {
        // GIVEN
        Widgets::TaskApplicationComponents components;
        auto appModelStub = QObjectPtr::create();
        RunningTaskModelStub runningTaskModelStub;
        appModelStub->setProperty("runningTaskModel", QVariant::fromValue(&runningTaskModelStub));

        // WHEN
        components.setModel(appModelStub);
        auto pageView = components.pageView();

        // THEN
        QCOMPARE(pageView->runningTaskModel(), &runningTaskModelStub);
    }

    void shouldApplyRunningTaskModelToRunningTaskWidget()
    {
        // GIVEN
        Widgets::TaskApplicationComponents components;
        auto appModelStub = QObjectPtr::create();
        RunningTaskModelStub runningTaskModelStub;
        appModelStub->setProperty("runningTaskModel", QVariant::fromValue(&runningTaskModelStub));

        // WHEN
        components.setModel(appModelStub);
        auto runningTaskWidget = components.runningTaskWidget();

        // THEN
        QCOMPARE(runningTaskWidget->model(), &runningTaskModelStub);
    }
};

ZANSHIN_TEST_MAIN(TaskApplicationComponentsTest)

#include "taskapplicationcomponentstest.moc"
