/*
 * SPDX-FileCopyrightText: 2019 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <testlib/qtest_zanshin.h>
#include <featurelib/zanshincontext.h>

using namespace Testlib;

// Feature: adding a task with start today button
//   As someone using tasks
//   I can set the date of a task to today by clicking on the "start today" button
//   In order to have my task start today

class WorkdayStartbuttonFeature : public QObject
{
    Q_OBJECT
};

ZANSHIN_TEST_MAIN(WorkdayStartbuttonFeature)

#include "workdaystartbuttonfeature.moc"
