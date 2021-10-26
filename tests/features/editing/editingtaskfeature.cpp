/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Editing tasks
//   As an organized person
//   I can edit a previously created task
//   In order to refine its definition and react to change
class EditingTaskFeature : public QObject
{
    Q_OBJECT
private slots:
    void Editing_a_task_text()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_central_list("Buy cheese"));
        When(c.I_open_the_item_in_the_editor());
        And(c.I_change_the_editor_field("text", "More information"));
        And(c.I_open_the_item_in_the_editor_again());
        Then(c.the_editor_shows_the_field("text", "More information"));
        And(c.the_editor_shows_the_field("title", "Buy cheese"));
    }

    void Editing_a_task_title_using_the_editor()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_central_list("Buy cheese"));
        When(c.I_open_the_item_in_the_editor());
        And(c.I_change_the_editor_field("title", "Borrow cheese"));
        And(c.I_open_the_item_in_the_editor_again());
        Then(c.the_editor_shows_the_field("title", "Borrow cheese"));
        And(c.there_is_an_item_in_the_central_list("Borrow cheese"));
    }

    void Editing_a_task_title_in_the_central_list()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_central_list("Buy cheese"));
        When(c.I_open_the_item_in_the_editor());
        And(c.I_rename_the_item("Buy better cheese"));
        Then(c.the_editor_shows_the_field("title", "Buy better cheese"));
    }

    void Editing_a_task_start_date()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_central_list("Buy cheese"));
        When(c.I_open_the_item_in_the_editor());
        And(c.I_change_the_editor_field("start date", "2014-06-20"));
        And(c.I_open_the_item_in_the_editor_again());
        Then(c.the_editor_shows_the_field("start date", "2014-06-20"));
    }

    void Editing_a_task_due_date()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_central_list("Buy cheese"));
        When(c.I_open_the_item_in_the_editor());
        And(c.I_change_the_editor_field("due date", "2014-07-20"));
        And(c.I_open_the_item_in_the_editor_again());
        Then(c.the_editor_shows_the_field("due date", "2014-07-20"));
    }

    void Editing_a_task_in_the_central_list_of_a_context_page()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Contexts / Errands"));
        And(c.I_look_at_the_central_list());
        And(c.there_is_an_item_in_the_central_list("Buy kiwis"));
        When(c.I_open_the_item_in_the_editor());
        And(c.I_rename_the_item("Buy better kiwis"));
        Then(c.the_editor_shows_the_field("title", "Buy better kiwis"));
        And(c.there_is_an_item_in_the_central_list("Buy better kiwis"));
    }
};

ZANSHIN_TEST_MAIN(EditingTaskFeature)

#include "editingtaskfeature.moc"
