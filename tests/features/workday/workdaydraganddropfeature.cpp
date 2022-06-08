/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
private Q_SLOTS:
    void Parenting_a_task_in_the_Workday_page()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Workday"));
        And(c.I_add_a_task("parent"));
        And(c.I_add_a_task("child"));
        And(c.there_is_an_item_in_the_central_list("parent"));
        And(c.there_is_an_item_in_the_central_list("child"));
        When(c.I_drop_the_item_on_the_central_list("parent"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_central_list_contains_items_named({
                                 "parent",
                                 "parent / child",
                             }));
    }

    void Deparenting_a_task_in_the_Workday_page()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Workday"));
        And(c.I_add_a_task("parent"));
        And(c.there_is_an_item_in_the_central_list("parent"));
        And(c.I_add_a_task_child("child", "parent"));
        And(c.there_is_an_item_in_the_central_list("parent / child"));
        When(c.I_drop_the_item_on_the_blank_area_of_the_central_list());
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        QEXPECT_FAIL("", "Setting the date during deparenting is broken, see the TODO in workdaypagemodel.cpp", Continue);
        Then(c.the_central_list_contains_items_named({
                                 "parent",
                                 "child",
                             }));
    }
};

ZANSHIN_TEST_MAIN(WorkdayDragAndDropFeature)

#include "workdaydraganddropfeature.moc"
