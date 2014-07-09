set(akonadi_SRCS
    akonadicollectionfetchjobinterface.cpp
    akonadicontextqueries.cpp
    akonadidatasourcequeries.cpp
    akonadiitemfetchjobinterface.cpp
    akonadimonitorimpl.cpp
    akonadimonitorinterface.cpp
    akonadinotequeries.cpp
    akonadinoterepository.cpp
    akonadiserializer.cpp
    akonadiserializerinterface.cpp
    akonadistorage.cpp
    akonadistorageinterface.cpp
    akonadistoragesettings.cpp
    akonaditagfetchjobinterface.cpp
    akonaditaskqueries.cpp
    akonaditaskrepository.cpp
)

kde4_add_library(akonadi STATIC ${akonadi_SRCS})
target_link_libraries(akonadi
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_AKONADI_NOTES_LIBS}
    ${KDEPIMLIBS_KCALCORE_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
)
