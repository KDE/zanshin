
add_library(zanshin_part MODULE)
target_sources(zanshin_part PRIVATE
    ../app/aboutdata.cpp
    part.cpp
    part.h
)
kde_enable_exceptions()
target_link_libraries(zanshin_part
    KF5::Parts
    integration
    widgets
)

install(TARGETS zanshin_part DESTINATION ${KDE_INSTALL_PLUGINDIR})

##########################


add_library(kontact_zanshinplugin MODULE)
target_sources(kontact_zanshinplugin PRIVATE
    kontact_plugin.cpp
    kontact_plugin.h
)
target_link_libraries(kontact_zanshinplugin KF5::Parts KF5::KontactInterface)

install(TARGETS kontact_zanshinplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim${QT_MAJOR_VERSION}/kontact)
install(FILES zanshin_part.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/zanshin)

