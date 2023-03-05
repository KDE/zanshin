
add_library(akonadi STATIC)
target_sources(akonadi PRIVATE
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

    akonadiapplicationselectedattribute.h
    akonadicache.h
    akonadicachingstorage.h
    akonadicollectionfetchjobinterface.h
    akonadiconfigdialog.h
    akonadicontextqueries.h
    akonadicontextrepository.h
    akonadidatasourcequeries.h
    akonadidatasourcerepository.h
    akonadiitemfetchjobinterface.h
    akonadilivequeryhelpers.h
    akonadilivequeryintegrator.h
    akonadimonitorimpl.h
    akonadimonitorinterface.h
    akonadiprojectqueries.h
    akonadiprojectrepository.h
    akonadiserializer.h
    akonadiserializerinterface.h
    akonadistorage.h
    akonadistorageinterface.h
    akonadistoragesettings.h
    akonaditaskqueries.h
    akonaditaskrepository.h
    akonaditimestampattribute.h
)
set_property(TARGET akonadi PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(akonadi
    KPim${QT_MAJOR_VERSION}::AkonadiCalendar
    KPim${QT_MAJOR_VERSION}::AkonadiCore
    KPim${QT_MAJOR_VERSION}::AkonadiWidgets
    KF${QT_MAJOR_VERSION}::CalendarCore
    KF${QT_MAJOR_VERSION}::ConfigCore
)
