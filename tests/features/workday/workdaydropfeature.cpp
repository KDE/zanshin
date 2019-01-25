/* This file is part of Zanshin

   Copyright 2019 Kevin Ottens <ervin@kde.org>
   Copyright 2019 David Faure <faure@kde.org>

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
