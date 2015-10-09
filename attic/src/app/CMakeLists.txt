set(app_SRCS
    ${CMAKE_SOURCE_DIR}/src/zanshin/app/aboutdata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/collectionconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/configdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/maincomponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resourceconfig.cpp
    PARENT_SCOPE
)

set(mainApp_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/debugwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
    PARENT_SCOPE
)

set(partApp_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/part.cpp
    PARENT_SCOPE
)

