# SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org> 
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

set(testlib_SRCS
   akonadidebug.cpp
   akonadifakedata.cpp
   akonadifakedataxmlloader.cpp
   akonadifakejobs.cpp
   akonadifakemonitor.cpp
   akonadifakestorage.cpp
   akonadifakestoragebehavior.cpp
   akonadistoragetestbase.cpp
   fakejob.cpp
   gencollection.cpp
   gentodo.cpp
   modeltest.cpp
   monitorspy.cpp
   testhelpers.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/src)
add_library(testlib STATIC ${testlib_SRCS})
target_link_libraries(testlib
    KF5::AkonadiCore
    KF5::AkonadiXml
    KF5::CalendarCore
    Qt5::DBus
    Qt5::Test
)

