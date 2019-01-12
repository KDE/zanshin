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

// Feature: Data sources selection
//   As an advanced user
//   I can select or deselect sources
//   In order to see more or less content
class DatasourceSelectionFeature : public QObject
{
    Q_OBJECT
private slots:
    void Unchecking_impacts_the_inbox()
    {
        ZANSHIN_CONTEXT;
        Given(I_display_the_page("Inbox"));
        And(there_is_an_item_in_the_available_data_sources("TestData / Calendar1"));
        When(I_uncheck_the_item());
        And(I_look_at_the_central_list());
        And(I_list_the_items());
        Then(the_list_is({
                             { "display" },
                             {
                                 { "Buy apples" },
                                 { "Buy pears" },
                                 { "Errands" },
                             }
                         }));
    }

    void Checking_impacts_the_inbox()
    {
        ZANSHIN_CONTEXT;
        Given(I_display_the_page("Inbox"));
        And(there_is_an_item_in_the_available_data_sources("TestData / Calendar1"));
        When(I_check_the_item());
        And(I_look_at_the_central_list());
        And(I_list_the_items());
        Then(the_list_is({
                             { "display" },
                             {
                                 { "\"Capital in the Twenty-First Century\" by Thomas Piketty" },
                                 { "\"The Pragmatic Programmer\" by Hunt and Thomas" },
                                 { "Buy cheese" },
                                 { "Buy kiwis" },
                                 { "Buy apples" },
                                 { "Buy pears" },
                                 { "Errands" },
                                 { "Buy rutabagas" },
                             }
                         }));
    }

    void Unchecking_impacts_project_list()
    {
        ZANSHIN_CONTEXT;
        Given(there_is_an_item_in_the_available_data_sources("TestData / Calendar1"));
        When(I_uncheck_the_item());
        And(I_display_the_available_pages());
        And(I_list_the_items());
        Then(the_list_is({
                             { "display" },
                             {
                                 { "Inbox" },
                                 { "Workday" },
                                 { "Projects" },
                                 { "Projects / TestData » Calendar1 » Calendar2" },
                                 { "Projects / TestData » Calendar1 » Calendar2 / Backlog" },
                                 { "Contexts" },
                                 { "Contexts / Errands" },
                                 { "Contexts / Online" },
                             }
                         }));
    }

    void Checking_impacts_project_list()
    {
        ZANSHIN_CONTEXT;
        Given(there_is_an_item_in_the_available_data_sources("TestData / Calendar1"));
        When(I_check_the_item());
        And(I_display_the_available_pages());
        And(I_list_the_items());
        Then(the_list_is({
                             { "display" },
                             {
                                 { "Inbox" },
                                 { "Workday" },
                                 { "Projects" },
                                 { "Projects / TestData » Calendar1" },
                                 { "Projects / TestData » Calendar1 / Prepare talk about TDD" },
                                 { "Projects / TestData » Calendar1 / Read List" },
                                 { "Projects / TestData » Calendar1 » Calendar2" },
                                 { "Projects / TestData » Calendar1 » Calendar2 / Backlog" },
                                 { "Contexts" },
                                 { "Contexts / Errands" },
                                 { "Contexts / Online" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(DatasourceSelectionFeature)

#include "datasourceselectionfeature.moc"
