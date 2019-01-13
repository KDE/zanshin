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

// Feature: Removing tasks
//   As a task junkie
//   I can delete a task so it is removed
//   In order to clean up the old junk I accumulated
class RemovingTaskFeature : public QObject
{
    Q_OBJECT
private slots:
    void Removing_a_simple_task_from_a_page_data()
    {
        QTest::addColumn<QString>("page");
        QTest::addColumn<QString>("title");

        QTest::newRow("inbox") << "Inbox" << "Buy cheese";
        QTest::newRow("readlist") << "Projects / TestData » Calendar1 / Read List" << "\"Domain Driven Design\" by Eric Evans";
    }

    void Removing_a_simple_task_from_a_page()
    {
        QFETCH(QString, page);
        QFETCH(QString, title);

        ZANSHIN_CONTEXT;
        Given(I_display_the_page(page));
        And(there_is_an_item_in_the_central_list(title));
        When(I_remove_the_item());
        And(I_list_the_items());
        Then(the_list_does_not_contain(title));
    }
};

ZANSHIN_TEST_MAIN(RemovingTaskFeature)

#include "removingtaskfeature.moc"
