/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Completing tasks
//   As someone who just accomplished something
//   I can mark a task as completed
//   In order to have a warm feeling about getting it done
class CompletingTaskFeature : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void Checking_task_in_the_list()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_central_list("Buy cheese"));
        When(c.I_check_the_item());
        Then(c.the_task_corresponding_to_the_item_is_done());
    }

    void Checking_task_in_the_editor()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_central_list("Buy apples"));
        When(c.I_open_the_item_in_the_editor());
        And(c.I_mark_the_item_done_in_the_editor());
        And(c.I_open_the_item_in_the_editor_again());
        Then(c.the_task_corresponding_to_the_item_is_done());
        And(c.the_editor_shows_the_task_as_done());
    }
};

ZANSHIN_TEST_MAIN(CompletingTaskFeature)

#include "completingtaskfeature.moc"
