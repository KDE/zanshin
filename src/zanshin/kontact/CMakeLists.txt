set(part_SRCS
    ../app/aboutdata.cpp
    part.cpp
)

add_library(zanshin_part MODULE ${part_SRCS})
kde_enable_exceptions()
target_link_libraries(zanshin_part
    KF5::Parts
    integration
    widgets
)

install(TARGETS zanshin_part DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES zanshin_part.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

##########################

set(kontact_SRCS
    kontact_plugin.cpp
    kontact_plugin.h
)

add_library(kontact_zanshinplugin MODULE ${kontact_SRCS})
target_link_libraries(kontact_zanshinplugin KF5::Parts KF5::KontactInterface)
kcoreaddons_desktop_to_json(kontact_zanshinplugin zanshin_plugin.desktop)

install(TARGETS kontact_zanshinplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/kontact5)
install(FILES zanshin_plugin.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/kontact)
install(FILES zanshin_part.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/zanshin)

