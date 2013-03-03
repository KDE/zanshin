include_directories(. utils pimlistview pimlistview/abstractmodels itemview reparentingmodel)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/utils)

add_subdirectory( reparentingmodel )
add_subdirectory( core )
add_subdirectory( icons )
add_subdirectory( runner )
add_subdirectory( utils )
add_subdirectory( pimlistview )
add_subdirectory( itemview )

########### zanshin core ###############

set(zanshin_core_SRCS
   combomodel.cpp
   selectionproxymodel.cpp
   sidebarmodel.cpp
   todohelpers.cpp
   ${core_SRCS}
   ${reparentingmodel_SRCS}
   ${pimlistview_SRCS}
   ${itemview_SRCS}
)

set(zanshin_core_LIBS
   ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS} utils ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS} nepomuk nepomukutils kio soprano ${NEPOMUK_CORE_LIBRARY}
)

if(KDE4_BUILD_TESTS)
   kde4_add_library(zanshin_static STATIC ${zanshin_core_SRCS})
   target_link_libraries(zanshin_static ${KDE4_KDEUI_LIBS} ${zanshin_core_LIBS})
endif(KDE4_BUILD_TESTS)

########### zanshin gui ###############

set(zanshin_gui_SRCS
   aboutdata.cpp
   actionduedatedelegate.cpp
   actionlistcombobox.cpp
   actionlistdelegate.cpp
   actionlisteditor.cpp
   actionlisteditorpage.cpp
   quickselectdialog.cpp
   sidebar.cpp
   sidebarpage.cpp
   todotreeview.cpp
)

kde4_add_ui_files(zanshin_gui_SRCS itemview/itemview.ui)
kde4_add_ui_files(zanshin_gui_SRCS itemview/tags.ui)
kde4_add_ui_files(zanshin_gui_SRCS itemview/properties.ui)

########### zanshin app ###############

set(zanshin_SRCS
   app/configdialog.cpp
   app/debugwindow.cpp
   app/main.cpp
   app/maincomponent.cpp
   app/mainwindow.cpp
   ${zanshin_core_SRCS}
   ${zanshin_gui_SRCS}
)


kde4_add_app_icon(zanshin_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-apps-zanshin.png")
kde4_add_executable(zanshin ${zanshin_SRCS})
target_link_libraries(zanshin ${KDE4_KDEUI_LIBS} ${zanshin_core_LIBS})

install(TARGETS zanshin DESTINATION ${BIN_INSTALL_DIR})
install(FILES app/zanshinui.rc DESTINATION ${DATA_INSTALL_DIR}/zanshin)
install(FILES contextviewui.rc  DESTINATION  ${DATA_INSTALL_DIR}/zanshin )
install(FILES editorui.rc  DESTINATION  ${DATA_INSTALL_DIR}/zanshin )
install(PROGRAMS app/zanshin.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

########### zanshin part ###############

set(zanshin_part_SRCS
   app/configdialog.cpp
   app/maincomponent.cpp
   app/part.cpp
   ${zanshin_core_SRCS}
   ${zanshin_gui_SRCS}
)

kde4_add_plugin(zanshin_part ${zanshin_part_SRCS})
target_link_libraries(zanshin_part ${KDE4_KPARTS_LIBS} ${zanshin_core_LIBS})

install(TARGETS zanshin_part DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES app/zanshin_part.rc DESTINATION ${DATA_INSTALL_DIR}/zanshin)
install(FILES app/zanshin_part.desktop DESTINATION ${SERVICES_INSTALL_DIR})

########### kontact zanshin plugin ###############

set(kontact_zanshinplugin_SRCS
   app/kontact_plugin.cpp
)

kde4_add_plugin(kontact_zanshinplugin ${kontact_zanshinplugin_SRCS})
target_link_libraries(kontact_zanshinplugin ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KONTACTINTERFACE_LIBS})

install(TARGETS kontact_zanshinplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES app/zanshin_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
