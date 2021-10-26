/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: Default data source
//   As an advanced user
//   I can choose the default data source
//   In order to quickly store tasks
class DatasourceDefaultSettingsFeature : public QObject
{
    Q_OBJECT
private slots:
    void Have_a_default_data_source_for_tasks_in_the_inbox()
    {
        ZanshinContext c;
        Given(c.I_display_the_available_data_sources());
        When(c.I_change_the_setting("defaultCollection", 7));
        Then(c.the_default_data_source_is("TestData / Calendar1 / Calendar2"));
    }

    void Change_the_default_data_source_for_tasks_in_the_inbox()
    {
        ZanshinContext c;
        Given(c.I_display_the_available_data_sources());
        And(c.I_change_the_setting("defaultCollection", 42));
        When(c.I_change_the_default_data_source("TestData / Calendar1 / Calendar2"));
        Then(c.the_setting_is("defaultCollection", 7));
    }
};

ZANSHIN_TEST_MAIN(DatasourceDefaultSettingsFeature)

#include "datasourcedefaultsettingsfeature.moc"
