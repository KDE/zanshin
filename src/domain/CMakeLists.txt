# SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org> 
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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
)

add_library(domain STATIC ${domain_SRCS})
target_link_libraries(domain Qt5::Core)
