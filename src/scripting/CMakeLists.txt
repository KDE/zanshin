set(scripting_SRCS
    scripthandler.cpp
    taskaction.cpp
)

add_library(scripting STATIC ${scripting_SRCS})
target_link_libraries(scripting
    Qt5::Core
    Qt5::Qml
)

