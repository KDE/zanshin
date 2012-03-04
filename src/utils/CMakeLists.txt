
include_directories(${SOPRANO_INCLUDE_DIR})
find_package(SharedDesktopOntologies)

set(utils_SRCS
    abstractpimitem.cpp
    incidenceitem.cpp
    note.cpp
    tagmanager.cpp
    queries.cpp
    guiclient.cpp
    datestringbuilder.cpp
    pimitem.cpp
 )

include_directories(${CMAKE_CURRENT_BINARY_DIR}) #to find ontology classes

soprano_add_ontology(utils_SRCS ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/kde/aneo.trig "ANEO" "Vocabulary" "trig")

kde4_add_library(utils STATIC ${utils_SRCS})
target_link_libraries(utils
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  calendarsupport
  akonadi-notes
  nepomukdatamanagement
  nepomukquery
)
