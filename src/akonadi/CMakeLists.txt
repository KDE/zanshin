set(akonadi_SRCS
    akonadicollectionfetchjobinterface.cpp
    akonadiitemfetchjobinterface.cpp
    akonadimonitorimpl.cpp
    akonadimonitorinterface.cpp
    akonadiserializer.cpp
    akonadiserializerinterface.cpp
    akonadistorage.cpp
    akonadistorageinterface.cpp
    akonadistoragesettings.cpp
    akonaditaskqueries.cpp
    akonaditaskrepository.cpp
)

kde4_add_library(akonadi STATIC ${akonadi_SRCS})
target_link_libraries(akonadi ${QT4_QTCORE_LIBRARY})
