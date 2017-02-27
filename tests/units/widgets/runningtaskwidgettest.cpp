/* This file is part of Zanshin

   Copyright 2016-2017 David Faure <faure@kde.org>

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

#include "widgets/runningtaskwidget.h"
#include "presentation/runningtaskmodelinterface.h"
#include <QAbstractButton>

class RunningTaskModelStub : public Presentation::RunningTaskModelInterface
{
    Q_OBJECT
public:
    Domain::Task::Ptr runningTask() const Q_DECL_OVERRIDE { return m_runningTask; }
    void setRunningTask(const Domain::Task::Ptr &runningTask) Q_DECL_OVERRIDE
    {
        m_runningTask = runningTask;
        emit runningTaskChanged(m_runningTask);
    }

public slots:
    void stopTask() Q_DECL_OVERRIDE
    {
        Q_ASSERT(m_runningTask);
        setRunningTask(Domain::Task::Ptr());
    }

    void doneTask() Q_DECL_OVERRIDE
    {
        Q_ASSERT(m_runningTask);
        m_runningTask->setDone(true);
        stopTask();
    }

private:
    Domain::Task::Ptr m_runningTask;
};

class RunningTaskWidgetTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldHaveDefaultState()
    {
        Widgets::RunningTaskWidget widget;
        QVERIFY(widget.isHidden());
    }

    void shouldShowWhenRunningATask()
    {
        // GIVEN
        Widgets::RunningTaskWidget widget;
        auto task = Domain::Task::Ptr::create();
        RunningTaskModelStub model;
        widget.setModel(&model);

        // WHEN
        model.setRunningTask(task);

        // THEN
        QVERIFY(!widget.isHidden());
    }

    void shouldShowWhenRunningADifferentTask()
    {
        // GIVEN
        Widgets::RunningTaskWidget widget;
        auto task1 = Domain::Task::Ptr::create();
        auto task2 = Domain::Task::Ptr::create();
        RunningTaskModelStub model;
        widget.setModel(&model);
        model.setRunningTask(task1);

        // WHEN
        model.setRunningTask(task2);

        // THEN
        QVERIFY(!widget.isHidden());
    }

    void shouldStopAndHideOnClickingStop()
    {
        // GIVEN
        Widgets::RunningTaskWidget widget;
        auto task = Domain::Task::Ptr::create();
        RunningTaskModelStub model;
        widget.setModel(&model);
        model.setRunningTask(task);
        auto button = widget.findChild<QAbstractButton *>("stopButton");
        QVERIFY(button);

        // WHEN
        button->click();

        // THEN stopTask should have been called
        QCOMPARE(model.runningTask(), Domain::Task::Ptr());
        QVERIFY(widget.isHidden());
    }

    void shouldMarkAsDoneAndHideOnClickingDone()
    {
        // GIVEN
        Widgets::RunningTaskWidget widget;
        auto task = Domain::Task::Ptr::create();
        RunningTaskModelStub model;
        widget.setModel(&model);
        model.setRunningTask(task);
        QVERIFY(!task->isDone());
        auto button = widget.findChild<QAbstractButton *>("doneButton");
        QVERIFY(button);

        // WHEN
        button->click();

        // THEN doneTask should have been called
        QVERIFY(task->isDone());
        QVERIFY(widget.isHidden());
    }

    void shouldHideOnExternalStop()
    {
        // GIVEN
        Widgets::RunningTaskWidget widget;
        auto task = Domain::Task::Ptr::create();
        RunningTaskModelStub model;
        widget.setModel(&model);
        model.setRunningTask(task);

        // WHEN
        model.stopTask();

        // THEN
        QVERIFY(widget.isHidden());
    }

    void shouldHideOnExternalDone()
    {
        // GIVEN
        Widgets::RunningTaskWidget widget;
        auto task = Domain::Task::Ptr::create();
        RunningTaskModelStub model;
        widget.setModel(&model);
        model.setRunningTask(task);

        // WHEN
        model.doneTask();

        // THEN
        QVERIFY(widget.isHidden());
    }
};

ZANSHIN_TEST_MAIN(RunningTaskWidgetTest)

#include "runningtaskwidgettest.moc"
