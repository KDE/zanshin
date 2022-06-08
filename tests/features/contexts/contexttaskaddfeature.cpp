/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Task creation from a context
//     As someone collecting tasks
//     I can add a task directly associated to a context
//     In order to give my tasks some semantic
class ContextTaskAddFeature : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void Task_added_from_a_context_appear_in_its_list()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Contexts / Online"));
        When(c.I_add_a_task("Checking mail"));
        And(c.I_look_at_the_central_list());
        When(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display" },
                             {
                                 { "Checking mail" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(ContextTaskAddFeature)

#include "contexttaskaddfeature.moc"
