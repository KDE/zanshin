# SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org> 
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

set(krunner_zanshin_SRCS
    zanshinrunner.cpp
)

add_library(krunner_zanshin MODULE ${krunner_zanshin_SRCS})
target_link_libraries(krunner_zanshin
    akonadi
    domain
    utils
    KF5::ConfigCore
    KF5::Runner
)

install(TARGETS krunner_zanshin DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES plasma-runner-zanshin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

