set(app_SRCS
    aboutdata.cpp
    dependencies.cpp
    main.cpp
)

add_executable(renku ${app_SRCS})
kde_enable_exceptions()
target_link_libraries(renku
    akonadi
    domain
    presentation
    scripting
    utils
    widgets
)

install(TARGETS renku DESTINATION ${KDE_INSTALL_BINDIR})
install(PROGRAMS org.kde.renku.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES renkuui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/renku)
install(FILES org.kde.renku.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
