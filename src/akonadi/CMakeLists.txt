# SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org> 
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

set(akonadi_SRCS
    akonadiapplicationselectedattribute.cpp
    akonadicache.cpp
    akonadicachingstorage.cpp
    akonadicollectionfetchjobinterface.cpp
    akonadiconfigdialog.cpp
    akonadicontextqueries.cpp
    akonadicontextrepository.cpp
    akonadidatasourcequeries.cpp
    akonadidatasourcerepository.cpp
    akonadiitemfetchjobinterface.cpp
    akonadilivequeryhelpers.cpp
    akonadilivequeryintegrator.cpp
    akonadimonitorimpl.cpp
    akonadimonitorinterface.cpp
    akonadiprojectqueries.cpp
    akonadiprojectrepository.cpp
    akonadiserializer.cpp
    akonadiserializerinterface.cpp
    akonadistorage.cpp
    akonadistorageinterface.cpp
    akonadistoragesettings.cpp
    akonaditaskqueries.cpp
    akonaditaskrepository.cpp
    akonaditimestampattribute.cpp
)

add_library(akonadi STATIC ${akonadi_SRCS})
target_link_libraries(akonadi
    KF5::AkonadiCalendar
    KF5::AkonadiCore
    KF5::AkonadiWidgets
    KF5::CalendarCore
    KF5::ConfigCore
)
