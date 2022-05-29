set(presentation_SRCS
    applicationmodel.cpp
    editormodel.cpp
    availablepagesmodel.cpp
    availablepagessortfilterproxymodel.cpp
    availablesourcesmodel.cpp
    contextpagemodel.cpp
    errorhandler.cpp
    errorhandlingmodelbase.cpp
    inboxpagemodel.cpp
    metatypes.cpp
    pagemodel.cpp
    projectpagemodel.cpp
    alltaskspagemodel.cpp
    querytreemodelbase.cpp
    runningtaskmodelinterface.cpp
    runningtaskmodel.cpp
    taskfilterproxymodel.cpp
    workdaypagemodel.cpp
)

add_library(presentation STATIC ${presentation_SRCS})
target_link_libraries(presentation Qt::Core Qt::Gui KF5::I18n domain utils)
