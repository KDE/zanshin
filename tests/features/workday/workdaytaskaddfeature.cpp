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

// Feature: Adding a task from the workday view
//   As someone adding tasks
//   I can input a task for today in the quick entry
//   In order to have a new task that starts today
class WorkdayTaskAddFeature : public QObject
{
    Q_OBJECT
private slots:
    void Tasks_added_from_the_workday_view_start_today()
    {
        ZanshinContext c;
        Given(c.I_display_the_page("Workday"));
        When(c.I_look_at_the_central_list());
        And(c.I_add_a_task("Burn some confidential documents"));
        And(c.I_list_the_items());
        Then(c.the_list_contains("Burn some confidential documents"));
    }
};

ZANSHIN_TEST_MAIN(WorkdayTaskAddFeature)

#include "workdaytaskaddfeature.moc"
