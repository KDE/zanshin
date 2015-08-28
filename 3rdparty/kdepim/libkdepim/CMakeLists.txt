KDE4_NO_ENABLE_FINAL(libkdepim)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5300)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(tests)

add_definitions(${QT_QTDBUS_DEFINITIONS})

include_directories( ${Boost_INCLUDE_DIR} )
include_directories( ${BALOO_INCLUDE_DIR} )

option(KDEPIM_INPROCESS_LDAP "Use in-process rather than KIO slave LDAP" FALSE)
if (KDEPIM_INPROCESS_LDAP)
  add_definitions( -DKDEPIM_INPROCESS_LDAP )
endif ()

set(CMAKE_AUTOMOC ON)

########### next target ###############

set(kdepim_LIB_SRCS
   addressline/completionordereditor.cpp
   addressline/addresseelineedit.cpp
   addressline/recentaddresses.cpp
   addressline/kmailcompletion.cpp
   ldap/ldapclient.cpp
   ldap/ldapsearchdialog.cpp
   ldap/ldapclientsearch.cpp
   ldap/ldapclientsearchconfig.cpp
   kdateedit.cpp
   kdatepickerpopup.cpp
)

if (KDEPIM_INPROCESS_LDAP)
  set(kdepim_LIB_SRCS ${kdepim_LIB_SRCS}
    ldap/ldapsession.cpp
    ldap/ldapqueryjob.cpp
  )
endif ()

kde4_add_library(zanshinkdepimstatic STATIC ${kdepim_LIB_SRCS})

target_link_libraries(zanshinkdepimstatic
#  ${KDEPIMLIBS_KRESOURCES_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KLDAP_LIBS}
#  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDE4_KCMUTILS_LIBS}
  ${KDE4_SOLID_LIBS}
#  ${KDEPIMLIBS_KMIME_LIBS}
#  ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
#  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
#  ${KDEPIMLIBS_AKONADI_CALENDAR_LIBS}
  ${BALOO_LIBRARIES}
)
