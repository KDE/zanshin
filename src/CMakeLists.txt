add_subdirectory( runner )

########### zanshin core ###############

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

if(KDE4_BUILD_TESTS)
   kde4_add_library(zanshin_static STATIC ${zanshin_core_SRCS})
   target_link_libraries(zanshin_static ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS})
endif(KDE4_BUILD_TESTS)

########### zanshin gui ###############

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

########### zanshin app ###############

set(zanshin_SRCS
   debugwindow.cpp
   main.cpp
   mainwindow.cpp
   ${zanshin_core_SRCS}
   ${zanshin_gui_SRCS}
)

kde4_add_executable(zanshin ${zanshin_SRCS})
target_link_libraries(zanshin ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS})

install(TARGETS zanshin DESTINATION ${BIN_INSTALL_DIR})
install(FILES zanshinui.rc DESTINATION ${DATA_INSTALL_DIR}/zanshin)
install(FILES zanshin.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

########### zanshin part ###############

set(zanshin_part_SRCS
   part.cpp
   ${zanshin_core_SRCS}
   ${zanshin_gui_SRCS}
)

kde4_add_plugin(zanshin_part ${zanshin_part_SRCS})
target_link_libraries(zanshin_part ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS})

install(TARGETS zanshin_part DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES zanshin_part.rc DESTINATION ${DATA_INSTALL_DIR}/zanshin)
install(FILES zanshin_part.desktop DESTINATION ${SERVICES_INSTALL_DIR})

########### kontact zanshin plugin ###############

set(kontact_zanshinplugin_SRCS
   kontact_plugin.cpp
)

kde4_add_plugin(kontact_zanshinplugin ${kontact_zanshinplugin_SRCS})
target_link_libraries(kontact_zanshinplugin ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KONTACTINTERFACE_LIBS})

install(TARGETS kontact_zanshinplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES zanshin_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

