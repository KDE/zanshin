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

// Feature: Context creation
//   As someone using tasks
//   I can create a context
//   In order to give them some semantic
class ContextAddFeature : public QObject
{
    Q_OBJECT
private slots:
    void New_contexts_appear_in_the_list()
    {
        ZANSHIN_CONTEXT;
        Given(I_display_the_available_pages());
        When(I_add_a_context("Internet"));
        And(I_list_the_items());
        Then(the_list_is({
                             { "display", "icon" },
                             {
                                 { "Inbox", "mail-folder-inbox" },
                                 { "Workday", "go-jump-today" },
                                 { "Projects", "folder" },
                                 { "Projects / TestData » Calendar1", "folder" },
                                 { "Projects / TestData » Calendar1 / Prepare talk about TDD", "view-pim-tasks" },
                                 { "Projects / TestData » Calendar1 / Read List", "view-pim-tasks" },
                                 { "Projects / TestData » Calendar1 » Calendar2", "folder" },
                                 { "Projects / TestData » Calendar1 » Calendar2 / Backlog", "view-pim-tasks" },
                                 { "Contexts", "folder" },
                                 { "Contexts / Errands", "view-pim-notes" },
                                 { "Contexts / Internet", "view-pim-notes" },
                                 { "Contexts / Online", "view-pim-notes" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(ContextAddFeature)

#include "contextaddfeature.moc"
