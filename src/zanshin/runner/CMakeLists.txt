set(krunner_zanshin_SRCS
    zanshinrunner.cpp
)

kde4_add_plugin(krunner_zanshin ${krunner_zanshin_SRCS})
target_link_libraries(krunner_zanshin
    ${KDE4_PLASMA_LIBS}
    akonadi
    domain
    utils
)

install(TARGETS krunner_zanshin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-zanshin.desktop DESTINATION ${SERVICES_INSTALL_DIR})

