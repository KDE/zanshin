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

// Feature: Completing tasks
//   As someone who just accomplished something
//   I can mark a task as completed
//   In order to have a warm feeling about getting it done
class CompletingTaskFeature : public QObject
{
    Q_OBJECT
private slots:
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
