
include_directories(${SOPRANO_INCLUDE_DIR})
find_package(SharedDesktopOntologies)

set(utils_SRCS
    queries.cpp
    datestringbuilder.cpp
    kdateedit.cpp
    kdatepickerpopup.cpp
 )

include_directories(${CMAKE_CURRENT_BINARY_DIR}) #to find ontology classes

#The aneo.trig resides in kdepim-runtime/agents/ontologies and was copied here to avoid a compiletime dependency
soprano_add_ontology(utils_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/aneo.trig "ANEO" "Vocabulary" "trig")

kde4_add_library(utils STATIC ${utils_SRCS})
target_link_libraries(utils
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${QT_QTXML_LIBRARY}
  akonadi-notes
  ${KDE4_NEPOMUK_LIBS}
#   nepomukdatamanagement
  ${NEPOMUK_CORE_LIBRARY}
  nepomukquery
#   kmime
)
