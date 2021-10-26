# SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org> 
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

zanshin_auto_tests(
  applicationcomponentstest
  availablepagesviewtest
  availablesourcesviewtest
  editorviewtest
  filterwidgettest
  nameanddatasourcedialogtest
  pageviewerrorhandlertest
  pageviewtest
  quickselectdialogtest
  runningtaskwidgettest
)

# These tests need a window that takes focus
set_tests_properties(tests-units-widgets-editorviewtest PROPERTIES RUN_SERIAL TRUE)
set_tests_properties(tests-units-widgets-pageviewtest PROPERTIES RUN_SERIAL TRUE)
