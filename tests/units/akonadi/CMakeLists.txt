# SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org> 
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

include(MacroAkonadiAutoTests)

zanshin_auto_tests(
  akonadiapplicationselectedattributetest
  akonadicachetest
  akonadicachingstoragetest
  akonadicontextqueriestest
  akonadicontextrepositorytest
  akonadidatasourcequeriestest
  akonadidatasourcerepositorytest
  akonadilivequeryhelperstest
  akonadilivequeryintegratortest
  akonadiprojectqueriestest
  akonadiprojectrepositorytest
  akonadiserializertest
  akonadistoragesettingstest
  akonaditaskqueriestest
  akonaditaskrepositorytest
  akonaditimestampattributetest
)

zanshin_akonadi_auto_tests(
  akonadistoragetest
  akonadicachingstorageintegrationtest
)

target_link_libraries(tests-units-akonadi-akonadistoragetest akonadi testlib)
target_link_libraries(tests-units-akonadi-akonadicachingstorageintegrationtest akonadi testlib)
