set(presentation_SRCS
    applicationmodel.cpp
    artifacteditormodel.cpp
    artifactfilterproxymodel.cpp
    availablepagesmodel.cpp
    availablesourcesmodel.cpp
    datasourcelistmodel.cpp
    inboxpagemodel.cpp
    metatypes.cpp
    pagemodel.cpp
    projectpagemodel.cpp
    querytreemodelbase.cpp
    tasklistmodel.cpp
    contextpagemodel.cpp
)

kde4_add_library(presentation STATIC ${presentation_SRCS})
target_link_libraries(presentation ${QT4_QTCORE_LIBRARY} domain utils)
