# SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org> 
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

set(app_SRCS
    aboutdata.cpp
    main.cpp
)
ecm_create_qm_loader(app_SRCS zanshin_qt)
qt5_add_resources(app_SRCS zanshin.qrc)

add_executable(zanshin ${app_SRCS})
kde_enable_exceptions()
target_link_libraries(zanshin
    KF5::XmlGui
    integration
    widgets
)

install(TARGETS zanshin DESTINATION ${KDE_INSTALL_BINDIR})
install(PROGRAMS org.kde.zanshin.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.zanshin.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
