/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Modifying a task's due date
// As someone adding a task
// I can set the due date by editing the field "due date"
// In order to have a task that ends at the entered date
class WorkdayDuedateEditFeature : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void Setting_a_date_s_due_date_to_today_makes_it_appear_in_the_Workday_page()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.I_add_a_task("Make more test tasks"));
        And(c.there_is_an_item_in_the_central_list("Make more test tasks"));
        When(c.I_open_the_item_in_the_editor());
        And(c.I_change_the_editor_field("due date", "2015-03-10"));
        And(c.I_display_the_page("Workday"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_contains("Make more test tasks"));
    }

    void Setting_a_date_s_due_date_to_a_date_in_the_past_makes_it_appear_in_the_Workday_page()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.I_add_a_task("Buy potatoes"));
        And(c.there_is_an_item_in_the_central_list("Buy potatoes"));
        When(c.I_open_the_item_in_the_editor());
        And(c.I_change_the_editor_field("due date", "2001-03-10"));
        And(c.I_display_the_page("Workday"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_contains("Buy potatoes"));
    }
};

ZANSHIN_TEST_MAIN(WorkdayDuedateEditFeature)

#include "workdayduedateeditfeature.moc"
