/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Removing a task
//   As someone using tasks
//   I can remove a task
//   In order to clean up
class WorkdayTaskRemoveFeature : public QObject
{
    Q_OBJECT
private slots:
    void Removing_a_task_that_appear_in_the_Workday_list()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Workday"));
        And(c.there_is_an_item_in_the_central_list("Buy pears"));
        When(c.I_remove_the_item());
        And(c.I_list_the_items());
        Then(c.the_list_does_not_contain("Buy pears"));
    }
};

ZANSHIN_TEST_MAIN(WorkdayTaskRemoveFeature)

#include "workdaytaskremovefeature.moc"
