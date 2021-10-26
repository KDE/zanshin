# SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org> 
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

set(featurelib_SRCS
   zanshincontext.cpp
)

add_definitions(-DZANSHIN_USER_XMLDATA="${CMAKE_SOURCE_DIR}/tests/features/testenv/data/testdata.xml")

include_directories(${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/src)
add_library(featurelib STATIC ${featurelib_SRCS})
target_link_libraries(featurelib
    Qt5::Test
    KF5::ConfigCore
    integration
)

