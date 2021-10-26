/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Postponing a task
//   As someone using tasks
//   I can change the start or due date of a task
//   In order to procrastinate
class WorkdayPostponingFeature : public QObject
{
    Q_OBJECT
private slots:
    void Setting_a_date_s_start_date_to_a_date_in_the_future_makes_it_disappear_in_the_Workday_page()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Workday"));
        And(c.there_is_an_item_in_the_central_list("Errands"));
        When(c.I_open_the_item_in_the_editor());
        And(c.I_change_the_editor_field("start date", "2015-03-20"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_does_not_contain("Errands"));
    }

    void Setting_a_date_s_due_date_to_a_date_in_the_future_makes_it_disappear_in_the_Workday_page()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Workday"));
        And(c.there_is_an_item_in_the_central_list("Buy kiwis"));
        When(c.I_open_the_item_in_the_editor());
        And(c.I_change_the_editor_field("due date", "2015-03-20"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_does_not_contain("Buy kiwis"));
    }
};

ZANSHIN_TEST_MAIN(WorkdayPostponingFeature)

#include "workdaypostponingfeature.moc"
