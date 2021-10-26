/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Context content
//   As someone collecting tasks
//   I can display a context
//   In order to see the tasks associated to it
class ContextDisplayFeature : public QObject
{
    Q_OBJECT
private slots:
    void Context_tasks_appear_in_the_corresponding_page()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Contexts / Errands"));
        And(c.I_look_at_the_central_list());
        When(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display" },
                             {
                                 { "Buy kiwis" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(ContextDisplayFeature)

#include "contextdisplayfeature.moc"
