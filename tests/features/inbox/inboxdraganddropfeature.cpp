/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Inbox task association
//   As someone collecting tasks
//   I can associate a task to another one
//   In order to deal with complex tasks requiring several steps
class InboxDragAndDropFeature : public QObject
{
    Q_OBJECT
private slots:
    void Dropping_a_task_on_another_one_makes_it_a_child()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_central_list("Buy apples"));
        When(c.I_drop_the_item_on_the_central_list("Errands"));
        And(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display" },
                             {
                                 { "Errands" },
                                 { "Errands / Buy apples" },
                                 { "\"Capital in the Twenty-First Century\" by Thomas Piketty" },
                                 { "\"The Pragmatic Programmer\" by Hunt and Thomas" },
                                 { "Buy cheese" },
                                 { "Buy kiwis" },
                                 { "Buy pears" },
                                 { "Buy rutabagas" }
                             }
                         }));
    }

    void Dropping_a_child_task_on_the_inbox_makes_it_top_level()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_central_list("Buy apples"));
        And(c.I_drop_the_item_on_the_central_list("Errands"));
        And(c.there_is_an_item_in_the_central_list("Errands / Buy apples"));
        When(c.I_drop_the_item_on_the_page_list("Inbox"));
        And(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display" },
                             {
                                 { "Errands" },
                                 { "Buy apples" },
                                 { "\"Capital in the Twenty-First Century\" by Thomas Piketty" },
                                 { "\"The Pragmatic Programmer\" by Hunt and Thomas" },
                                 { "Buy cheese" },
                                 { "Buy kiwis" },
                                 { "Buy pears" },
                                 { "Buy rutabagas" }
                             }
                         }));
    }

    void Dropping_two_tasks_on_another_one_makes_them_children()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.the_central_list_contains_items_named({"Buy apples", "Buy pears"}));
        When(c.I_drop_items_on_the_central_list("Errands"));
        And(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display" },
                             {
                                 { "Errands" },
                                 { "Errands / Buy apples" },
                                 { "Errands / Buy pears" },
                                 { "\"Capital in the Twenty-First Century\" by Thomas Piketty" },
                                 { "\"The Pragmatic Programmer\" by Hunt and Thomas" },
                                 { "Buy cheese" },
                                 { "Buy kiwis" },
                                 { "Buy rutabagas" }
                             }
                         }));
    }

    void Dropping_two_child_tasks_on_the_inbox_makes_them_top_level()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.the_central_list_contains_items_named({"Buy apples", "Buy pears"}));
        And(c.I_drop_items_on_the_central_list("Errands"));
        And(c.the_central_list_contains_items_named({"Errands / Buy apples", "Errands / Buy pears"}));
        When(c.I_drop_items_on_the_page_list("Inbox"));
        And(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display" },
                             {
                                 { "Errands" },
                                 { "Buy apples" },
                                 { "\"Capital in the Twenty-First Century\" by Thomas Piketty" },
                                 { "\"The Pragmatic Programmer\" by Hunt and Thomas" },
                                 { "Buy cheese" },
                                 { "Buy kiwis" },
                                 { "Buy pears" },
                                 { "Buy rutabagas" }
                             }
                         }));
    }

    void Dropping_a_task_on_the_inbox_removes_it_from_its_associated_project()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Projects / TestData » Calendar1 / Prepare talk about TDD"));
        And(c.there_is_an_item_in_the_central_list("Create Sozi SVG"));
        When(c.I_drop_the_item_on_the_page_list("Inbox"));
        And(c.I_display_the_page("Inbox"));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display" },
                             {
                                 { "Errands" },
                                 { "Buy apples" },
                                 { "\"Capital in the Twenty-First Century\" by Thomas Piketty" },
                                 { "\"The Pragmatic Programmer\" by Hunt and Thomas" },
                                 { "Buy cheese" },
                                 { "Buy kiwis" },
                                 { "Buy pears" },
                                 { "Buy rutabagas" },
                                 { "Create Sozi SVG" }
                             }
                         }));
    }

    void Deparenting_a_task_by_dropping_on_the_central_list_blank_area()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.I_look_at_the_central_list());
        And(c.there_is_an_item_in_the_central_list("Buy apples"));
        And(c.I_drop_the_item_on_the_central_list("Errands"));
        And(c.I_look_at_the_central_list());
        And(c.there_is_an_item_in_the_central_list("Errands / Buy apples"));
        When(c.I_drop_the_item_on_the_blank_area_of_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display" },
                             {
                                 { "Errands" },
                                 { "Buy apples" },
                                 { "\"Capital in the Twenty-First Century\" by Thomas Piketty" },
                                 { "\"The Pragmatic Programmer\" by Hunt and Thomas" },
                                 { "Buy cheese" },
                                 { "Buy kiwis" },
                                 { "Buy pears" },
                                 { "Buy rutabagas" }
                             }
                         }));
    }

    void Dropping_a_task_on_the_inbox_removes_it_from_all_its_contexts()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Contexts / Errands"));
        And(c.there_is_an_item_in_the_central_list("Buy kiwis"));
        When(c.I_drop_the_item_on_the_page_list("Inbox"));
        And(c.I_display_the_page("Contexts / Errands"));
        And(c.I_look_at_the_central_list());
        Then(c.the_list_does_not_contain("Buy kiwis"));
    }
};

ZANSHIN_TEST_MAIN(InboxDragAndDropFeature)

#include "inboxdraganddropfeature.moc"
