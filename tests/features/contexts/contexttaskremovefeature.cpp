/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Context task dissociation
//   As someone collecting tasks
//   I can delete a task related to a context
//   In order to keep my context meaningful
class ContextTaskRemoveFeature : public QObject
{
    Q_OBJECT
private slots:
    void Removing_a_task_from_a_context_keeps_it_in_the_project_page_it_s_linked_to()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Projects / TestData » Calendar1 / Prepare talk about TDD"));
        And(c.there_is_an_item_in_the_central_list("Create examples and exercices"));
        And(c.I_drop_the_item_on_the_page_list("Contexts / Online"));
        And(c.I_display_the_page("Contexts / Online"));
        And(c.there_is_an_item_in_the_central_list("Create examples and exercices"));
        When(c.I_remove_the_item());
        And(c.I_look_at_the_central_list());
        Then(c.the_list_does_not_contain("Create examples and exercices"));
        And(c.I_display_the_page("Projects / TestData » Calendar1 / Prepare talk about TDD"));
        Then(c.there_is_an_item_in_the_central_list("Create examples and exercices"));
    }

    void Removing_a_task_linked_only_to_a_context_moves_it_back_to_the_inbox()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.I_look_at_the_central_list());
        Then(c.the_list_is({
                             { "display" },
                             {
                             }
                         }));
        And(c.I_display_the_page("Contexts / Errands"));
        And(c.there_is_an_item_in_the_central_list("Buy kiwis"));
        When(c.I_remove_the_item());
        And(c.I_look_at_the_central_list());
        Then(c.the_list_does_not_contain("Buy kiwis"));
        And(c.I_display_the_page("Inbox"));
        Then(c.there_is_an_item_in_the_central_list("Buy kiwis"));
    }
};

ZANSHIN_TEST_MAIN(ContextTaskRemoveFeature)

#include "contexttaskremovefeature.moc"
