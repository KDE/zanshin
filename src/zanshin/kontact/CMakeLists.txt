
add_library(zanshin_part MODULE)
target_sources(zanshin_part PRIVATE
    ../app/aboutdata.cpp
    part.cpp
    part.h
    kontact_plugins.qrc
)
kde_enable_exceptions()
target_link_libraries(zanshin_part
    KF6::Parts
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
target_link_libraries(kontact_zanshinplugin KF6::Parts KPim6::KontactInterface)

install(TARGETS kontact_zanshinplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kontact)

