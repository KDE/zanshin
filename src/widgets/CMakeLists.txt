add_library(widgets STATIC)

target_sources(widgets PRIVATE
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

    applicationcomponents.h
    availablepagesview.h
    availablesourcesview.h
    datasourcedelegate.h
    editorview.h
    filterwidget.h
    itemdelegate.h
    messagebox.h
    messageboxinterface.h
    nameanddatasourcedialog.h
    nameanddatasourcedialoginterface.h
    pageview.h
    pageviewerrorhandler.h
    runningtaskwidget.h
    quickselectdialog.h
    quickselectdialoginterface.h
)

ki18n_wrap_ui(widgets_SRCS
    editorview.ui
    filterwidget.ui
    nameanddatasourcedialog.ui
)

set_property(TARGET widgets PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(widgets
    Qt::Widgets
    presentation
    zanshinkdepimstatic
    KF5::ConfigCore
    KF5::ItemModels
    KF5::WindowSystem
)
