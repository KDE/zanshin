/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Modifying a task's start date
// As someone using tasks
// I can modify the start date of tasks to today 
// In order to have them plan from today
class WorkdayStartdateEditFeature : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void Setting_a_date_s_start_date_to_today_makes_it_appear_in_the_workday_view()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_central_list("Buy rutabagas"));
        When(c.I_open_the_item_in_the_editor());
        And(c.I_change_the_editor_field("start date", "2015-03-10"));
        And(c.I_display_the_page("Workday"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_contains("Buy rutabagas"));
    }

    void Setting_a_date_s_start_date_to_a_date_in_the_past_makes_it_appear_in_the_workday_view()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Workday"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        And(c.the_list_does_not_contain("Buy rutabagas"));
        And(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_central_list("Buy rutabagas"));
        When(c.I_open_the_item_in_the_editor());
        And(c.I_change_the_editor_field("start date", "2014-03-10"));
        And(c.I_display_the_page("Workday"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_contains("Buy rutabagas"));
    }
};

ZANSHIN_TEST_MAIN(WorkdayStartdateEditFeature)

#include "workdaystartdateeditfeature.moc"
