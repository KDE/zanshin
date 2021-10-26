/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Data sources listing
//   As an advanced user
//   I can list sources
//   In order to list and store tasks
class DatasourceListingFeature : public QObject
{
    Q_OBJECT
private slots:
    void All_task_sources_appear_in_the_list()
    {
        ZanshinContext c;
        Given(c.I_display_the_available_data_sources());
        When(c.I_list_the_items());
        Then(c.the_list_is({
                             { "display", "icon" },
                             {
                                 { "TestData", "folder" },
                                 { "TestData / Calendar1", "view-calendar-tasks" },
                                 { "TestData / Calendar1 / Calendar2", "view-calendar-tasks" },
                                 { "TestData / Calendar1 / Calendar2 / Calendar3", "folder" },
                             }
                         }));
    }
};

ZANSHIN_TEST_MAIN(DatasourceListingFeature)

#include "datasourcelistingfeature.moc"
