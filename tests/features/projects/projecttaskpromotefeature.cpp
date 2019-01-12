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

// Feature: Task promotion
// As someone collecting tasks
// I can promote a task into a project
// In order to organize my tasks
class ProjectTaskPromoteFeature : public QObject
{
    Q_OBJECT
private slots:
    void Task_promoted_into_a_project_appears_in_the_list()
    {
        ZANSHIN_CONTEXT;
        Given(I_display_the_page("Projects / TestData » Calendar1 » Calendar2 / Backlog"));
        And(I_add_a_task("Design a present"));
        And(I_look_at_the_central_list());
        And(there_is_an_item_in_the_central_list("Design a present"));
        When(I_promote_the_item());
        And(I_display_the_available_pages());
        And(I_list_the_items());
        Then(the_list_is({
                             { "display", "icon" },
                             {
                                 { "Inbox", "mail-folder-inbox" },
                                 { "Workday", "go-jump-today" },
                                 { "Projects", "folder" },
                                 { "Projects / TestData » Calendar1", "folder" },
                                 { "Projects / TestData » Calendar1 / Prepare talk about TDD", "view-pim-tasks" },
                                 { "Projects / TestData » Calendar1 / Read List", "view-pim-tasks" },
                                 { "Projects / TestData » Calendar1 » Calendar2", "folder" },
                                 { "Projects / TestData » Calendar1 » Calendar2 / Backlog", "view-pim-tasks" },
                                 { "Projects / TestData » Calendar1 » Calendar2 / Design a present", "view-pim-tasks" },
                                 { "Contexts", "folder" },
                                 { "Contexts / Errands", "view-pim-notes" },
                                 { "Contexts / Online", "view-pim-notes" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(ProjectTaskPromoteFeature)

#include "projecttaskpromotefeature.moc"
