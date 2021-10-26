# SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org> 
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

set(utils_SRCS
    compositejob.cpp
    datetime.cpp
    dependencymanager.cpp
    jobhandler.cpp
)

add_library(utils STATIC ${utils_SRCS})
target_link_libraries(utils
    KF5::CoreAddons
)
