set(presentation_SRCS
    datasourcelistmodel.cpp
    inboxmodel.cpp
    querytreemodelbase.cpp
    tasklistmodel.cpp
)

kde4_add_library(presentation STATIC ${presentation_SRCS})
target_link_libraries(presentation ${QT4_QTCORE_LIBRARY})
