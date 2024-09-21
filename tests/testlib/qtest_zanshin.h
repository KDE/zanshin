/*
 * SPDX-FileCopyrightText: 2016 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef QTEST_ZANSHIN_H
#define QTEST_ZANSHIN_H

#include <QTest>
#include <QDebug>

// We have our own test macro in order to:
//  * make sure QHash instances are not randomized
//  * avoid DrKonqi in case of crashes
//  * Use a much simpler style during tests to avoid
//    animations or such polluting the environment

#ifdef QT_GUI_LIB
#define _ZANSHIN_TESTLIB_INTERNAL_FORCE_GUI_ENVIRONMENT \
    bool _zanshin_testlib_internal_forceGuiEnvironment() \
    { \
        QGuiApplication::setDesktopSettingsAware(false); \
        qputenv("QT_STYLE_OVERRIDE", "fusion"); \
        return true; \
    } \
    Q_CONSTRUCTOR_FUNCTION(_zanshin_testlib_internal_forceGuiEnvironment)
#else
#define _ZANSHIN_TESTLIB_INTERNAL_FORCE_GUI_ENVIRONMENT
#endif

#define ZANSHIN_TEST_MAIN(TestCase) \
    _ZANSHIN_TESTLIB_INTERNAL_FORCE_GUI_ENVIRONMENT \
    \
    bool _zanshin_testlib_internal_forceEnvironment() \
    { \
        QHashSeed::setDeterministicGlobalSeed(); \
        qputenv("KDE_DEBUG", "1"); \
        qunsetenv("LANG"); \
        qunsetenv("LANGUAGE"); \
        qputenv("LC_ALL", "en_US"); \
        return true; \
    } \
    Q_CONSTRUCTOR_FUNCTION(_zanshin_testlib_internal_forceEnvironment) \
    \
    void _zanshin_testlib_internal_workaround_kxmlgui_startupfunc_leak() \
    { \
        QCoreApplication::processEvents(); \
        QCoreApplication::processEvents(); \
    } \
    \
    extern "C" void Q_CORE_EXPORT qt_startup_hook() \
    { \
        _zanshin_testlib_internal_workaround_kxmlgui_startupfunc_leak(); \
    } \
    \
    QTEST_MAIN(TestCase)

#endif // QTEST_ZANSHIN_H
