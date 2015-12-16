set(part_SRCS
    ../app/aboutdata.cpp
    ../app/dependencies.cpp
    part.cpp
)

kde4_add_plugin(zanshin_part ${part_SRCS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
target_link_libraries(zanshin_part
    ${KDE4_KPARTS_LIBS}
    akonadi
    domain
    presentation
    scripting
    utils
    widgets
    ${KDEPIM_STATIC_LIBS}
)

install(TARGETS zanshin_part DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES zanshin_part.desktop DESTINATION ${SERVICES_INSTALL_DIR})

##########################

set(kontact_SRCS
    kontact_plugin.cpp
)

kde4_add_plugin(kontact_zanshinplugin ${kontact_SRCS})
target_link_libraries(kontact_zanshinplugin ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KONTACTINTERFACE_LIBS})

install(TARGETS kontact_zanshinplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES zanshin_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install(FILES zanshin_part.rc DESTINATION ${DATA_INSTALL_DIR}/zanshin)

