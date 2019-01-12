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

// Feature: Project task association
//   As someone collecting tasks
//   I can associate a task to a project
//   In order to organize my work
class ProjectDragAndDropFeature : public QObject
{
    Q_OBJECT
private slots:
    void Dropping_a_task_on_a_project()
    {
        ZANSHIN_CONTEXT;
        Given(I_display_the_page("Inbox"));
        And(there_is_an_item_in_the_central_list("\"The Pragmatic Programmer\" by Hunt and Thomas"));
        When(I_drop_the_item_on_the_page_list("Projects / TestData » Calendar1 / Read List"));
        And(I_display_the_page("Projects / TestData » Calendar1 / Read List"));
        And(I_look_at_the_central_list());
        And(I_list_the_items());
        Then(the_list_is({
                             { "display" },
                             {
                                 { "\"Clean Code\" by Robert C Martin" },
                                 { "\"Domain Driven Design\" by Eric Evans" },
                                 { "\"The Pragmatic Programmer\" by Hunt and Thomas" },
                             }
                         }));
    }

    void Dropping_a_task_on_a_project_from_context_central_list()
    {
        ZANSHIN_CONTEXT;
        Given(I_display_the_page("Contexts / Errands"));
        And(there_is_an_item_in_the_central_list("Buy kiwis"));
        When(I_drop_the_item_on_the_page_list("Projects / TestData » Calendar1 » Calendar2 / Backlog"));
        And(I_display_the_page("Projects / TestData » Calendar1 » Calendar2 / Backlog"));
        And(I_look_at_the_central_list());
        And(I_list_the_items());
        Then(the_list_is({
                             { "display" },
                             {
                                 { "Buy kiwis" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(ProjectDragAndDropFeature)

#include "projectdraganddropfeature.moc"
