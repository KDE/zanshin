/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

        ZanshinContext c;
        Given(c.I_display_the_page(page));
        And(c.I_look_at_the_central_list());
        When(c.I_add_a_task(title));
        And(c.I_list_the_items());
        Then(c.the_list_contains(title));
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

        ZanshinContext c;
        Given(c.I_display_the_page(page));
        And(c.I_add_a_task(parent));
        And(c.I_look_at_the_central_list());
        And(c.I_list_the_items());
        When(c.I_add_a_task_child(title, parent));
        And(c.I_list_the_items());
        Then(c.the_list_contains(parent + " / " + title));
    }
};

ZANSHIN_TEST_MAIN(AddingTaskFeature)

#include "addingtaskfeature.moc"
