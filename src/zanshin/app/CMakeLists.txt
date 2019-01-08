set(app_SRCS
    aboutdata.cpp
    dependencies.cpp
    main.cpp
)
ecm_create_qm_loader(app_SRCS zanshin_qt)
qt5_add_resources(app_SRCS zanshin.qrc)

add_executable(zanshin ${app_SRCS})
kde_enable_exceptions()
target_link_libraries(zanshin
    akonadi
    domain
    presentation
    utils
    widgets
)

install(TARGETS zanshin DESTINATION ${KDE_INSTALL_BINDIR})
install(PROGRAMS org.kde.zanshin.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.zanshin.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
