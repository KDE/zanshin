add_subdirectory( runner )

########### zanshin ###############

set(zanshin_core_SRCS
   actionlistcheckablemodel.cpp
   actionlistcompletermodel.cpp
   categorymanager.cpp
   combomodel.cpp
   modelstack.cpp
   selectionproxymodel.cpp
   sidebarmodel.cpp
   todocategoriesmodel.cpp
   todohelpers.cpp
   todoproxymodelbase.cpp
   todotreemodel.cpp
   todometadatamodel.cpp
   todomodel.cpp
   todonode.cpp
   todonodemanager.cpp
)

set(zanshin_gui_SRCS
   aboutdata.cpp
   actionduedatedelegate.cpp
   actionlistcombobox.cpp
   actionlistcompleterview.cpp
   actionlistdelegate.cpp
   actionlisteditor.cpp
   actionlisteditorpage.cpp
   configdialog.cpp
   kdateedit.cpp
   kdatepickerpopup.cpp
   maincomponent.cpp
   quickselectdialog.cpp
   sidebar.cpp
   sidebarpage.cpp
   todotreeview.cpp
)

set(zanshin_SRCS
   debugwindow.cpp
   main.cpp
   mainwindow.cpp
   ${zanshin_core_SRCS}
   ${zanshin_gui_SRCS}
)

kde4_add_executable(zanshin ${zanshin_SRCS})
target_link_libraries(zanshin ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS})

if(KDE4_BUILD_TESTS)
kde4_add_library(zanshin_static STATIC ${zanshin_core_SRCS})
target_link_libraries(zanshin_static ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS})
endif(KDE4_BUILD_TESTS)

########### install files ###############

install(TARGETS zanshin DESTINATION ${BIN_INSTALL_DIR})
install(FILES zanshinui.rc DESTINATION ${DATA_INSTALL_DIR}/zanshin)
install(FILES zanshin.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

