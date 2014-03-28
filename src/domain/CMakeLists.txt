set(domain_SRCS
    artifact.cpp
    artifactqueries.cpp
    artifactrepository.cpp
    project.cpp
    projectqueries.cpp
    projectrepository.cpp
    queryresult.cpp
    queryresultprovider.cpp
    tag.cpp
    tagqueries.cpp
    tagrepository.cpp
    task.cpp
    taskqueries.cpp
    taskrepository.cpp
)

kde4_add_library(domain STATIC ${domain_SRCS})
target_link_libraries(domain ${QT4_QTCORE_LIBRARY})
