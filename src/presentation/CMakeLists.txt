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

    applicationmodel.h
    editormodel.h
    availablepagesmodel.h
    availablepagessortfilterproxymodel.h
    availablesourcesmodel.h
    contextpagemodel.h
    errorhandler.h
    errorhandlingmodelbase.h
    inboxpagemodel.h
    metatypes.h
    pagemodel.h
    projectpagemodel.h
    alltaskspagemodel.h
    querytreemodelbase.h
    runningtaskmodelinterface.h
    runningtaskmodel.h
    taskfilterproxymodel.h
    workdaypagemodel.h
)

add_library(presentation STATIC ${presentation_SRCS})
set_property(TARGET presentation PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(presentation Qt::Core Qt::Gui KF5::I18n domain utils)
