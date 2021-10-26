/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Display completed tasks in Workday list
//   As someone using tasks
//   I can display the Workday list
//   In order to know which tasks I’ve completed today (e.g. if done date is today)
class WorkdayDisplayCompletedFeature : public QObject
{
    Q_OBJECT
private slots:
    void The_tasks_that_have_been_done_today_appear_in_the_Workday_list()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_central_list("Buy rutabagas"));
        When(c.I_check_the_item());
        And(c.I_display_the_page("Workday"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_contains("Buy rutabagas"));
    }
};

ZANSHIN_TEST_MAIN(WorkdayDisplayCompletedFeature)

#include "workdaydisplaycompletedfeature.moc"
