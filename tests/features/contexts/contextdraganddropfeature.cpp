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

// Feature: Context task association
//   As someone collecting tasks
//   I can associate tasks to a context
//   In order to describe the tasks resources
class ContextDragAndDropFeature : public QObject
{
    Q_OBJECT
private slots:
    void Dropping_a_task_on_a_context_from_the_inbox()
    {
        ZANSHIN_CONTEXT;
        Given(I_display_the_page("Inbox"));
        And(there_is_an_item_in_the_central_list("Buy rutabagas"));
        When(I_drop_the_item_on_the_page_list("Contexts / Errands"));
        And(I_display_the_page("Contexts / Errands"));
        And(I_look_at_the_central_list());
        And(I_list_the_items());
        Then(the_list_is({
                             { "display" },
                             {
                                 { "Buy kiwis" },
                                 { "Buy rutabagas" },
                             }
                         }));
    }

    void Dropping_a_task_on_a_context_from_the_project_central_list()
    {
        ZANSHIN_CONTEXT;
        Given(I_display_the_page("Projects / TestData » Calendar1 / Prepare talk about TDD"));
        And(there_is_an_item_in_the_central_list("Create examples and exercices"));
        When(I_drop_the_item_on_the_page_list("Contexts / Online"));
        And(I_display_the_page("Contexts / Online"));
        And(I_look_at_the_central_list());
        And(I_list_the_items());
        Then(the_list_is({
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
