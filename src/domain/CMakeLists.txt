set(domain_SRCS
    artifact.cpp
    context.cpp
    contextqueries.cpp
    contextrepository.cpp
    datasource.cpp
    datasourcequeries.cpp
    note.cpp
    notequeries.cpp
    noterepository.cpp
    project.cpp
    projectqueries.cpp
    projectrepository.cpp
    queryresult.cpp
    queryresultprovider.cpp
    task.cpp
    taskqueries.cpp
    taskrepository.cpp
    topic.cpp
    topicqueries.cpp
    topicrepository.cpp
)

kde4_add_library(domain STATIC ${domain_SRCS})
target_link_libraries(domain ${QT4_QTCORE_LIBRARY})
