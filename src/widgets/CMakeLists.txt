set(widgets_SRCS
    applicationcomponents.cpp
    datasourcecombobox.cpp
    editorview.cpp
    itemdelegate.cpp
    pageview.cpp
)

kde4_add_library(widgets STATIC ${widgets_SRCS})
target_link_libraries(widgets ${QT4_QTGUI_LIBRARY} pimlibs presentation)
