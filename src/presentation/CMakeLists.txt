set(presentation_SRCS
    applicationmodel.cpp
    artifacteditormodel.cpp
    artifactfilterproxymodel.cpp
    availablenotepagesmodel.cpp
    availablepagesmodelinterface.cpp
    availablepagessortfilterproxymodel.cpp
    availabletaskpagesmodel.cpp
    availablesourcesmodel.cpp
    contextpagemodel.cpp
    errorhandler.cpp
    errorhandlingmodelbase.cpp
    metatypes.cpp
    noteinboxpagemodel.cpp
    pagemodel.cpp
    projectpagemodel.cpp
    querytreemodelbase.cpp
    tagpagemodel.cpp
    taskinboxpagemodel.cpp
    tasklistmodel.cpp
    workdaypagemodel.cpp
)

kde4_add_library(presentation STATIC ${presentation_SRCS})
target_link_libraries(presentation ${QT4_QTCORE_LIBRARY} domain utils)
