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
