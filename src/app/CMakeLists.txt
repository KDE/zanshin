set(app_SRCS
    aboutdata.cpp
    main.cpp
)

kde4_add_executable(zanshin-next ${app_SRCS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
target_link_libraries(zanshin-next
    ${KDE4_KDEUI_LIBS}
    akonadi
    domain
    presentation
    utils
    widgets
)

install(TARGETS zanshin-next DESTINATION ${BIN_INSTALL_DIR})
install(PROGRAMS zanshin-next.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

