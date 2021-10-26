/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Workday content
//   As someone using tasks
//   I can display the Workday list
//   In order to know which tasks should be completed today (e.g. if start date or due date is today or in the past)
class WorkdayDisplayFeature : public QObject
{
    Q_OBJECT
private slots:
    void The_tasks_that_need_to_be_done_today_appear_in_the_Workday_list()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Workday"));
        And(c.I_look_at_the_central_list());
        When(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display" },
                             {
                                 { "\"Clean Code\" by Robert C Martin" },
                                 { "Buy kiwis" },
                                 { "Buy pears" },
                                 { "Errands" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(WorkdayDisplayFeature)

#include "workdaydisplayfeature.moc"
