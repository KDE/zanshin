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

// Feature: Adding tasks
//   As a task junkie
//   I can create task by giving a title
//   In order to collect ideas while reflecting on my life
class AddingTaskFeature : public QObject
{
    Q_OBJECT
private slots:
    void Adding_a_task_in_a_page_data()
    {
        QTest::addColumn<QString>("page");
        QTest::addColumn<QString>("title");

        QTest::newRow("inbox") << "Inbox" << "Buy a book";
        QTest::newRow("release") << "Projects / TestData » Calendar1 » Calendar2 / Backlog" << "Start a release party";
    }

    void Adding_a_task_in_a_page()
    {
        QFETCH(QString, page);
        QFETCH(QString, title);

        ZANSHIN_CONTEXT;
        Given(I_display_the_page(page));
        And(I_look_at_the_central_list());
        When(I_add_a_task(title));
        And(I_list_the_items());
        Then(the_list_contains(title));
    }

    void Adding_a_task_as_a_child_of_another_task_in_a_page_data()
    {
        QTest::addColumn<QString>("page");
        QTest::addColumn<QString>("parent");
        QTest::addColumn<QString>("title");

        QTest::newRow("inbox") << "Inbox" << "Buy a book" << "Make sure it is a good book";
        QTest::newRow("release") << "Projects / TestData » Calendar1 » Calendar2 / Backlog" << "Start a release party" << "Make sure there was a release";
    }

    void Adding_a_task_as_a_child_of_another_task_in_a_page()
    {
        QFETCH(QString, page);
        QFETCH(QString, parent);
        QFETCH(QString, title);

        ZANSHIN_CONTEXT;
        Given(I_display_the_page(page));
        And(I_add_a_task(parent));
        And(I_look_at_the_central_list());
        And(I_list_the_items());
        When(I_add_a_task_child(title, parent));
        And(I_list_the_items());
        Then(the_list_contains(parent + " / " + title));
    }
};

ZANSHIN_TEST_MAIN(AddingTaskFeature)

#include "addingtaskfeature.moc"
