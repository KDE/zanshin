set(app_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/aboutdata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/maincomponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/configdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resourceconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/collectionconfig.cpp
    PARENT_SCOPE)

set(mainApp_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/debugwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    PARENT_SCOPE)

set(partApp_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/part.cpp
    PARENT_SCOPE)

