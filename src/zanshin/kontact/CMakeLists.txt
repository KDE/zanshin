set(part_SRCS
    ../app/aboutdata.cpp
    ../app/dependencies.cpp
    part.cpp
)

add_library(zanshin_part MODULE ${part_SRCS})
kde_enable_exceptions()
target_link_libraries(zanshin_part
    KF5::Parts
    akonadi
    domain
    presentation
    utils
    widgets
)

install(TARGETS zanshin_part DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES zanshin_part.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

##########################

set(kontact_SRCS
    kontact_plugin.cpp
)

add_library(kontact_zanshinplugin MODULE ${kontact_SRCS})
target_link_libraries(kontact_zanshinplugin KF5::Parts KF5::KontactInterface)

install(TARGETS kontact_zanshinplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES zanshin_plugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kontact)
install(FILES zanshin_part.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/zanshin)

