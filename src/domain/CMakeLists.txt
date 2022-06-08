set(domain_SRCS
    context.cpp
    contextqueries.cpp
    contextrepository.cpp
    datasource.cpp
    datasourcequeries.cpp
    datasourcerepository.cpp
    project.cpp
    projectqueries.cpp
    projectrepository.cpp
    queryresult.cpp
    queryresultinterface.cpp
    queryresultprovider.cpp
    task.cpp
    taskqueries.cpp
    taskrepository.cpp

    context.h
    contextqueries.h
    contextrepository.h
    datasource.h
    datasourcequeries.h
    datasourcerepository.h
    project.h
    projectqueries.h
    projectrepository.h
    queryresult.h
    queryresultinterface.h
    queryresultprovider.h
    task.h
    taskqueries.h
    taskrepository.h
)

add_library(domain STATIC ${domain_SRCS})
set_property(TARGET domain PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(domain Qt::Core)
