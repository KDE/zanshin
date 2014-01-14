set(domain_SRCS
    artifact.cpp
    project.cpp
    tag.cpp
    task.cpp
)

kde4_add_library(domain STATIC ${domain_SRCS})
target_link_libraries(domain ${QT4_QTCORE_LIBRARY})
