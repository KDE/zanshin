/* This file is part of Zanshin

   Copyright 2016 Kevin Ottens <ervin@kde.org>

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

#include <QtTest>
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
    static bool _zanshin_testlib_internal_isGuiEnvironmentForced = _zanshin_testlib_internal_forceGuiEnvironment();
#else
#define _ZANSHIN_TESTLIB_INTERNAL_FORCE_GUI_ENVIRONMENT
#endif

#define ZANSHIN_TEST_MAIN(TestCase) \
    extern Q_CORE_EXPORT QBasicAtomicInt qt_qhash_seed; \
    _ZANSHIN_TESTLIB_INTERNAL_FORCE_GUI_ENVIRONMENT \
    \
    bool _zanshin_testlib_internal_forceEnvironment() \
    { \
        qt_qhash_seed.store(0); \
        qputenv("KDE_DEBUG", "1"); \
        qunsetenv("LANG"); \
        qunsetenv("LANGUAGE"); \
        qputenv("LC_ALL", "en_US"); \
        return true; \
    } \
    static bool _zanshin_testlib_internal_isEnvironmentForced = _zanshin_testlib_internal_forceEnvironment(); \
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
