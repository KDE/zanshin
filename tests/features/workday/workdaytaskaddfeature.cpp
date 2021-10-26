/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Adding a task from the workday view
//   As someone adding tasks
//   I can input a task for today in the quick entry
//   In order to have a new task that starts today
class WorkdayTaskAddFeature : public QObject
{
    Q_OBJECT
private slots:
    void Tasks_added_from_the_workday_view_start_today()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Workday"));
        When(c.I_look_at_the_central_list());
        And(c.I_add_a_task("Burn some confidential documents"));
        And(c.I_list_the_items());
        Then(c.the_list_contains("Burn some confidential documents"));
    }
};

ZANSHIN_TEST_MAIN(WorkdayTaskAddFeature)

#include "workdaytaskaddfeature.moc"
