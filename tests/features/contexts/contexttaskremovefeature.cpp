/* This file is part of Zanshin

   Copyright 2019 Kevin Ottens <ervin@kde.org>
   Copyright 2019 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
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
