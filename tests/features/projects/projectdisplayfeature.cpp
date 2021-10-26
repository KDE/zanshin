/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Project content
//   As someone collecting tasks
//   I can display a project
//   In order to see the artifacts associated to it
class ProjectDisplayFeature : public QObject
{
    Q_OBJECT
private slots:
    void Project_tasks_appear_in_the_corresponding_page()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Projects / TestData » Calendar1 / Read List"));
        And(c.I_look_at_the_central_list());
        When(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display" },
                             {
                                 { "\"Clean Code\" by Robert C Martin" },
                                 { "\"Domain Driven Design\" by Eric Evans" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(ProjectDisplayFeature)

#include "projectdisplayfeature.moc"
