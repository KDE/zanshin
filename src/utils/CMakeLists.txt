set(utils_SRCS
    compositejob.cpp
    datetime.cpp
    dependencymanager.cpp
    jobhandler.cpp

    compositejob.h
    datetime.h
    dependencymanager.h
    jobhandler.h
)

add_library(utils STATIC ${utils_SRCS})
set_property(TARGET utils PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(utils
    KF5::CoreAddons
)
