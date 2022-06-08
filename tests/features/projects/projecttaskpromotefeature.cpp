/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
private Q_SLOTS:
    void Task_promoted_into_a_project_appears_in_the_list()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Projects / TestData » Calendar1 » Calendar2 / Backlog"));
        And(c.I_add_a_task("Design a present"));
        And(c.I_look_at_the_central_list());
        And(c.there_is_an_item_in_the_central_list("Design a present"));
        When(c.I_promote_the_item());
        And(c.I_display_the_available_pages());
        And(c.I_list_the_items());
        Then(c.the_list_is({
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
                                 { "All Tasks", "view-pim-tasks" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(ProjectTaskPromoteFeature)

#include "projecttaskpromotefeature.moc"
