set(akonadi_SRCS
    akonadicollectionfetchjobinterface.cpp
    akonadidatasourcequeries.cpp
    akonadiitemfetchjobinterface.cpp
    akonadimonitorimpl.cpp
    akonadimonitorinterface.cpp
    akonadiserializer.cpp
    akonadiserializerinterface.cpp
    akonadistorage.cpp
    akonadistorageinterface.cpp
    akonadistoragesettings.cpp
    akonaditaskqueries.cpp
    akonadinotequeries.cpp
    akonaditaskrepository.cpp
)

kde4_add_library(akonadi STATIC ${akonadi_SRCS})
target_link_libraries(akonadi
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_AKONADI_NOTES_LIBS}
    ${KDEPIMLIBS_KCALCORE_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
)
