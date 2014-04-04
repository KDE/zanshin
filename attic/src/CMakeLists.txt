include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(app)
add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(icons)
add_subdirectory(reparentingmodel)
add_subdirectory(runner)
add_subdirectory(utils)

########### zanshin core ###############

set(zanshin_core_SRCS
    todohelpers.cpp
    ${core_SRCS}
    ${gui_SRCS}
    ${reparentingmodel_SRCS}
)

set(zanshin_core_LIBS
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_KCALCORE_LIBS}
    ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTXML_LIBRARY}
    akonadi
    domain
    oldutils
    utils
)

########### zanshin gui ###############

set(zanshin_gui_SRCS
    ${gui_SRCS}
)

kde4_add_ui_files(zanshin_gui_SRCS gui/itemeditor/itemview.ui)
kde4_add_ui_files(zanshin_gui_SRCS gui/itemeditor/properties.ui)

########### zanshin app ###############

set(zanshin_SRCS
    ${app_SRCS}
    ${mainApp_SRCS}
    ${zanshin_core_SRCS}
    ${zanshin_gui_SRCS}
)


kde4_add_app_icon(zanshin_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-apps-zanshin.png")
kde4_add_executable(zanshin ${zanshin_SRCS})
target_link_libraries(zanshin ${KDE4_KDEUI_LIBS} ${zanshin_core_LIBS})

install(TARGETS zanshin DESTINATION ${BIN_INSTALL_DIR})
install(FILES app/zanshinui.rc DESTINATION ${DATA_INSTALL_DIR}/zanshin)
install(FILES gui/itemeditor/editorui.rc DESTINATION ${DATA_INSTALL_DIR}/zanshin)
install(PROGRAMS app/zanshin.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

########### zanshin part ###############

set(zanshin_part_SRCS
    ${app_SRCS}
    ${partApp_SRCS}
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

########### zanshin static version ###############

if(KDE4_BUILD_TESTS)
   kde4_add_library(zanshin_static STATIC ${zanshin_core_SRCS})
   target_link_libraries(zanshin_static ${KDE4_KDEUI_LIBS} ${zanshin_core_LIBS})
   add_dependencies(zanshin_static zanshin)
endif()
