/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Project destruction
//   As someone collecting tasks
//   I can delete a project
//   In order to clean my tasks
class ProjectRemoveFeature : public QObject
{
    Q_OBJECT
private slots:
    void Removing_a_simple_project_from_the_list()
    {
        ZanshinContext c;
        Given(c.I_display_the_available_pages());
        When(c.I_remove_a_page("Projects / TestData » Calendar1", "Prepare talk about TDD"));
        And(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display", "icon" },
                             {
                                 { "Inbox", "mail-folder-inbox" },
                                 { "Workday", "go-jump-today" },
                                 { "Projects", "folder" },
                                 { "Projects / TestData » Calendar1", "folder" },
                                 { "Projects / TestData » Calendar1 / Read List", "view-pim-tasks" },
                                 { "Projects / TestData » Calendar1 » Calendar2", "folder" },
                                 { "Projects / TestData » Calendar1 » Calendar2 / Backlog", "view-pim-tasks" },
                                 { "Contexts", "folder" },
                                 { "Contexts / Errands", "view-pim-notes" },
                                 { "Contexts / Online", "view-pim-notes" },
                                 { "All Tasks", "view-pim-tasks" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(ProjectRemoveFeature)

#include "projectremovefeature.moc"
