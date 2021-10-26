/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

        ZanshinContext c;
        Given(c.I_display_the_page(page));
        And(c.there_is_an_item_in_the_central_list(title));
        When(c.I_remove_the_item());
        And(c.I_list_the_items());
        Then(c.the_list_does_not_contain(title));
    }
};

ZANSHIN_TEST_MAIN(RemovingTaskFeature)

#include "removingtaskfeature.moc"
