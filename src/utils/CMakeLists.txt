set(utils_SRCS
    compositejob.cpp
    datetime.cpp
    dependencymanager.cpp
    jobhandler.cpp
)

kde4_add_library(utils STATIC ${utils_SRCS})
target_link_libraries(utils
    ${KDE4_KDECORE_LIBS}
)
