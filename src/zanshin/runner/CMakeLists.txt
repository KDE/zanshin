
add_library(krunner_zanshin MODULE)
target_sources(krunner_zanshin PRIVATE
    zanshinrunner.cpp
    zanshinrunner.h
)
target_link_libraries(krunner_zanshin
    akonadi
    domain
    utils
    KF5::ConfigCore
    KF5::Runner
)

install(TARGETS krunner_zanshin DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES plasma-runner-zanshin.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

