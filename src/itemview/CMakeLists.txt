
set(itemview_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/itemcontext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itemviewer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editablewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nepomukcontextmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nepomukcontextview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editorwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/toolbox.cpp
    PARENT_SCOPE)
#kde4_add_ui_files(itemview_SRCS itemview.ui)