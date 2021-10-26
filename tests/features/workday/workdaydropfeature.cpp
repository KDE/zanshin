/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Drop a task on workday view
//   As someone reviewing his tasks
//   I can drag a task from the current view and drop it in the workday view
//   In order to have the task start today
class WorkdayDropFeature : public QObject
{
    Q_OBJECT
private slots:
    void Dropping_a_task_on_Workday_page()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.I_add_a_task("Buy Pineapples"));
        And(c.there_is_an_item_in_the_central_list("Buy Pineapples"));
        When(c.I_drop_the_item_on_the_page_list("Workday"));
        And(c.I_display_the_page("Workday"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_contains("Buy Pineapples"));
    }

    void Dropping_two_tasks_on_Workday_page()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.I_add_a_task("Don't eat the cake"));
        And(c.I_add_a_task("The cake is a lie"));
        And(c.the_central_list_contains_items_named({
                                 "Don't eat the cake",
                                 "The cake is a lie",
                             }));
        When(c.I_drop_items_on_the_page_list("Workday"));
        And(c.I_display_the_page("Workday"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_contains("Don't eat the cake"));
        And(c.the_list_contains("The cake is a lie"));
    }
};

ZANSHIN_TEST_MAIN(WorkdayDropFeature)

#include "workdaydropfeature.moc"
