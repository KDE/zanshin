/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Task creation from a project
// As someone collecting tasks
// I can add a task directly inside a project
// In order to organize my tasks
class ProjectTaskAddFeature : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void Task_added_from_a_project_appear_in_its_list()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Projects / TestData » Calendar1 » Calendar2 / Backlog"));
        When(c.I_add_a_task("Buy a cake"));
        And(c.I_add_a_task("Buy a present"));
        And(c.I_look_at_the_central_list());
        When(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display" },
                             {
                                 { "Buy a cake" },
                                 { "Buy a present" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(ProjectTaskAddFeature)

#include "projecttaskaddfeature.moc"
