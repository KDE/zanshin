set(widgets_SRCS
    applicationcomponents.cpp
    availablepagesview.cpp
    availablesourcesview.cpp
    datasourcedelegate.cpp
    editorview.cpp
    filterwidget.cpp
    itemdelegate.cpp
    messagebox.cpp
    messageboxinterface.cpp
    newprojectdialog.cpp
    newprojectdialoginterface.cpp
    pageview.cpp
    quickselectdialog.cpp
    quickselectdialoginterface.cpp
    scripteditor.cpp
)

qt5_wrap_ui(widgets_SRCS
    editorview.ui
    filterwidget.ui
    newprojectdialog.ui
)

add_library(widgets STATIC ${widgets_SRCS})
target_link_libraries(widgets
    Qt5::Qml
    Qt5::Widgets
    presentation
    zanshinkdepimstatic
)
