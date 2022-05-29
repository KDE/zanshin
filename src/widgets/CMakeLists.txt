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
    nameanddatasourcedialog.cpp
    nameanddatasourcedialoginterface.cpp
    pageview.cpp
    pageviewerrorhandler.cpp
    runningtaskwidget.cpp
    quickselectdialog.cpp
    quickselectdialoginterface.cpp
)

ki18n_wrap_ui(widgets_SRCS
    editorview.ui
    filterwidget.ui
    nameanddatasourcedialog.ui
)

add_library(widgets STATIC ${widgets_SRCS})
target_link_libraries(widgets
    Qt::Widgets
    presentation
    zanshinkdepimstatic
    KF5::ConfigCore
    KF5::ItemModels
    KF5::WindowSystem
)
