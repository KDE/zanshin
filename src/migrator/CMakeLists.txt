set(migrator_SRCS
    zanshin021migrator.cpp
)

kde4_add_library(migrator STATIC ${migrator_SRCS})
target_link_libraries(migrator_SRCS ${QT4_QTCORE_LIBRARY})

