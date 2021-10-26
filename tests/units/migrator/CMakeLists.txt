# SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org> 
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

include(MacroAkonadiAutoTests)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../src/zanshin/migrator)

zanshin_auto_tests(
)

zanshin_akonadi_auto_tests(
  zanshin021migrationtest
  zanshincontextitemsmigrationtest
)

target_link_libraries(tests-units-migrator-zanshin021migrationtest
   migrator
   testlib
)
target_link_libraries(tests-units-migrator-zanshincontextitemsmigrationtest
   migrator
   testlib
)
