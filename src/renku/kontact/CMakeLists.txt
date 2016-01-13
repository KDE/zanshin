set(part_SRCS
    ../app/aboutdata.cpp
    ../app/dependencies.cpp
    part.cpp
)

add_library(renku_part MODULE ${part_SRCS})
kde_enable_exceptions()
target_link_libraries(renku_part
    KF5::Parts
    akonadi
    domain
    presentation
    scripting
    utils
    widgets
)

install(TARGETS renku_part DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES renku_part.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

##########################

set(kontact_SRCS
    kontact_plugin.cpp
)

add_library(kontact_renkuplugin MODULE ${kontact_SRCS})
target_link_libraries(kontact_renkuplugin KF5::Parts KF5::KontactInterface)

install(TARGETS kontact_renkuplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES renku_plugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kontact)
install(FILES renku_part.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/renku)

