/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_central_list("\"The Pragmatic Programmer\" by Hunt and Thomas"));
        When(c.I_drop_the_item_on_the_page_list("Projects / TestData » Calendar1 / Read List"));
        And(c.I_display_the_page("Projects / TestData » Calendar1 / Read List"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_is({
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
        ZanshinContext c;
        Given(c.I_display_the_page("Contexts / Errands"));
        And(c.there_is_an_item_in_the_central_list("Buy kiwis"));
        When(c.I_drop_the_item_on_the_page_list("Projects / TestData » Calendar1 » Calendar2 / Backlog"));
        And(c.I_display_the_page("Projects / TestData » Calendar1 » Calendar2 / Backlog"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display" },
                             {
                                 { "Buy kiwis" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(ProjectDragAndDropFeature)

#include "projectdraganddropfeature.moc"
