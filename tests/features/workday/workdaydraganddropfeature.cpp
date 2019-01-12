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

// Feature: Drag and drop a task in workday view
//   As someone reviewing his tasks in the workday view
//   I can drag task within the workday view
//   In order to change parent/child relationships
class WorkdayDragAndDropFeature : public QObject
{
    Q_OBJECT
private slots:
    void Parenting_a_task_in_the_Workday_page()
    {
        ZANSHIN_CONTEXT;
        Given(I_display_the_page("Workday"));
        And(I_add_a_task("parent"));
        And(I_add_a_task("child"));
        And(there_is_an_item_in_the_central_list("parent"));
        And(there_is_an_item_in_the_central_list("child"));
        When(I_drop_the_item_on_the_central_list("parent"));
        And(I_look_at_the_central_list());
        And(I_list_the_items());
        Then(the_central_list_contains_items_named({
                                 "parent",
                                 "parent / child",
                             }));
    }

    void Deparenting_a_task_in_the_Workday_page()
    {
        ZANSHIN_CONTEXT;
        Given(I_display_the_page("Workday"));
        And(I_add_a_task("parent"));
        And(there_is_an_item_in_the_central_list("parent"));
        And(I_add_a_task_child("child", "parent"));
        And(there_is_an_item_in_the_central_list("parent / child"));
        When(I_drop_the_item_on_the_blank_area_of_the_central_list());
        And(I_look_at_the_central_list());
        And(I_list_the_items());
        Then(the_central_list_contains_items_named({
                                 "parent",
                                 "child",
                             }));
    }
};

ZANSHIN_TEST_MAIN(WorkdayDragAndDropFeature)

#include "workdaydraganddropfeature.moc"
