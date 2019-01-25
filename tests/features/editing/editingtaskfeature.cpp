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
};

ZANSHIN_TEST_MAIN(EditingTaskFeature)

#include "editingtaskfeature.moc"
