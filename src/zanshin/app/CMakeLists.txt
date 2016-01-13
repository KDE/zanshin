set(app_SRCS
    aboutdata.cpp
    dependencies.cpp
    main.cpp
)

add_executable(zanshin ${app_SRCS})
kde_enable_exceptions()
target_link_libraries(zanshin
    akonadi
    domain
    presentation
    scripting
    utils
    widgets
)

install(TARGETS zanshin DESTINATION ${KDE_INSTALL_BINDIR})
install(PROGRAMS org.kde.zanshin.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES zanshinui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/zanshin)
install(FILES zanshin.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
