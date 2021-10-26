/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include "testhelpers.h"

#include <QTest>

#include "utils/jobhandler.h"

using namespace Testlib;

void TestHelpers::waitForEmptyJobQueue()
{
    while (Utils::JobHandler::jobCount() != 0) {
        QTest::qWait(20);
    }
}
