set(krunner_zanshin_SRCS
    zanshinrunner.cpp
    zanshinrunner.h
)

add_library(krunner_zanshin MODULE ${krunner_zanshin_SRCS})
target_link_libraries(krunner_zanshin
    akonadi
    domain
    utils
    KF5::ConfigCore
    KF5::Runner
)

install(TARGETS krunner_zanshin DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES plasma-runner-zanshin.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

