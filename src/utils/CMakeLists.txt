
include_directories(${SOPRANO_INCLUDE_DIR})
find_package(SharedDesktopOntologies)
include(NepomukAddOntologyClasses)


set(utils_SRCS
    #akonadibreadcrumbs.cpp
    #breadcrumbsitem.cpp
    #breadcrumbs.cpp
    abstractpimitem.cpp
    incidenceitem.cpp
    note.cpp
    tagmanager.cpp
    queries.cpp
    guiclient.cpp
    #localresourcecreator.cpp
    #akonadicollection.cpp
    datestringbuilder.cpp
    pimitem.cpp
    #pimitemconversionjob.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/zanshin/todonode.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/zanshin/todonodemanager.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/zanshin/todoproxymodelbase.cpp
 )

include_directories(${CMAKE_CURRENT_BINARY_DIR}) #to find ontology classes

nepomuk_add_ontology_classes(
  utils_SRCS
  ONTOLOGIES
  ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig
  ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/ncal.trig
  )

# generate nfo extra since otherwise we have two Attachments classes
nepomuk_add_ontology_classes(
  utils_SRCS
  ONTOLOGIES
  ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig
  CLASSES
  "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#HtmlDocument"
  )

soprano_add_ontology(utils_SRCS ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/kde/aneo.trig "ANEO" "Vocabulary" "trig")

#soprano_add_ontology(utils_SRCS ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/pimo/pimo.trig "PIMO" "Vocabulary" "trig")
#soprano_add_ontology(utils_SRCS ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig "NIE" "Vocabulary" "trig")

# generates a D-Bus interface description from a KConfigXT file (copied from kdepim cmakelists.txt)
#find_package(LibXslt)
#  macro_log_feature(XSLTPROC_EXECUTABLE "xsltproc" "The command line XSLT processor from libxslt" "http://xmlsoft.org/XSLT/" TRUE "" "Required to generate the D-Bus interfaces.")

# macro(kcfg_generate_dbus_interface _kcfg _name)
#   add_custom_command(
#     OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_name}.xml
#     COMMAND ${XSLTPROC_EXECUTABLE} --stringparam interfaceName ${_name}
#     ${KDEPIMLIBS_DATA_DIR}/akonadi-kde/kcfg2dbus.xsl
#     ${_kcfg}
#     > ${CMAKE_CURRENT_BINARY_DIR}/${_name}.xml
#     DEPENDS ${KDEPIMLIBS_DATA_DIR}/akonadi-kde/kcfg2dbus.xsl
#     ${_kcfg}
#     )
# endmacro(kcfg_generate_dbus_interface)

#########################################################################
# Auto creation for local notes.
#########################################################################
# macro(add_resource_iface _kcfgFile _ifaceName _className)
#   kcfg_generate_dbus_interface(${_kcfgFile} ${_ifaceName})
#   string(TOLOWER ${_className} _codeFile)
#   set_source_files_properties( ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES INCLUDE "metatype.h")
#   qt4_add_dbus_interface(utils_SRCS
#     ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_codeFile} ${_className}
#   )
# endmacro(add_resource_iface)

# add_resource_iface( ${CMAKE_CURRENT_SOURCE_DIR}/maildirresource.kcfg
#                     org.kde.Akonadi.Maildir.Settings MaildirSettings )


#########################################################################
# D-Bus interface
#########################################################################

# qt4_generate_dbus_interface( akonadicollection.h org.kde.AkonadiCollection.xml OPTIONS -m )
# qt4_add_dbus_interfaces(utils_SRCS
#     ${CMAKE_CURRENT_BINARY_DIR}/org.kde.AkonadiCollection.xml)


kde4_add_library(utils STATIC ${utils_SRCS})
target_link_libraries(utils
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  calendarsupport
  akonadi_next
  akonadi-notes
  nepomukdatamanagement
  nepomukquery
)
