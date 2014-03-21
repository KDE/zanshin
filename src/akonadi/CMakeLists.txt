set(akonadi_SRCS
    akonadicollectionfetchjobinterface.cpp
    akonadiitemfetchjobinterface.cpp
    akonadimonitorinterface.cpp
    akonadimonitorimpl.cpp
    akonadiserializerinterface.cpp
    akonadiserializer.cpp
    akonadistorageinterface.cpp
    akonadistorage.cpp
    akonaditaskqueries.cpp
)

kde4_add_library(akonadi STATIC ${akonadi_SRCS})
target_link_libraries(akonadi ${QT4_QTCORE_LIBRARY})
