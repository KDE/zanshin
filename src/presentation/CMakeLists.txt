set(presentation_SRCS
    applicationmodel.cpp
    artifacteditormodel.cpp
    artifactfilterproxymodel.cpp
    availablepagesmodelinterface.cpp
    availablepagessortfilterproxymodel.cpp
    availabletaskpagesmodel.cpp
    availablesourcesmodel.cpp
    contextpagemodel.cpp
    errorhandler.cpp
    errorhandlingmodelbase.cpp
    metatypes.cpp
    pagemodel.cpp
    projectpagemodel.cpp
    querytreemodelbase.cpp
    runningtaskmodelinterface.cpp
    runningtaskmodel.cpp
    taskinboxpagemodel.cpp
    tasklistmodel.cpp
    taskapplicationmodel.cpp
    workdaypagemodel.cpp
)

add_library(presentation STATIC ${presentation_SRCS})
target_link_libraries(presentation Qt5::Core Qt5::Gui KF5::I18n domain utils)
