add_subdirectory(tests)

########### zanshin ###############

set(zanshin_SRCS
   actionduedatedelegate.cpp
   actionlistdelegate.cpp
   actionlisteditor.cpp
   actionlistmodel.cpp
   actionlistview.cpp
   configdialog.cpp
   contextsmodel.cpp
   globalmodel.cpp
   kdateedit.cpp
   kdatepickerpopup.cpp
   librarymodel.cpp
   main.cpp
   mainwindow.cpp
   projectsmodel.cpp
   quickselectdialog.cpp
   todocategoriesattribute.cpp
   todocategoriesmodel.cpp
   todoflatmodel.cpp
   todotreemodel.cpp
   sidebar.cpp
)

kde4_add_kcfg_files(zanshin_SRCS globalsettings.kcfgc)

kde4_add_executable(zanshin ${zanshin_SRCS})

target_link_libraries(zanshin ${KDE4_KDEUI_LIBS} ${KDE4_AKONADI_LIBS} ${KDE4_KCAL_LIBS})

########### install files ###############

install(TARGETS zanshin DESTINATION ${BIN_INSTALL_DIR})
install(FILES zanshinui.rc DESTINATION ${DATA_INSTALL_DIR}/zanshin)
install(FILES zanshin.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

