set(presentation_SRCS
    applicationmodel.cpp
    artifacteditormodel.cpp
    artifactfilterproxymodel.cpp
    availablepagesmodelinterface.cpp
    availablepagessortfilterproxymodel.cpp
    availabletaskpagesmodel.cpp
    availablesourcesmodel.cpp
    contextpagemodel.cpp
    datasourcelistmodel.cpp
    errorhandler.cpp
    errorhandlingmodelbase.cpp
    inboxpagemodel.cpp
    metatypes.cpp
    noteinboxpagemodel.cpp
    pagemodel.cpp
    projectpagemodel.cpp
    querytreemodelbase.cpp
    tagpagemodel.cpp
    taskapplicationmodel.cpp
    tasklistmodel.cpp
    workdaypagemodel.cpp
)

kde4_add_library(presentation STATIC ${presentation_SRCS})
target_link_libraries(presentation ${QT4_QTCORE_LIBRARY} domain utils)
