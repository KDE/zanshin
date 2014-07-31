set(widgets_SRCS
    datasourcecombobox.cpp
    itemdelegate.cpp
    pageview.cpp
)

kde4_add_library(widgets STATIC ${widgets_SRCS})
target_link_libraries(widgets ${QT4_QTGUI_LIBRARY} presentation)
