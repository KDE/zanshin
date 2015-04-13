set(scripting_SRCS
    scripthandler.cpp
    taskaction.cpp
)

kde4_add_library(scripting STATIC ${scripting_SRCS})
target_link_libraries(scripting
    ${KDE4_KDECORE_LIBS} ${QT_QTSCRIPT_LIBRARY}
)

