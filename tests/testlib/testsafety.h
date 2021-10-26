/*
 * SPDX-FileCopyrightText: 2014 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef TESTSAFETY_H
#define TESTSAFETY_H

#include <QTest>
#include <QDebug>

namespace TestLib {
    namespace TestSafety {

        /**
         * Checks that the test is running in the proper test environment (akonaditest)
         */
        bool checkTestIsIsolated()
        {
            if (qEnvironmentVariableIsEmpty("AKONADI_TESTRUNNER_PID")) {
                qCritical() << "This test must be run using ctest, in order to use the testrunner environment. Aborting, to avoid messing up your real akonadi";
                return false;
            }
            return true;
        }

    }
}

#endif // TESTSAFETY_H
