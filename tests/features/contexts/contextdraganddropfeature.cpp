/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Context task association
//   As someone collecting tasks
//   I can associate tasks to a context
//   In order to describe the tasks resources
class ContextDragAndDropFeature : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void Dropping_a_task_on_a_context_from_the_inbox()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_central_list("Buy rutabagas"));
        When(c.I_drop_the_item_on_the_page_list("Contexts / Errands"));
        And(c.I_display_the_page("Contexts / Errands"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display" },
                             {
                                 { "Buy kiwis" },
                                 { "Buy rutabagas" },
                             }
                         }));
    }

    void Dropping_a_task_on_a_context_from_the_project_central_list()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Projects / TestData » Calendar1 / Prepare talk about TDD"));
        And(c.there_is_an_item_in_the_central_list("Create examples and exercices"));
        When(c.I_drop_the_item_on_the_page_list("Contexts / Online"));
        And(c.I_display_the_page("Contexts / Online"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display" },
                             {
                                 { "Create examples and exercices" },
                                 { "Create examples and exercices / Train for the FizzBuzz kata" },
                                 { "Create examples and exercices / Train for the Gilded Rose kata" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(ContextDragAndDropFeature)

#include "contextdraganddropfeature.moc"
