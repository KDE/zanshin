/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Inbox content
//   As someone collecting tasks
//   I can display the Inbox
//   In order to see the tasks which need to be organized (e.g. any task not associated to any project or context)
class InboxDisplayFeature : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void Unorganized_tasks_appear_in_the_inbox()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Inbox"));
        And(c.I_look_at_the_central_list());
        When(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display" },
                             {
                                 { "Errands" },
                                 { "\"Capital in the Twenty-First Century\" by Thomas Piketty" },
                                 { "\"The Pragmatic Programmer\" by Hunt and Thomas" },
                                 { "Buy cheese" },
                                 { "Buy kiwis" },
                                 { "Buy apples" },
                                 { "Buy pears" },
                                 { "Buy rutabagas" }
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(InboxDisplayFeature)

#include "inboxdisplayfeature.moc"
