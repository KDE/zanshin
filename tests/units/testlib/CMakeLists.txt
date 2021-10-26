# SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org> 
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

add_definitions(-DSOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")

zanshin_auto_tests(
  akonadifakedatatest
  akonadifakedataxmlloadertest
  akonadifakestoragetest
  gencollectiontest
  gentodotest
  monitorspytest
)
