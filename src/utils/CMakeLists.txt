set(utils_SRCS
    compositejob.cpp
    datetime.cpp
    dependencymanager.cpp
    jobhandler.cpp
)

add_library(utils STATIC ${utils_SRCS})
target_link_libraries(utils
    KF5::CoreAddons
)
