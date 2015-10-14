set(part_SRCS
    ../app/aboutdata.cpp
    ../app/dependencies.cpp
    part.cpp
)

kde4_add_plugin(renku_part ${part_SRCS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
target_link_libraries(renku_part
    ${KDE4_KPARTS_LIBS}
    akonadi
    domain
    presentation
    scripting
    utils
    widgets
    ${KDEPIM_STATIC_LIBS}
)

install(TARGETS renku_part DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES renku_part.desktop DESTINATION ${SERVICES_INSTALL_DIR})

##########################

set(kontact_SRCS
    kontact_plugin.cpp
)

kde4_add_plugin(kontact_renkuplugin ${kontact_SRCS})
target_link_libraries(kontact_renkuplugin ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KONTACTINTERFACE_LIBS})

install(TARGETS kontact_renkuplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES renku_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install(FILES renku_part.rc DESTINATION ${DATA_INSTALL_DIR}/renku)

