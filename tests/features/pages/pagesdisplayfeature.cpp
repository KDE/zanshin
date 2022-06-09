/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
 * SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Available pages content
//   As someone collecting tasks
//   I can see all the pages available to me
//   In order to display them and add tasks to them
class PagesDisplayFeature : public QObject
{
    Q_OBJECT
private slots:
    void Inbox_projects_and_contexts_appear_in_the_list()
    {
        ZanshinContext c;
        Given(c.I_display_the_available_pages());
        When(c.I_list_the_items());
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
                                 { "Contexts", "folder" },
                                 { "Contexts / Errands", "view-pim-notes" },
                                 { "Contexts / Online", "view-pim-notes" },
                                 { "All Tasks", "view-pim-tasks" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(PagesDisplayFeature)

#include "pagesdisplayfeature.moc"
