set(app_SRCS
    aboutdata.cpp
    dependencies.cpp
    main.cpp
)

kde4_add_executable(zanshin ${app_SRCS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
target_link_libraries(zanshin
    ${KDE4_KDEUI_LIBS}
    akonadi
    domain
    presentation
    scripting
    utils
    widgets
    ${KDEPIM_STATIC_LIBS}
)

install(TARGETS zanshin DESTINATION ${BIN_INSTALL_DIR})
install(PROGRAMS zanshin.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES zanshinui.rc DESTINATION ${DATA_INSTALL_DIR}/zanshin)
install(FILES zanshin.appdata.xml DESTINATION share/appdata)
