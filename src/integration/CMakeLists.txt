# SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org> 
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

set(integration_SRCS
    dependencies.cpp
)

add_library(integration STATIC ${integration_SRCS})
target_link_libraries(integration
    akonadi
    domain
    presentation
    utils
)
