add_library(utils STATIC)
target_sources(utils PRIVATE
    compositejob.cpp
    datetime.cpp
    dependencymanager.cpp
    jobhandler.cpp

    compositejob.h
    datetime.h
    dependencymanager.h
    jobhandler.h
)

set_property(TARGET utils PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(utils
    KF${QT_MAJOR_VERSION}::CoreAddons
)
