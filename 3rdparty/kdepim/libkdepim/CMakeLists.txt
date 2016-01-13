add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DTRANSLATION_DOMAIN=\"libkdepim\")

option(KDEPIM_INPROCESS_LDAP "Use in-process rather than KIO slave LDAP" FALSE)
if (KDEPIM_INPROCESS_LDAP)
  add_definitions( -DKDEPIM_INPROCESS_LDAP )
endif ()

set(kdepim_LIB_SRCS
   addressline/addresseelineedit.cpp
   addressline/addresseelineedit_p.cpp
   addressline/addresseelineeditstatic.cpp
   addressline/baloocompletionemail.cpp
   addressline/kmailcompletion.cpp
   blacklistbaloocompletion/blacklistbalooemailcompletiondialog.cpp
   blacklistbaloocompletion/blacklistbalooemailcompletionwidget.cpp
   blacklistbaloocompletion/blacklistbalooemaillist.cpp
   blacklistbaloocompletion/blacklistbalooemailsearchjob.cpp
   blacklistbaloocompletion/blacklistbalooemailutil.cpp
   blacklistbaloocompletion/blacklistbalooemailwarning.cpp
   completionorder/completionordereditor.cpp
   completionorder/completionorderwidget.cpp
   completionconfiguredialog/completionconfiguredialog.cpp
   recentaddress/recentaddressdialog.cpp
   recentaddress/recentaddresses.cpp
   recentaddress/recentaddresswidget.cpp
   kdateedit.cpp
   kdatepickerpopup.cpp
   ldap/ldapclient.cpp
   ldap/ldapclient_debug.cpp
   ldap/ldapclientsearch.cpp
   ldap/ldapclientsearchconfig.cpp
   libkdepim_debug.cpp
)

if (KDEPIM_INPROCESS_LDAP)
  set(kdepim_LIB_SRCS ${kdepim_LIB_SRCS}
    ldap/ldapsession.cpp
    ldap/ldapqueryjob.cpp
  )
endif ()

add_library(zanshinkdepimstatic STATIC ${kdepim_LIB_SRCS})
generate_export_header(zanshinkdepimstatic BASE_NAME kdepim)

target_link_libraries(zanshinkdepimstatic
  KF5::AkonadiContact
  KF5::AkonadiSearchPIM
  KF5::Codecs
  KF5::Completion
  KF5::ConfigCore
  KF5::ConfigWidgets
  KF5::I18n
  KF5::ItemViews
  KF5::JobWidgets
  KF5::KDELibs4Support
  KF5::KIOCore
  KF5::Ldap
  KF5::Wallet
  KF5::WidgetsAddons
  Qt5::DBus
  Qt5::Network
)
add_subdirectory(tests)
