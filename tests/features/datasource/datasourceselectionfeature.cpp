/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_available_data_sources("TestData / Calendar1"));
        When(c.I_uncheck_the_item());
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_is({
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
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.there_is_an_item_in_the_available_data_sources("TestData / Calendar1"));
        When(c.I_check_the_item());
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        Then(c.the_list_is({
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
        ZanshinContext c;
        Given(c.there_is_an_item_in_the_available_data_sources("TestData / Calendar1"));
        When(c.I_uncheck_the_item());
        And(c.I_display_the_available_pages());
        And(c.I_list_the_items());
        Then(c.the_list_is({
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
                                 { "All Tasks" },
                             }
                         }));
    }

    void Checking_impacts_project_list()
    {
        ZanshinContext c;
        Given(c.there_is_an_item_in_the_available_data_sources("TestData / Calendar1"));
        When(c.I_check_the_item());
        And(c.I_display_the_available_pages());
        And(c.I_list_the_items());
        Then(c.the_list_is({
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
                                 { "All Tasks" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(DatasourceSelectionFeature)

#include "datasourceselectionfeature.moc"
