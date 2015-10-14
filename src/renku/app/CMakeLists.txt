set(app_SRCS
    aboutdata.cpp
    dependencies.cpp
    main.cpp
)

kde4_add_executable(renku ${app_SRCS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
target_link_libraries(renku
    ${KDE4_KDEUI_LIBS}
    akonadi
    domain
    presentation
    scripting
    utils
    widgets
    ${KDEPIM_STATIC_LIBS}
)

install(TARGETS renku DESTINATION ${BIN_INSTALL_DIR})
install(PROGRAMS renku.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES renku.appdata.xml DESTINATION share/appdata)
